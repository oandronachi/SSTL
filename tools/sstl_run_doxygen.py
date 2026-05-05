#!/usr/bin/env python3
"""Generate local Doxygen documentation for SSTL.

How to run from a terminal:
  python tools/sstl_run_doxygen.py --backend auto
  python tools/sstl_run_doxygen.py --backend local
  python tools/sstl_run_doxygen.py --backend docker --yes-virtualize
  python tools/sstl_run_doxygen.py --backend podman --yes-virtualize
  python tools/sstl_run_doxygen.py --backend wsl --yes-virtualize

How to run from a file explorer:
  Double-click this file. With no command-line arguments it opens a Tkinter GUI
  with buttons for local generation, automatic backend discovery, virtualized
  backends, help, cancellation, and opening the generated HTML index.

What the script does:
  - Writes a deterministic Doxyfile under artifacts/doxygen.
  - Uses local doxygen first when --backend auto is selected.
  - Uses Docker, Podman, or WSL only when the user explicitly chose that backend
    or consented through --yes-virtualize, a CLI prompt, or the GUI prompt.
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
import subprocess
import sys
import threading
import time
import webbrowser


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent if SCRIPT_DIR.name == "tools" else SCRIPT_DIR
ARTIFACT_ROOT = ROOT / "artifacts"
DEFAULT_OUTPUT = ARTIFACT_ROOT / "doxygen"
DOCKER_IMAGE = "gcc:13-bookworm"
BACKENDS = ("auto", "local", "docker", "podman", "wsl")
DEFAULT_TIMEOUT_SECONDS = 20 * 60
NO_OUTPUT_TIMEOUT_SECONDS = 2 * 60

HELP_TEXT = """SSTL Doxygen Runner

Terminal examples:
  python tools/sstl_run_doxygen.py --backend auto
  python tools/sstl_run_doxygen.py --backend local
  python tools/sstl_run_doxygen.py --backend docker --yes-virtualize
  python tools/sstl_run_doxygen.py --backend auto --fail-on-warnings

Backend behavior:
  auto    Try local Doxygen first. If local Doxygen is missing, ask before
          trying Docker, Podman, or WSL.
  local   Use doxygen from PATH and write artifacts/doxygen locally.
  docker  Run Doxygen in a disposable Linux container.
  podman  Same as Docker, using Podman.
  wsl     Use Doxygen already installed inside WSL.

Double-click behavior:
  With no command-line arguments, this script opens the GUI. Click Auto Backend
  for normal discovery, Local Doxygen to avoid virtualized environments, or one
  of Docker/Podman/WSL when you explicitly want a virtualized run.

Generated files:
  artifacts/doxygen/Doxyfile
  artifacts/doxygen/html/index.html
  artifacts/doxygen/doxygen-warnings.log
  artifacts/doxygen/doxygen_summary.yaml
