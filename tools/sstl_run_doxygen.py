#!/usr/bin/env python3
"""Generate local Doxygen documentation for SSTL.

How to run from a terminal:
  python tools/sstl_run_doxygen.py --backend auto
  python tools/sstl_run_doxygen.py --backend local
  python tools/sstl_run_doxygen.py --backend docker --yes-virtualize
  python tools/sstl_run_doxygen.py --backend podman --yes-virtualize
  python tools/sstl_run_doxygen.py --backend wsl --yes-virtualize
  python tools/sstl_run_doxygen.py --backend auto --install-missing
  python tools/sstl_run_doxygen.py --backend auto --prepare-backends

How to run from a file explorer:
  Double-click this file. With no command-line arguments it opens a Tkinter GUI
  with buttons for auto/local/container/WSL generation, backend preparation,
  setup recommendations, help, cancellation, and opening the generated HTML
  index.

What the script does:
  - Purges the previous generated Doxygen artifacts owned by this script.
    Container backends can also clean up prior container-owned Doxygen output
    when Windows refuses to delete those files directly.
  - Writes a deterministic Doxyfile under artifacts/doxygen.
  - Uses local doxygen first when --backend auto is selected.
  - Uses Docker, Podman, or WSL only when the user explicitly chose that backend
    or consented through --yes-virtualize, a CLI prompt, or the GUI prompt.
  - Prints safe install recommendations and can prepare installed container/WSL
    backends after explicit user confirmation.
  - Generates HTML into artifacts/doxygen/html.
  - Writes artifacts/doxygen/doxygen-warnings.log and
    artifacts/doxygen/doxygen_summary.yaml for later inspection.

Guardrails:
  - Every external command has an overall timeout.
  - A no-output watchdog terminates commands that appear stuck.
  - The GUI Cancel button requests termination of the active command.
  - Auto mode falls through to recommendations instead of silently doing
    virtualized work without consent.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import os
from pathlib import Path
import queue
import shutil
import stat
import subprocess
import sys
import threading
import time
import webbrowser

from sstl_tool_common import (
    InstallOption,
    ask_yes_no,
    attach_text_copy_context_menu,
    availability_label,
    container_mount_arg_for_root,
    container_engine_probe,
    container_engine_summary,
    display_command_arg_for_root,
    find_container_engine_executable,
    find_docker_desktop_executable,
    find_named_executable,
    maybe_relaunch_windows_gui,
    podman_machine_exists,
    repo_root_from_script,
    set_minimum_window_size,
    shell_quote,
    wait_for_container_engine,
    wsl_candidate_distros,
    wsl_command,
    wsl_installed_distros,
)


ROOT = repo_root_from_script(__file__)
ARTIFACT_ROOT = ROOT / "artifacts"
DEFAULT_OUTPUT = ARTIFACT_ROOT / "doxygen"
DOCKER_IMAGE = "gcc:13-bookworm"
BACKENDS = ("auto", "local", "docker", "podman", "wsl")
DEFAULT_TIMEOUT_SECONDS = 20 * 60
NO_OUTPUT_TIMEOUT_SECONDS = 2 * 60
WSL_DOXYGEN_TOOL_INSTALL = "sudo apt-get update && sudo apt-get install -y doxygen graphviz"
WSL_DOXYGEN_TOOL_INSTALL_ROOT = "DEBIAN_FRONTEND=noninteractive apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y doxygen graphviz"

HELP_TEXT = """SSTL Doxygen Runner

Terminal examples:
  python tools/sstl_run_doxygen.py --backend auto
  python tools/sstl_run_doxygen.py --backend local
  python tools/sstl_run_doxygen.py --backend docker --yes-virtualize
  python tools/sstl_run_doxygen.py --backend podman --yes-virtualize
  python tools/sstl_run_doxygen.py --backend wsl --yes-virtualize --wsl-distro Ubuntu
  python tools/sstl_run_doxygen.py --backend auto --fail-on-warnings

Backend behavior:
  auto    Try local Doxygen first. If local Doxygen is missing, ask before
          trying Docker, Podman, or WSL.
  local   Use doxygen from PATH and write artifacts/doxygen locally.
  docker  Run Doxygen in a disposable Linux container.
          Requires a reachable Docker Desktop/daemon.
  podman  Same as Docker, using Podman.
          Requires a reachable Podman machine/socket.
  wsl     Use Doxygen already installed inside a non-internal WSL distro.
          Use --wsl-distro Ubuntu or SSTL_WSL_DISTRO=Ubuntu to force a distro.

Backend setup:
  --install-missing prints setup options and asks before running the first safe
  installer command for the selected backend.
  --prepare-backends asks before trying to make installed Docker/Podman/WSL
  backends usable. This can start Docker Desktop, initialize/start Podman, or
  install Doxygen/Graphviz inside WSL.

Double-click behavior:
  With no command-line arguments, this script opens the GUI. Click Auto Doxygen
  for normal discovery, Local Doxygen to avoid virtualized environments, or one
  of the backend-specific Doxygen buttons when you explicitly want that backend.
  Backend-specific buttons are disabled until their backend is available.

Generated files:
  artifacts/doxygen/Doxyfile
  artifacts/doxygen/html/index.html
  artifacts/doxygen/doxygen-warnings.log
  artifacts/doxygen/doxygen_summary.yaml

Before each generation run, the runner removes its previous owned artifacts:
  artifacts/doxygen/html
  artifacts/doxygen/Doxyfile
  artifacts/doxygen/doxygen-warnings.log
  artifacts/doxygen/doxygen_summary.yaml

If the previous Doxygen output was produced by a container and cannot be
removed directly by the Windows host, Docker/Podman generation retries cleanup
inside a disposable container before writing the new Doxyfile.
"""


def now_utc() -> str:
    """Return an ISO-8601 UTC timestamp for generated evidence files."""

    return datetime.now(timezone.utc).isoformat()


class Runner:
    """Small subprocess runner with streaming logs and freeze detection."""

    def __init__(self, timeout: int, no_output_timeout: int, log_func=print) -> None:
        self.timeout = timeout
        self.no_output_timeout = no_output_timeout
        self.log = log_func
        self.cancelled = threading.Event()
        self.process: subprocess.Popen[str] | None = None

    def cancel(self) -> None:
        """Request cancellation and terminate the active child process."""

        self.cancelled.set()
        if self.process and self.process.poll() is None:
            self._terminate_process(self.process)

    def run(self, name: str, cmd: list[str], cwd: Path) -> bool:
        """Run one command, stream output, and return whether it completed."""

        self.log(f"\n== {name}")
        self.log(" ".join(display_command_arg_for_root(ROOT, arg) for arg in cmd))
        output_queue: queue.Queue[str | None] = queue.Queue()
        start = time.monotonic()
        last_output = start

        try:
            creationflags = 0
            if os.name == "nt":
                creationflags = getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0)
            self.process = subprocess.Popen(
                cmd,
                cwd=str(cwd),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                creationflags=creationflags,
            )
        except OSError as exc:
            self.log(f"FAILED to start command: {exc}")
            return False

        def reader() -> None:
            assert self.process is not None
            assert self.process.stdout is not None
            try:
                for line in self.process.stdout:
                    output_queue.put(line.rstrip("\n"))
            finally:
                output_queue.put(None)

        thread = threading.Thread(target=reader, daemon=True)
        thread.start()

        stream_done = False
        while True:
            try:
                item = output_queue.get(timeout=0.2)
                if item is None:
                    stream_done = True
                else:
                    last_output = time.monotonic()
                    self.log(item)
            except queue.Empty:
                pass

            assert self.process is not None
            rc = self.process.poll()
            now = time.monotonic()
            if self.cancelled.is_set():
                self._terminate_process(self.process)
                self.log("FAILED: command cancelled.")
                return False
            if rc is None and now - start > self.timeout:
                self._terminate_process(self.process)
                self.log(f"FAILED: command exceeded timeout of {self.timeout} seconds.")
                return False
            if rc is None and now - last_output > self.no_output_timeout:
                self._terminate_process(self.process)
                self.log(
                    "FAILED: command produced no output for "
                    f"{self.no_output_timeout} seconds."
                )
                return False
            if rc is not None and stream_done:
                self.log(f"exit code: {rc}")
                return rc == 0

    def _terminate_process(self, process: subprocess.Popen[str]) -> None:
        """Terminate a process without assuming a platform-specific shell."""

        if process.poll() is not None:
            return
        try:
            process.terminate()
            process.wait(timeout=5)
            return
        except Exception:
            pass
        try:
            process.kill()
        except Exception:
            pass


def output_paths(output: Path) -> dict[str, Path]:
    """Return the files and folders owned by one documentation run."""

    return {
        "root": output,
        "html": output / "html",
        "index": output / "html" / "index.html",
        "doxyfile": output / "Doxyfile",
        "warnings": output / "doxygen-warnings.log",
        "summary": output / "doxygen_summary.yaml",
    }


def output_is_safe_to_purge(output: Path) -> bool:
    """Return whether the output path is specific enough for generated-doc cleanup."""

    resolved_output = output.resolve()
    resolved_root = ROOT.resolve()
    if resolved_output == resolved_root:
        return False
    try:
        resolved_output.relative_to(resolved_root)
    except ValueError:
        return False
    return True


def purge_generated_output(output: Path, runner: Runner) -> bool:
    """Remove prior Doxygen artifacts owned by this runner before a new run."""

    def make_writable_and_retry(func, path, _exc_info) -> None:
        os.chmod(path, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
        func(path)

    def remove_file(path: Path) -> None:
        try:
            path.unlink()
        except PermissionError:
            os.chmod(path, stat.S_IREAD | stat.S_IWRITE)
            path.unlink()

    if not output_is_safe_to_purge(output):
        runner.log("Refusing to purge Doxygen artifacts for unsafe output path: " + str(output))
        return False

    paths = output_paths(output)
    paths["root"].mkdir(parents=True, exist_ok=True)
    for key in ("html", "doxyfile", "warnings", "summary"):
        path = paths[key]
        try:
            if path.is_dir():
                shutil.rmtree(path, onerror=make_writable_and_retry)
                runner.log("Removed previous Doxygen artifact directory: " + str(path))
            elif path.exists():
                remove_file(path)
                runner.log("Removed previous Doxygen artifact file: " + str(path))
        except Exception as exc:
            runner.log("Failed to remove previous Doxygen artifact " + str(path) + ": " + str(exc))
            return False
    return True


def container_purge_generated_output(output: Path, runner: Runner, engine: str) -> bool:
    """Remove prior container-owned Doxygen artifacts through Docker/Podman."""

    if not output_is_safe_to_purge(output):
        runner.log("Refusing container Doxygen purge for unsafe output path: " + str(output))
        return False
    engine_ok, launcher, detail = container_engine_probe(engine, timeout=20)
    if not engine_ok:
        runner.log(container_engine_summary(engine, detail) + ".")
        if detail:
            runner.log("Diagnostic: " + detail)
        return False
    paths = output_paths(output)
    try:
        rel_root = paths["root"].resolve().relative_to(ROOT.resolve()).as_posix()
        owned = [
            paths["html"].resolve().relative_to(ROOT.resolve()).as_posix(),
            paths["doxyfile"].resolve().relative_to(ROOT.resolve()).as_posix(),
            paths["warnings"].resolve().relative_to(ROOT.resolve()).as_posix(),
            paths["summary"].resolve().relative_to(ROOT.resolve()).as_posix(),
        ]
    except ValueError as exc:
        runner.log("Refusing container Doxygen purge outside repo root: " + str(exc))
        return False
    if not rel_root or rel_root == ".":
        runner.log("Refusing container Doxygen purge for unsafe root: " + rel_root)
        return False
    shell = (
        "set -e; cd /work; "
        "for p in "
        + " ".join(shell_quote(path) for path in owned)
        + "; do "
        "case \"$p\" in */..|../*|/*|'') echo \"refusing unsafe purge path: $p\"; exit 2 ;; esac; "
        "rm -rf -- \"$p\"; "
        "done; "
        "mkdir -p -- "
        + shell_quote(rel_root)
    )
    return runner.run(
        f"{engine} Doxygen artifact purge",
        [
            launcher,
            "run",
            "--rm",
            "-v",
            container_mount_arg_for_root(ROOT),
            "-w",
            "/work",
            DOCKER_IMAGE,
            "bash",
            "-lc",
            shell,
        ],
        ROOT,
    )


def prepare_generated_output(output: Path, runner: Runner, backend: str) -> bool:
    """Prepare the output directory, using the backend when host cleanup is blocked."""

    if purge_generated_output(output, runner):
        return True
    if backend in {"docker", "podman"}:
        runner.log("Host Doxygen purge failed; trying " + backend + " cleanup for prior container-owned artifacts.")
        return container_purge_generated_output(output, runner, backend)
    return False


def write_doxyfile(output: Path) -> Path:
    """Create the Doxyfile used by local and virtualized backends."""

    paths = output_paths(output)
    paths["html"].mkdir(parents=True, exist_ok=True)
    doxyfile = paths["doxyfile"]
    rel_output = output.relative_to(ROOT).as_posix()
    doxyfile.write_text(
        "\n".join(
            [
                "PROJECT_NAME           = SSTL",
                "PROJECT_BRIEF          = Static STL subset public API documentation",
                f"OUTPUT_DIRECTORY       = {rel_output}",
                "INPUT                  = include",
                "RECURSIVE              = YES",
                "FILE_PATTERNS          = *.hpp *.h",
                "EXCLUDE_PATTERNS       = */deprecated/* */Deprecated*/*",
                "GENERATE_HTML          = YES",
                "GENERATE_LATEX         = NO",
                "GENERATE_XML           = NO",
                "QUIET                  = NO",
                "WARNINGS               = YES",
                "WARN_IF_UNDOCUMENTED   = YES",
                "WARN_IF_DOC_ERROR      = YES",
                "WARN_NO_PARAMDOC       = YES",
                "WARN_AS_ERROR          = NO",
                f"WARN_LOGFILE           = {rel_output}/doxygen-warnings.log",
                "EXTRACT_ALL            = YES",
                "EXTRACT_PRIVATE        = YES",
                "EXTRACT_STATIC         = YES",
                "EXTRACT_LOCAL_CLASSES  = YES",
                "EXTRACT_LOCAL_METHODS  = YES",
                "ENABLE_PREPROCESSING   = YES",
                "MACRO_EXPANSION        = YES",
                "EXPAND_ONLY_PREDEF     = NO",
                "PREDEFINED             = SSTL_C_UNUSED= SSTL_C_DEPRECATED= __attribute__(x)= __declspec(x)=",
                "SKIP_FUNCTION_MACROS   = NO",
                "HAVE_DOT               = YES",
                "CALL_GRAPH             = NO",
                "CALLER_GRAPH           = NO",
                "SOURCE_BROWSER         = YES",
                "INLINE_SOURCES         = NO",
                "FULL_PATH_NAMES        = NO",
                "STRIP_FROM_PATH        = /work",
                "HTML_OUTPUT            = html",
                "",
            ]
        ),
        encoding="utf-8",
    )
    return doxyfile


def warning_count(warnings: Path) -> int:
    """Count Doxygen warning lines in the generated warning log."""

    if not warnings.exists():
        return 0
    return sum(1 for line in warnings.read_text(encoding="utf-8", errors="replace").splitlines() if "warning:" in line)


def write_summary(output: Path, backend: str, ok: bool, warnings: int) -> None:
    """Write machine-readable status for reports and future validation."""

    paths = output_paths(output)
    status = "pass" if ok and warnings == 0 else ("warn" if ok else "fail")
    lines = [
        f"status: {status}",
        f"backend: {backend}",
        f"generated_at_utc: {now_utc()}",
        f"warning_count: {warnings}",
        f"html_index: {paths['index'].relative_to(ROOT).as_posix()}",
        f"warning_log: {paths['warnings'].relative_to(ROOT).as_posix()}",
    ]
    paths["summary"].write_text("\n".join(lines) + "\n", encoding="utf-8")


def doxygen_available() -> str | None:
    """Return the local Doxygen executable path when available."""

    return find_named_executable("doxygen")


def local_backend(runner: Runner, output: Path) -> bool:
    """Generate documentation with the local Doxygen executable."""

    doxygen = doxygen_available()
    if not doxygen:
        runner.log("Local Doxygen was not found on PATH.")
        return False
    doxyfile = write_doxyfile(output)
    runner.log(f"Using local Doxygen: {doxygen}")
    return runner.run("doxygen version", [doxygen, "--version"], ROOT) and runner.run(
        "generate Doxygen HTML", [doxygen, str(doxyfile)], ROOT
    )


def container_backend(runner: Runner, output: Path, engine: str) -> bool:
    """Generate documentation inside a disposable Docker or Podman container."""

    engine_ok, launcher, detail = container_engine_probe(engine, timeout=20)
    if not engine_ok:
        runner.log(container_engine_summary(engine, detail) + ".")
        if detail:
            runner.log("Diagnostic: " + detail)
        return False
    doxyfile = write_doxyfile(output)
    inner = (
        "set -e; "
        "if ! command -v doxygen >/dev/null 2>&1; then "
        "apt-get update && apt-get install -y --no-install-recommends doxygen graphviz; "
        "fi; "
        f"doxygen {shell_quote(doxyfile.relative_to(ROOT).as_posix())}"
    )
    return runner.run(
        f"{engine} Doxygen generation",
        [
            launcher,
            "run",
            "--rm",
            "-v",
            container_mount_arg_for_root(ROOT),
            "-w",
            "/work",
            DOCKER_IMAGE,
            "bash",
            "-lc",
            inner,
        ],
        ROOT,
    )


def probe_wsl_doxygen_distro(wsl: str, distro: str | None) -> tuple[bool, str]:
    """Return whether a non-internal WSL distro can generate Doxygen output."""

    probe = (
        "uname -s; "
        "test -d include && test -d tools || { echo 'checkout is not visible from WSL inherited cwd'; exit 3; }; "
        "if command -v doxygen >/dev/null 2>&1 && command -v dot >/dev/null 2>&1; then "
        "  echo ready; "
        "elif command -v apt-get >/dev/null 2>&1; then "
        "  echo 'missing doxygen/graphviz; install inside WSL with: " + WSL_DOXYGEN_TOOL_INSTALL + "'; "
        "  exit 3; "
        "else "
        "  echo 'missing doxygen/graphviz and apt-get is not available'; "
        "  exit 3; "
        "fi"
    )
    try:
        proc = subprocess.run(
            wsl_command(wsl, distro, "sh", "-lc", probe),
            cwd=str(ROOT),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            timeout=60,
        )
    except Exception as exc:
        return False, str(exc)
    output = " ".join(line.strip() for line in (proc.stdout or "").splitlines() if line.strip())
    return proc.returncode == 0, output or f"exit {proc.returncode}"


def select_wsl_doxygen_distro(configured: str = "", log_func=None) -> tuple[str, str | None] | None:
    """Select a non-internal WSL distro with Doxygen and graphviz available."""

    if os.name != "nt":
        if log_func:
            log_func("WSL backend is only available from Windows hosts.")
        return None
    wsl = find_named_executable("wsl")
    if not wsl:
        if log_func:
            log_func("wsl executable was not found.")
        return None
    for label, distro in wsl_candidate_distros(wsl, configured):
        ok, detail = probe_wsl_doxygen_distro(wsl, distro)
        if ok:
            if log_func:
                log_func("Selected WSL distro: " + label + " (" + detail + ")")
            return wsl, distro
        if log_func:
            log_func("WSL distro is not usable for Doxygen: " + label + " (" + detail + ")")
    if configured:
        if log_func:
            log_func("Configured WSL distro was not usable for Doxygen: " + configured)
    elif log_func:
        log_func("No Doxygen-capable WSL distro found. Install Ubuntu/Debian tools or pass --wsl-distro.")
    return None


def wsl_installable_distro(configured: str = "", log_func=None) -> tuple[str, str | None] | None:
    """Select a non-internal WSL distro that can install Doxygen with apt-get."""

    if os.name != "nt":
        if log_func:
            log_func("WSL preparation is only available from Windows hosts.")
        return None
    wsl = find_named_executable("wsl")
    if not wsl:
        if log_func:
            log_func("wsl executable was not found.")
        return None
    for label, distro in wsl_candidate_distros(wsl, configured):
        try:
            proc = subprocess.run(
                wsl_command(wsl, distro, "sh", "-lc", "uname -s && command -v apt-get"),
                cwd=str(ROOT),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                errors="replace",
                timeout=60,
            )
        except Exception as exc:
            if log_func:
                log_func("WSL distro is not installable: " + label + f" ({exc})")
            continue
        detail = " ".join(line.strip() for line in (proc.stdout or "").splitlines() if line.strip())
        if proc.returncode == 0:
            if log_func:
                log_func("Selected WSL distro for preparation: " + label + " (" + detail + ")")
            return wsl, distro
        if log_func:
            log_func("WSL distro is not installable: " + label + " (" + detail + ")")
    if log_func:
        log_func("No Ubuntu/Debian-like WSL distro with apt-get was found for preparation.")
    return None


def wsl_backend(runner: Runner, output: Path, wsl_distro: str = "") -> bool:
    """Generate documentation with Doxygen already installed inside WSL."""

    selected = select_wsl_doxygen_distro(wsl_distro, runner.log)
    if not selected:
        return False
    wsl, distro = selected
    doxyfile = write_doxyfile(output)
    command = (
        "if ! command -v doxygen >/dev/null 2>&1; then "
        "echo 'Doxygen is not installed inside WSL. Install doxygen and graphviz there, or use Docker/Podman.'; "
        "exit 3; "
        "fi; "
        f"doxygen {shell_quote(doxyfile.relative_to(ROOT).as_posix())}"
    )
    return runner.run("WSL Doxygen generation", wsl_command(wsl, distro, "sh", "-lc", command), ROOT)


def install_options() -> list[InstallOption]:
    """Return safe setup commands for Doxygen-capable backends."""

    opts: list[InstallOption] = []
    if os.name == "nt":
        winget = find_named_executable("winget")
        if winget:
            opts.extend(
                [
                    InstallOption(
                        "Doxygen + Graphviz via winget",
                        "local",
                        [[winget, "install", "--id", "doxygen.doxygen", "-e"], [winget, "install", "--id", "Graphviz.Graphviz", "-e"]],
                        ["Installs the local documentation toolchain used by the Local Doxygen button."],
                    ),
                    InstallOption(
                        "Docker Desktop via winget",
                        "docker",
                        [[winget, "install", "--id", "Docker.DockerDesktop", "-e"]],
                        ["May require administrator approval, sign-in, or a reboot. Start Docker Desktop once after installation."],
                    ),
                    InstallOption(
                        "Podman Desktop via winget",
                        "podman",
                        [[winget, "install", "--id", "RedHat.Podman-Desktop", "-e"]],
                        ["After installation, start Podman Desktop once or run podman machine init && podman machine start."],
                    ),
                ]
            )
        opts.append(
            InstallOption(
                "WSL2 Ubuntu",
                "wsl",
                [[find_named_executable("wsl") or "wsl", "--install", "-d", "Ubuntu"]],
                [
                    "May require administrator approval and a reboot.",
                    "After first Ubuntu launch, install Doxygen tools inside WSL: " + WSL_DOXYGEN_TOOL_INSTALL,
                ],
            )
        )
        return opts
    if sys.platform == "darwin":
        brew = find_named_executable("brew")
        if brew:
            opts.append(InstallOption("Doxygen + Graphviz via Homebrew", "local", [[brew, "install", "doxygen", "graphviz"]], []))
            opts.append(InstallOption("Podman via Homebrew", "podman", [[brew, "install", "podman"]], ["Run podman machine init/start after install if prompted."]))
        opts.append(InstallOption("Docker Desktop manual install", "docker", [], ["Download and install Docker Desktop from https://www.docker.com/products/docker-desktop/."]))
        return opts

    sudo = find_named_executable("sudo") or "sudo"
    apt_get = find_named_executable("apt-get")
    dnf = find_named_executable("dnf")
    pacman = find_named_executable("pacman")
    if apt_get:
        opts.append(InstallOption("Doxygen + Graphviz via apt", "local", [[sudo, apt_get, "update"], [sudo, apt_get, "install", "-y", "doxygen", "graphviz"]], []))
        opts.append(InstallOption("Podman via apt", "podman", [[sudo, apt_get, "update"], [sudo, apt_get, "install", "-y", "podman"]], []))
    if dnf:
        opts.append(InstallOption("Doxygen + Graphviz via dnf", "local", [[sudo, dnf, "install", "-y", "doxygen", "graphviz"]], []))
        opts.append(InstallOption("Podman via dnf", "podman", [[sudo, dnf, "install", "-y", "podman"]], []))
    if pacman:
        opts.append(InstallOption("Doxygen + Graphviz via pacman", "local", [[sudo, pacman, "-S", "--needed", "doxygen", "graphviz"]], []))
    return opts


def recommendation_lines(backend: str = "auto") -> list[str]:
    if backend in ("docker", "podman"):
        ok, resolved, detail = container_engine_probe(backend, timeout=10)
        if resolved and not ok:
            if backend == "podman":
                return [
                    "Podman is installed, but its Linux VM/socket is not reachable.",
                    "Recommended setup commands:",
                    "  podman machine list",
                    "  podman machine init",
                    "  podman machine start",
                    "After that, rerun Doxygen or verify with: podman info",
                    "Diagnostic: " + detail,
                ]
            return [
                "Docker is installed, but the Docker engine is not reachable.",
                "Start Docker Desktop, wait until it reports the engine is running, then rerun Doxygen.",
                "You can verify with: docker info",
                "Diagnostic: " + detail,
            ]

    if backend == "wsl":
        wsl = find_named_executable("wsl")
        if wsl and wsl_installed_distros(wsl):
            return [
                "WSL is installed, but no non-internal distro with Doxygen and Graphviz was found.",
                "Install tools inside your Ubuntu/Debian WSL distro, then rerun Doxygen:",
                "  " + WSL_DOXYGEN_TOOL_INSTALL,
                "Verify inside WSL with:",
                "  command -v doxygen && command -v dot",
            ]

    opts = install_options()
    if backend != "auto":
        opts = [opt for opt in opts if opt.backend == backend]
    if not opts:
        if backend == "auto":
            return ["No automatic installer was detected. Install Doxygen locally or make Docker, Podman, or WSL available."]
        return [f"No automatic installer was detected for backend {backend}."]
    lines = ["Recommended install options:" if backend == "auto" else f"Recommended install options for backend {backend}:"]
    for i, opt in enumerate(opts, 1):
        lines.append(f"  {i}. {opt.name} -> backend {opt.backend}")
        if opt.commands:
            lines.append("     Commands:")
            for cmd in opt.commands:
                lines.append("       " + " ".join(cmd))
        for note in opt.notes:
            lines.append("     " + note)
    return lines


def print_recommendations(runner: Runner, backend: str = "auto") -> None:
    runner.log("")
    runner.log("== Install recommendations")
    for line in recommendation_lines(backend):
        runner.log(line)


def recommendations() -> str:
    """Return practical setup guidance when no backend can run."""

    return "No usable Doxygen backend completed.\n\n" + "\n".join(recommendation_lines("auto"))


def install_option(runner: Runner, option: InstallOption) -> bool:
    if not option.commands:
        runner.log(f"{option.name} has no safe automatic install command. Please install it manually.")
        return False
    runner.log("")
    runner.log("== Installing " + option.name)
    for note in option.notes:
        runner.log("Note: " + note)
    ok = True
    for cmd in option.commands:
        ok = runner.run("install: " + option.name, cmd, ROOT) and ok
        if not ok:
            break
    return ok


def maybe_install_missing(runner: Runner, backend: str, assume_yes: bool) -> bool:
    opts = install_options()
    if backend != "auto":
        opts = [opt for opt in opts if opt.backend == backend]
    if not opts:
        print_recommendations(runner, backend)
        return False
    print_recommendations(runner, backend)
    option = opts[0]
    if not assume_yes and not ask_yes_no(f"Install '{option.name}' now?", default=False):
        runner.log("Install declined.")
        return False
    return install_option(runner, option)


def should_prepare(runner: Runner, message: str, assume_yes: bool) -> bool:
    if assume_yes:
        return True
    if ask_yes_no(message, default=False):
        return True
    runner.log("Preparation declined: " + message)
    return False


def run_prepare_capture(runner: Runner, name: str, cmd: list[str], timeout: int = 300) -> tuple[bool, str]:
    runner.log("")
    runner.log("== " + name)
    runner.log("+ " + " ".join(cmd))
    try:
        proc = subprocess.run(
            cmd,
            cwd=str(ROOT),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as exc:
        output = exc.stdout if isinstance(exc.stdout, str) else (exc.stdout or b"").decode(errors="replace")
        runner.log(output.rstrip())
        runner.log("FAILED: " + name + f" (timed out after {timeout} seconds)")
        return False, output
    except Exception as exc:
        runner.log("FAILED: " + name + f" ({exc})")
        return False, ""
    output = proc.stdout or ""
    if output:
        runner.log(output.rstrip())
    ok = proc.returncode == 0
    runner.log(("PASS: " if ok else "FAIL: ") + name + ("" if ok else f" (exit {proc.returncode})"))
    return ok, output


def prepare_docker_backend(runner: Runner, assume_yes_prepare: bool, install_missing: bool, assume_yes_install: bool) -> bool:
    ok, _, detail = container_engine_probe("docker", timeout=10)
    if ok:
        runner.log("Docker engine is already reachable.")
        return True
    docker = find_container_engine_executable("docker")
    if not docker:
        runner.log("Docker executable was not found.")
        return maybe_install_missing(runner, "docker", assume_yes_install) if install_missing else False
    desktop = find_docker_desktop_executable()
    if not desktop:
        runner.log("Docker is installed, but Docker Desktop could not be found. Start the Docker daemon/Desktop manually.")
        runner.log("Diagnostic: " + detail)
        return False
    if not should_prepare(runner, "Start Docker Desktop and wait for the engine?", assume_yes_prepare):
        return False
    runner.log("")
    runner.log("== start Docker Desktop")
    runner.log("+ " + desktop)
    try:
        subprocess.Popen([desktop], cwd=str(ROOT), close_fds=True)
    except Exception as exc:
        runner.log("FAIL: start Docker Desktop (" + str(exc) + ")")
        return False
    ok, _ = wait_for_container_engine("docker", 180, runner.log)
    return ok


def prepare_podman_backend(runner: Runner, assume_yes_prepare: bool, install_missing: bool, assume_yes_install: bool) -> bool:
    ok, _, _ = container_engine_probe("podman", timeout=10)
    if ok:
        runner.log("Podman engine is already reachable.")
        return True
    podman = find_container_engine_executable("podman")
    if not podman:
        runner.log("Podman executable was not found.")
        return maybe_install_missing(runner, "podman", assume_yes_install) if install_missing else False
    if not should_prepare(runner, "Initialize/start a Podman machine?", assume_yes_prepare):
        return False
    list_ok, list_output = run_prepare_capture(runner, "podman machine list", [podman, "machine", "list"], timeout=120)
    if not list_ok:
        return False
    if not podman_machine_exists(list_output):
        if not run_prepare_capture(runner, "podman machine init", [podman, "machine", "init"], timeout=20 * 60)[0]:
            return False
    if not run_prepare_capture(runner, "podman machine start", [podman, "machine", "start"], timeout=20 * 60)[0]:
        return False
    ok, _ = wait_for_container_engine("podman", 180, runner.log)
    return ok


def prepare_wsl_backend(
    runner: Runner,
    assume_yes_prepare: bool,
    install_missing: bool,
    assume_yes_install: bool,
    wsl_distro: str = "",
) -> bool:
    if select_wsl_doxygen_distro(wsl_distro):
        runner.log("WSL Doxygen tools are already available.")
        return True
    installable = wsl_installable_distro(wsl_distro, runner.log)
    if not installable:
        return maybe_install_missing(runner, "wsl", assume_yes_install) if install_missing else False
    if not should_prepare(runner, "Install Doxygen and Graphviz inside WSL as root?", assume_yes_prepare):
        return False
    wsl, distro = installable
    ok = runner.run(
        "wsl install Doxygen tools",
        wsl_command(wsl, distro, "-u", "root", "sh", "-lc", WSL_DOXYGEN_TOOL_INSTALL_ROOT),
        ROOT,
    )
    return ok and select_wsl_doxygen_distro(wsl_distro, runner.log) is not None


def prepare_local_backend(runner: Runner, install_missing: bool, assume_yes_install: bool) -> bool:
    if doxygen_available():
        runner.log("Local Doxygen is already available.")
        return True
    runner.log("Local Doxygen was not found.")
    if install_missing:
        return maybe_install_missing(runner, "local", assume_yes_install)
    runner.log("Local Doxygen installation is handled by Recommend / Install, not Prepare Backends.")
    return False


def prepare_selected_backends(
    runner: Runner,
    backend: str,
    assume_yes_prepare: bool,
    install_missing: bool = False,
    assume_yes_install: bool = False,
    wsl_distro: str = "",
) -> bool:
    targets = [backend] if backend != "auto" else ["docker", "podman", "wsl"]
    ok = True
    for target in targets:
        if target == "docker":
            ok = prepare_docker_backend(runner, assume_yes_prepare, install_missing, assume_yes_install) and ok
        elif target == "podman":
            ok = prepare_podman_backend(runner, assume_yes_prepare, install_missing, assume_yes_install) and ok
        elif target == "wsl":
            ok = prepare_wsl_backend(runner, assume_yes_prepare, install_missing, assume_yes_install, wsl_distro) and ok
        elif target == "local":
            ok = prepare_local_backend(runner, install_missing, assume_yes_install) and ok
    return ok


def run_backend(
    runner: Runner,
    backend: str,
    output: Path,
    virtualize: bool | None,
    fail_on_warnings: bool,
    wsl_distro: str = "",
) -> bool:
    """Run the requested backend and emit a summary file."""

    paths = output_paths(output)
    ok = False
    used_backend = backend

    if backend == "local":
        if not prepare_generated_output(output, runner, "local"):
            return False
        ok = local_backend(runner, output)
        used_backend = "local"
    elif backend in {"docker", "podman"}:
        if not prepare_generated_output(output, runner, backend):
            return False
        ok = container_backend(runner, output, backend)
        used_backend = backend
    elif backend == "wsl":
        if not prepare_generated_output(output, runner, "wsl"):
            return False
        ok = wsl_backend(runner, output, wsl_distro)
        used_backend = "wsl"
    elif backend == "auto":
        if doxygen_available():
            if prepare_generated_output(output, runner, "local") and local_backend(runner, output):
                ok = True
                used_backend = "local"
            else:
                runner.log("Local Doxygen preparation or generation failed.")
        else:
            local_backend(runner, output)
        if not ok:
            consent = virtualize
            if consent is None:
                consent = ask_yes_no("Local Doxygen is unavailable. Try a virtualized backend?", False)
            if not consent:
                runner.log("Virtualized Doxygen generation was not consented; stopping after local discovery.")
            else:
                for candidate in ("docker", "podman", "wsl"):
                    runner.log(f"\n-- Trying {candidate} backend")
                    if not prepare_generated_output(output, runner, candidate):
                        continue
                    if candidate in {"docker", "podman"}:
                        ok = container_backend(runner, output, candidate)
                    else:
                        ok = wsl_backend(runner, output, wsl_distro)
                    used_backend = candidate
                    if ok:
                        break
    else:
        runner.log(f"Unknown backend: {backend}")
        ok = False

    warnings = warning_count(paths["warnings"])
    write_summary(output, used_backend, ok, warnings)
    if paths["index"].exists():
        runner.log(f"\nHTML index: {paths['index']}")
    if paths["warnings"].exists():
        runner.log(f"Warning log: {paths['warnings']}")
    runner.log(f"Doxygen warning count: {warnings}")
    if not ok:
        runner.log("\n" + recommendations())
        return False
    if fail_on_warnings and warnings:
        runner.log("FAILED: --fail-on-warnings was set and Doxygen reported warnings.")
        return False
    return True


def open_index(output: Path) -> bool:
    """Open the generated HTML index with the user's default browser."""

    index = output_paths(output)["index"]
    if not index.exists():
        return False
    webbrowser.open(index.resolve().as_uri())
    return True