"""


def shell_quote(value: str) -> str:
    """Return a conservative POSIX-shell single-quoted string."""

    return "'" + value.replace("'", "'\"'\"'") + "'"


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
        self.log(" ".join(cmd))
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

    return shutil.which("doxygen")


def executable_available(name: str) -> str | None:
    """Return an executable path when the named backend launcher is on PATH."""

    return shutil.which(name)


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

    launcher = executable_available(engine)
    if not launcher:
        runner.log(f"{engine} was not found on PATH.")
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
            f"{ROOT}:/work",
            "-w",
            "/work",
            DOCKER_IMAGE,
            "bash",
            "-lc",
            inner,
        ],
        ROOT,
    )


def windows_path_to_wsl(path: Path) -> str | None:
    """Ask WSL to translate a Windows path into its Linux mount path."""

    wsl = executable_available("wsl")
    if not wsl:
        return None
    try:
        completed = subprocess.run(
            [wsl, "wslpath", "-a", str(path)],
            cwd=str(ROOT),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=20,
            check=False,
        )
    except Exception:
        return None
    if completed.returncode == 0 and completed.stdout.strip():
        return completed.stdout.strip()
    return None


def wsl_backend(runner: Runner, output: Path) -> bool:
    """Generate documentation with Doxygen already installed inside WSL."""

    if os.name != "nt":
        runner.log("WSL backend is only available from Windows hosts.")
        return False
    wsl = executable_available("wsl")
    if not wsl:
        runner.log("wsl was not found on PATH.")
        return False
    wroot = windows_path_to_wsl(ROOT)
    if not wroot:
        runner.log("Could not translate the workspace path for WSL.")
        return False
    doxyfile = write_doxyfile(output)
    command = (
        f"cd {shell_quote(wroot)} && "
        "if ! command -v doxygen >/dev/null 2>&1; then "
        "echo 'Doxygen is not installed inside WSL. Install doxygen and graphviz there, or use Docker/Podman.'; "
        "exit 3; "
        "fi; "
        f"doxygen {shell_quote(doxyfile.relative_to(ROOT).as_posix())}"
    )
    return runner.run("WSL Doxygen generation", [wsl, "sh", "-lc", command], ROOT)


def recommendations() -> str:
    """Return practical setup guidance when no backend can run."""

    return "\n".join(
        [
            "No usable Doxygen backend completed.",
            "",
            "Recommended options, easiest first:",
            "  1. Install Doxygen locally and rerun: python tools/sstl_run_doxygen.py --backend local",
            "  2. Start Docker Desktop, then rerun: python tools/sstl_run_doxygen.py --backend docker --yes-virtualize",
            "  3. Start Podman, then rerun: python tools/sstl_run_doxygen.py --backend podman --yes-virtualize",
            "  4. Install Doxygen inside WSL, then rerun: python tools/sstl_run_doxygen.py --backend wsl --yes-virtualize",
        ]
    )


def ask_yes_no(prompt: str, default: bool = False) -> bool:
    """Prompt the CLI user for consent when stdin is interactive."""

    if not sys.stdin.isatty():
        return default
    suffix = " [Y/n]: " if default else " [y/N]: "
    answer = input(prompt + suffix).strip().lower()
    if not answer:
        return default
    return answer in {"y", "yes"}


def run_backend(
    runner: Runner,
    backend: str,
    output: Path,
    virtualize: bool | None,
    fail_on_warnings: bool,
) -> bool:
    """Run the requested backend and emit a summary file."""

    paths = output_paths(output)
    paths["root"].mkdir(parents=True, exist_ok=True)
    ok = False
    used_backend = backend

    if backend == "local":
        ok = local_backend(runner, output)
        used_backend = "local"
    elif backend in {"docker", "podman"}:
        ok = container_backend(runner, output, backend)
        used_backend = backend
    elif backend == "wsl":
        ok = wsl_backend(runner, output)
        used_backend = "wsl"
    elif backend == "auto":
        if local_backend(runner, output):
            ok = True
            used_backend = "local"
        else:
            consent = virtualize
            if consent is None:
                consent = ask_yes_no("Local Doxygen is unavailable. Try a virtualized backend?", False)
            if not consent:
                runner.log("Virtualized Doxygen generation was not consented; stopping after local discovery.")
            else:
                for candidate in ("docker", "podman", "wsl"):
                    runner.log(f"\n-- Trying {candidate} backend")
                    if candidate in {"docker", "podman"}:
                        ok = container_backend(runner, output, candidate)
                    else:
                        ok = wsl_backend(runner, output)
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
    parser.add_argument("--fail-on-warnings", action="store_true", help="Return failure if Doxygen reports warnings.")
    parser.add_argument("--open", action="store_true", help="Open artifacts/doxygen/html/index.html after success.")
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
    ok = run_backend(runner, args.backend, Path(args.output).resolve(), virtualize, args.fail_on_warnings)
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
    log_box.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

    state: dict[str, Runner | None] = {"runner": None}
    buttons: list[tk.Button] = []

    def write(text: str) -> None:
        log_box.insert(tk.END, text + "\n")
        log_box.see(tk.END)
        root.update_idletasks()

    def set_running(running: bool) -> None:
        for button in buttons:
            button.config(state=tk.DISABLED if running else tk.NORMAL)
        cancel_button.config(state=tk.NORMAL if running else tk.DISABLED)

    def start(backend: str) -> None:
        if backend == "auto":
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
            ok = run_backend(runner, backend, output, consent, False)
            def done() -> None:
                set_running(False)
                if ok:
                    messagebox.showinfo("SSTL Doxygen Runner", "Doxygen documentation was generated locally.")
                else:
                    messagebox.showerror("SSTL Doxygen Runner", "Doxygen generation failed or was cancelled.")
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
    for label, backend in (
        ("Auto Backend", "auto"),
        ("Local Doxygen", "local"),
        ("Docker", "docker"),
        ("Podman", "podman"),
        ("WSL", "wsl"),
    ):
        button = tk.Button(controls, text=label, command=lambda b=backend: start(b))
        button.pack(side=tk.LEFT, padx=(0, 6))
        buttons.append(button)
    open_button = tk.Button(controls, text="Open Docs", command=open_docs)
    open_button.pack(side=tk.LEFT, padx=(8, 6))
    buttons.append(open_button)
    help_button = tk.Button(controls, text="Help", command=show_help)
    help_button.pack(side=tk.LEFT, padx=(0, 6))
    buttons.append(help_button)
    cancel_button = tk.Button(controls, text="Cancel", command=cancel, state=tk.DISABLED)
    cancel_button.pack(side=tk.LEFT)

    write("Choose Auto Backend to use local Doxygen first and ask before virtualization.")
    write(f"Generated docs will be stored under: {output}")
    root.mainloop()
    return 0


def main() -> int:
    """Dispatch to GUI for double-click use and CLI when arguments exist."""

    if len(sys.argv) == 1:
        return gui()
    return cli(sys.argv[1:])


if __name__ == "__main__":
    raise SystemExit(main())