def parse_args(argv: list[str]) -> argparse.Namespace:
    """Parse CLI options for documentation generation."""

    parser = argparse.ArgumentParser(description="Generate SSTL Doxygen documentation.")
    parser.add_argument("--backend", choices=BACKENDS, default="auto")
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT), help="Output directory for generated documentation.")
    parser.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT_SECONDS)
    parser.add_argument("--no-output-timeout", type=int, default=NO_OUTPUT_TIMEOUT_SECONDS)
    parser.add_argument("--yes-virtualize", action="store_true", help="Consent to Docker/Podman/WSL in auto mode.")
    parser.add_argument("--no-virtualize", action="store_true", help="Disallow Docker/Podman/WSL in auto mode.")
    parser.add_argument("--wsl-distro", default=os.environ.get("SSTL_WSL_DISTRO", ""), help="Use a specific WSL distro.")
    parser.add_argument("--install-missing", action="store_true", help="Print setup recommendations and ask before running the first safe installer.")
    parser.add_argument("--yes-install", action="store_true", help="Run the recommended installer without an interactive prompt.")
    parser.add_argument("--prepare-backends", action="store_true", help="Try to make installed Docker/Podman/WSL backends available.")
    parser.add_argument("--yes-prepare", action="store_true", help="Run backend preparation without an interactive prompt.")
    parser.add_argument("--fail-on-warnings", action="store_true", help="Return failure if Doxygen reports warnings.")
    parser.add_argument("--open", action="store_true", help="Open the selected output directory's html/index.html after success.")
    return parser.parse_args(argv)


def cli(argv: list[str]) -> int:
    """Run the command-line interface."""

    args = parse_args(argv)
    if args.yes_virtualize and args.no_virtualize:
        print("Choose only one of --yes-virtualize or --no-virtualize.", file=sys.stderr)
        return 2
    virtualize: bool | None
    if args.backend in {"docker", "podman", "wsl"}:
        virtualize = True
    elif args.yes_virtualize:
        virtualize = True
    elif args.no_virtualize:
        virtualize = False
    else:
        virtualize = None

    runner = Runner(args.timeout, args.no_output_timeout)
    if args.prepare_backends:
        ok = prepare_selected_backends(
            runner,
            args.backend,
            args.yes_prepare,
            install_missing=args.install_missing,
            assume_yes_install=args.yes_install,
            wsl_distro=args.wsl_distro,
        )
        return 0 if ok else 1
    if args.install_missing:
        ok = maybe_install_missing(runner, args.backend, args.yes_install)
        return 0 if ok else 1

    ok = run_backend(runner, args.backend, Path(args.output).resolve(), virtualize, args.fail_on_warnings, args.wsl_distro)
    if ok and args.open:
        if not open_index(Path(args.output).resolve()):
            print("Documentation index was not found.", file=sys.stderr)
            return 1
    return 0 if ok else 1


def gui() -> int:
    """Run the double-click GUI."""

    try:
        import tkinter as tk
        from tkinter import messagebox, scrolledtext
    except Exception as exc:
        print(f"Could not start GUI: {exc}", file=sys.stderr)
        return cli(["--backend", "auto"])

    root = tk.Tk()
    root.title("SSTL Doxygen Runner")
    root.geometry("1000x680")

    output = DEFAULT_OUTPUT
    log_box = scrolledtext.ScrolledText(root, wrap=tk.WORD, font=("Consolas", 10))
    attach_text_copy_context_menu(log_box)
    log_box.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

    state: dict[str, Runner | None] = {"runner": None}
    buttons: list[tk.Button] = []
    docker_available, _, docker_reason = container_engine_probe("docker", timeout=10)
    podman_available, _, podman_reason = container_engine_probe("podman", timeout=10)
    local_available = doxygen_available() is not None
    wsl_available = select_wsl_doxygen_distro() is not None

    def write(text: str) -> None:
        log_box.insert(tk.END, text + "\n")
        log_box.see(tk.END)
        root.update_idletasks()

    def docs_available() -> bool:
        return output_paths(output)["index"].exists()

    def set_running(running: bool) -> None:
        auto_button.config(state=tk.DISABLED if running else tk.NORMAL)
        local_button.config(
            text=availability_label("Local Doxygen", local_available),
            state=tk.DISABLED if running or not local_available else tk.NORMAL,
        )
        docker_button.config(
            text=availability_label("Docker Doxygen", docker_available),
            state=tk.DISABLED if running or not docker_available else tk.NORMAL,
        )
        podman_button.config(
            text=availability_label("Podman Doxygen", podman_available),
            state=tk.DISABLED if running or not podman_available else tk.NORMAL,
        )
        wsl_button.config(
            text=availability_label("WSL Doxygen", wsl_available),
            state=tk.DISABLED if running or not wsl_available else tk.NORMAL,
        )
        for button in (install_button, prepare_button, help_button):
            button.config(state=tk.DISABLED if running else tk.NORMAL)
        open_button.config(state=tk.DISABLED if running or not docs_available() else tk.NORMAL)
        cancel_button.config(state=tk.NORMAL if running else tk.DISABLED)

    def refresh_backend_availability() -> None:
        nonlocal docker_available, docker_reason, podman_available, podman_reason, local_available, wsl_available
        docker_available, _, docker_reason = container_engine_probe("docker", timeout=10)
        podman_available, _, podman_reason = container_engine_probe("podman", timeout=10)
        local_available = doxygen_available() is not None
        wsl_available = select_wsl_doxygen_distro() is not None
        set_running(False)

    def start(backend: str, install_missing: bool = False, prepare: bool = False) -> None:
        consent: bool | None = None
        if install_missing:
            set_running(True)
            runner = Runner(DEFAULT_TIMEOUT_SECONDS, NO_OUTPUT_TIMEOUT_SECONDS, log_func=write)
            state["runner"] = runner
            runner.log("GUI selected action: install recommendations")
            runner.log("GUI install flow: recommendations will be printed here.")
            runner.log("For safety, run an installer from a terminal with:")
            runner.log("  python tools/sstl_run_doxygen.py --backend auto --install-missing")
            print_recommendations(runner, backend)
            set_running(False)
            messagebox.showinfo("SSTL Doxygen Runner", "Install recommendations printed. No commands were run.")
            return
        if prepare:
            if not messagebox.askyesno(
                "Prepare Backends",
                "Try to make installed Doxygen backends available now?\n\n"
                "This can start Docker Desktop, initialize or start a Podman machine, "
                "and install Doxygen/Graphviz inside a suitable WSL distro.",
                default=messagebox.NO,
            ):
                return
        elif backend == "auto":
            consent: bool | None = None
            if not doxygen_available():
                consent = messagebox.askyesno(
                    "Virtualized Doxygen",
                    "Local Doxygen was not found. May the script try Docker, Podman, or WSL?",
                )
        elif backend in {"docker", "podman", "wsl"}:
            consent = True
        else:
            consent = False

        set_running(True)
        runner = Runner(DEFAULT_TIMEOUT_SECONDS, NO_OUTPUT_TIMEOUT_SECONDS, log_func=write)
        state["runner"] = runner

        def worker() -> None:
            if prepare:
                runner.log("GUI selected action: prepare backends")
                runner.log("GUI selected backend: " + backend)
                ok = prepare_selected_backends(runner, backend, True)
            else:
                runner.log("GUI selected action: Doxygen generation")
                runner.log("GUI selected backend: " + backend)
                ok = run_backend(runner, backend, output, consent, False)
            def done() -> None:
                if prepare:
                    refresh_backend_availability()
                set_running(False)
                if ok and prepare:
                    messagebox.showinfo("SSTL Doxygen Runner", "Backend preparation finished. Availability buttons were refreshed.")
                elif ok:
                    messagebox.showinfo("SSTL Doxygen Runner", "Doxygen documentation was generated locally.")
                    open_docs()
                else:
                    message = "Backend preparation failed or was cancelled." if prepare else "Doxygen generation failed or was cancelled."
                    messagebox.showerror("SSTL Doxygen Runner", message)
            root.after(0, done)

        threading.Thread(target=worker, daemon=True).start()

    def cancel() -> None:
        runner = state.get("runner")
        if runner:
            runner.cancel()

    def show_help() -> None:
        messagebox.showinfo("SSTL Doxygen Runner Help", HELP_TEXT)

    def open_docs() -> None:
        if not open_index(output):
            messagebox.showwarning("SSTL Doxygen Runner", "No generated HTML index exists yet.")

    controls = tk.Frame(root)
    controls.pack(fill=tk.X, padx=8, pady=(0, 8))

    for column in range(11):
        controls.grid_columnconfigure(column, weight=0)
    controls.grid_columnconfigure(5, minsize=12)

    def place(button: "tk.Button", column: int, padx: tuple[int, int] = (0, 6)) -> None:
        button.grid(row=0, column=column, sticky="ew", padx=padx, pady=(0, 4))
        buttons.append(button)

    auto_button = tk.Button(controls, text="Auto Doxygen", command=lambda: start("auto"))
    place(auto_button, 0)
    local_button = tk.Button(
        controls,
        text=availability_label("Local Doxygen", local_available),
        command=lambda: start("local"),
        state=tk.NORMAL if local_available else tk.DISABLED,
    )
    place(local_button, 1)
    docker_button = tk.Button(
        controls,
        text=availability_label("Docker Doxygen", docker_available),
        command=lambda: start("docker"),
        state=tk.NORMAL if docker_available else tk.DISABLED,
    )
    place(docker_button, 2)
    podman_button = tk.Button(
        controls,
        text=availability_label("Podman Doxygen", podman_available),
        command=lambda: start("podman"),
        state=tk.NORMAL if podman_available else tk.DISABLED,
    )
    place(podman_button, 3)
    wsl_button = tk.Button(
        controls,
        text=availability_label("WSL Doxygen", wsl_available),
        command=lambda: start("wsl"),
        state=tk.NORMAL if wsl_available else tk.DISABLED,
    )
    place(wsl_button, 4)
    install_button = tk.Button(controls, text="Recommend / Install", command=lambda: start("auto", install_missing=True))
    place(install_button, 6)
    prepare_button = tk.Button(controls, text="Prepare Backends", command=lambda: start("auto", prepare=True))
    place(prepare_button, 7)
    open_button = tk.Button(controls, text="Open Docs", command=open_docs)
    place(open_button, 8)
    help_button = tk.Button(controls, text="Help", command=show_help)
    place(help_button, 9)
    cancel_button = tk.Button(controls, text="Cancel", command=cancel, state=tk.DISABLED)
    place(cancel_button, 10, padx=(0, 0))

    write("Choose Auto Doxygen to use local Doxygen first and ask before virtualization.")
    write("Backend-specific buttons are strict and are disabled when that backend is unavailable.")
    if not local_available:
        write("Local Doxygen is disabled because doxygen.exe was not detected.")
    if not docker_available:
        write("Docker Doxygen is disabled: " + container_engine_summary("docker", docker_reason) + ".")
    if not podman_available:
        write("Podman Doxygen is disabled: " + container_engine_summary("podman", podman_reason) + ".")
    if not wsl_available:
        write("WSL Doxygen is disabled because no non-internal WSL distro with Doxygen and Graphviz was detected.")
    if not docs_available():
        write("Open Docs is disabled because no generated HTML index exists yet.")
    write("Recommend / Install prints safe setup commands. Run them from a terminal to confirm installation.")
    write("Prepare Backends asks first, then can start Docker Desktop, start/init Podman, or install WSL Doxygen tools.")
    write(f"Generated docs will be stored under: {output}")
    set_running(False)
    set_minimum_window_size(root, 1000, 680)
    root.mainloop()
    return 0


def main() -> int:
    """Dispatch to GUI for double-click use and CLI when arguments exist."""

    if len(sys.argv) == 1:
        if maybe_relaunch_windows_gui(ROOT, __file__, sys.argv[1:]):
            return 0
        return gui()
    return cli(sys.argv[1:])


if __name__ == "__main__":
    raise SystemExit(main())
