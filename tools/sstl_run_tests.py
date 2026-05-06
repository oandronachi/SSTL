#!/usr/bin/env python3
"""Compile and run the SSTL C/C++ tests.

The script is intentionally self-contained and does not emulate any C or C++
test in Python. It invokes CMake/CTest to build and execute the actual test
programs.

Behavior:
  - From a terminal, it behaves as a CLI.
  - If command-line arguments are supplied, it always behaves as a CLI, even
    when launched from an environment whose stdout is not a real TTY.
  - When double-clicked with no arguments, it opens a small Tkinter GUI.
  - In auto mode, it discovers an available backend: Docker, Podman, WSL, then
    local CMake/CTest.
  - Coverage can be run through auto fallback or forced to Docker, Podman, WSL,
    or local coverage when that backend has the required tools.
  - If no suitable backend exists, it prints OS-specific install
    recommendations ranked by ease of setup.

Guardrails:
  - Every external command has an overall timeout.
  - A watchdog terminates commands that produce no output for too long.
  - Backend fallback is automatic in auto mode.
  - Output is streamed live so a stuck command is visible.
  - Runtime comparison CSV artifacts can be opened as a built-in GUI chart.

Common terminal commands:
  # Recommended: discover and try Docker, Podman, WSL, then local CMake/CTest.
  python tools/sstl_run_tests.py --backend auto

  # Use one specific backend.
  python tools/sstl_run_tests.py --backend docker
  python tools/sstl_run_tests.py --backend podman
  python tools/sstl_run_tests.py --backend wsl
  python tools/sstl_run_tests.py --backend wsl --wsl-distro Ubuntu
  python tools/sstl_run_tests.py --backend local

  # If nothing usable is installed, print recommendations and ask before
  # running the easiest safe installer command for the current OS.
  python tools/sstl_run_tests.py --backend auto --install-missing

  # Non-interactive install flow for automation.
  python tools/sstl_run_tests.py --backend auto --install-missing --yes-install

  # Try to make installed backends available. This can start Docker Desktop,
  # initialize/start a Podman machine, or install validation/coverage tools inside WSL.
  python tools/sstl_run_tests.py --backend auto --prepare-backends

  # Tighter guardrails for slow or suspicious runs.
  python tools/sstl_run_tests.py --backend auto --timeout 900 --no-output-timeout 60

  # Runtime comparison writes sstl_runtime_interface_comparison.csv and copies
  # the newest CSV into artifacts/. In the GUI, click Runtime Chart to
  # visualize the newest CSV artifact.
  # In the CLI, print a text report from the newest runtime CSV artifact.
  python tools/sstl_run_tests.py --runtime-report-only
  python tools/sstl_run_tests.py --backend auto --runtime-report
  python tools/sstl_run_tests.py --runtime-report-only --color always

  # Print a compact dashboard from the latest command results and artifacts.
  python tools/sstl_run_tests.py --quick-summary-only
  python tools/sstl_run_tests.py --backend auto --quick-summary

  # Generate and show line/branch coverage, overall and per file.
  python tools/sstl_run_tests.py --backend docker --coverage-only
  python tools/sstl_run_tests.py --backend auto --coverage
  # WSL coverage reports thresholds but does not gate on them, because host
  # distro compiler versions can emit different synthetic branch counters.

  # Run optional vendor compiler container image probes after the normal backend.
  # Missing configured images ask before pull by default.
  python tools/sstl_run_tests.py --backend podman --vendor-container-images
  python tools/sstl_run_tests.py --vendor-container-nominees
  python tools/sstl_run_tests.py --vendor-container-images --vendor-container-image arm-none-eabi=my/arm-gcc:latest
"""

from __future__ import annotations

import argparse
import csv
import os
from pathlib import Path
import queue
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from dataclasses import dataclass
from datetime import datetime, timezone

from sstl_tool_common import (
    InstallOption,
    ask_yes_no,
    attach_text_copy_context_menu,
    container_mount_arg_for_root,
    container_engine_probe,
    container_engine_summary,
    decode_wsl_output,
    display_command_arg_for_root,
    find_container_engine_executable,
    find_docker_desktop_executable,
    find_named_executable,
    find_on_path,
    maybe_relaunch_windows_gui,
    parent_is_known_terminal,
    podman_machine_exists,
    repo_root_from_script,
    terminal_like_launch,
    set_minimum_window_size,
    windows_program_roots,
    wsl_command,
    wsl_default_distro,
    wsl_distro_is_internal,
    wsl_installed_distros,
)


ROOT = repo_root_from_script(__file__)
TEST_ROOT = ROOT / "testing"
SSTL_ROOT = ROOT
ARTIFACT_ROOT = ROOT / "artifacts"
DOCKER_IMAGE = "gcc:13-bookworm"

DEFAULT_TIMEOUT_SECONDS = 30 * 60
NO_OUTPUT_TIMEOUT_SECONDS = 5 * 60
RUNTIME_CSV_NAME = "sstl_runtime_interface_comparison.csv"
COVERAGE_INFO_NAME = "sstl_coverage.filtered.info"
COVERAGE_LCOV_CAPTURE_LOG_NAME = "sstl_coverage.capture.lcov.log"
COVERAGE_LCOV_FILTER_LOG_NAME = "sstl_coverage.filter.lcov.log"
WSL_TEST_TOOL_INSTALL = "sudo apt-get update && sudo apt-get install -y cmake ninja-build gcc g++ clang libclang-rt-dev util-linux python3 lcov"
WSL_COVERAGE_TOOL_INSTALL = "sudo apt-get update && sudo apt-get install -y cmake ninja-build gcc g++ lcov"
WSL_TEST_TOOL_INSTALL_ROOT = "DEBIAN_FRONTEND=noninteractive apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y cmake ninja-build gcc g++ clang libclang-rt-dev util-linux python3 lcov"
COVERAGE_LINE_MIN = 0.95
COVERAGE_BRANCH_MIN = 0.90
COVERAGE_SOURCE_PREFIXES = ("include/sstl/",)
COVERAGE_BRANCH_POLICY = "LCOV capture excludes compiler-generated exception branches"
LOCAL_SANITY_TIMEOUT_SECONDS = 120
# LCOV's branch counter includes synthetic C++ exception/unwind edges unless
# geninfo is told to drop them at capture time. The filtered .info file only
# keeps plain BRDA records, so this has to happen during capture rather than in
# the later Python parser.
LCOV_BRANCH_RC_ARGS = ["--rc", "lcov_branch_coverage=1", "--rc", "geninfo_no_exception_branch=1"]
LCOV_BRANCH_RC_SHELL = "--rc lcov_branch_coverage=1 --rc geninfo_no_exception_branch=1"

BACKENDS = ("auto", "docker", "podman", "wsl", "local")
VENDOR_CONTAINER_NOMINEES = {
    "wischner-arm-none-eabi-1_1_0": (
        "arm-none-eabi",
        "wischner_1_1_0",
        "wischner/gcc-arm-none-eabi:1.1.0",
    ),
    "jafee-arm-none-eabi-15_2_rel1": (
        "arm-none-eabi",
        "jafee_15_2_rel1",
        "jafee201153/arm-none-eabi-gcc:15.2.Rel1",
    ),
    "jafee-arm-none-eabi-14_3_rel1": (
        "arm-none-eabi",
        "jafee_14_3_rel1",
        "jafee201153/arm-none-eabi-gcc:14.3.Rel1",
    ),
    "gonzarub-arm-none-eabi-13_3": (
        "arm-none-eabi",
        "gonzarub_13_3",
        "gonzarub/gcc-arm-none-eabi:toolchain-13.3",
    ),
}

HELP_TEXT = """How to run SSTL tests

Recommended terminal command:
  python tools/sstl_run_tests.py --backend auto

Backend choices:
  auto    Discover and try Docker, Podman, WSL, then local CMake/CTest.
          Container backends are selected only when their engine/socket is
          reachable.
  docker  Run the Linux GCC validation lanes in Docker.
          Requires Docker Desktop/daemon to be running.
  podman  Run the same containerized lanes through Podman.
          Requires a reachable Podman machine/socket.
  wsl     On Windows, run the Linux GCC lanes inside WSL.
          Use --wsl-distro Ubuntu or SSTL_WSL_DISTRO=Ubuntu to avoid
          Docker Desktop's internal WSL distro. Requires cmake, ninja,
          gcc/g++, and clang/clang++ inside that distro.
  local   Run host-machine CMake/CTest lanes only after a tiny C/C++ configure
          and build sanity probe passes. On Windows this can use Visual Studio,
          LLVM, MinGW, or any other CMake-supported local compiler.

If no backend or toolchain is available:
  python tools/sstl_run_tests.py --backend auto --install-missing

The script will print recommendations and ask before running any installer.

To try to make installed but unavailable backends ready:
  python tools/sstl_run_tests.py --backend auto --prepare-backends
  python tools/sstl_run_tests.py --backend podman --prepare-backends --yes-prepare
  python tools/sstl_run_tests.py --backend wsl --prepare-backends --wsl-distro Ubuntu

This may start Docker Desktop, initialize/start a Podman machine, or install
required tools inside WSL. The GUI exposes the same flow as Prepare Backends.
Use --yes-prepare for non-interactive preparation. Use --install-missing and
--yes-install separately if you also want to install missing host applications.

Guardrail tuning:
  python tools/sstl_run_tests.py --backend auto --timeout 900 --no-output-timeout 60

Runtime observability:
  The runtime comparison writes sstl_runtime_interface_comparison.csv in the
  active test working directory and copies the newest CSV into artifacts/.
  Click Runtime Chart to visualize the newest CSV artifact as CPU and memory
  bars.
  python tools/sstl_run_tests.py --runtime-report-only
  python tools/sstl_run_tests.py --backend auto --runtime-report

  The runtime report is shown overall by family and per comparison. Normal
  benchmark artifacts are preferred over coverage-instrumented artifacts.
  Add --color always to force ANSI colors, or --color never for plain logs.

Quick summary:
  python tools/sstl_run_tests.py --quick-summary-only
  python tools/sstl_run_tests.py --backend auto --quick-summary

  The quick summary combines current command results, latest coverage status,
  latest runtime status, weak coverage files, and runtime family totals. In the
  GUI, click Quick Summary for the same first-glance dashboard.

Coverage observability:
  python tools/sstl_run_tests.py --backend docker --coverage-only
  python tools/sstl_run_tests.py --backend auto --coverage

  The coverage report is shown overall and per file. The newest coverage info
  is copied into artifacts/. In the GUI, use the Coverage row to run auto
  coverage or force Docker, Podman, WSL, or local coverage; the report opens
  after a successful coverage run. Local coverage additionally requires lcov
  and a GNU/Clang-style compiler coverage sanity probe.
  WSL coverage reports thresholds, but does not gate the command result on
  branch coverage because host distro GCC/LCOV versions emit different
  synthetic branch counters than the canonical Docker/local lanes.

Vendor compiler container probes:
  python tools/sstl_run_tests.py --backend docker --vendor-container-images
  python tools/sstl_run_tests.py --backend podman --vendor-container-images
  python tools/sstl_run_tests.py --backend local --vendor-container-images --vendor-container-engine podman
  python tools/sstl_run_tests.py --list-vendor-container-nominees
  python tools/sstl_run_tests.py --vendor-container-nominees
  python tools/sstl_run_tests.py --vendor-container-nominee wischner-arm-none-eabi-1_1_0
  python tools/sstl_run_tests.py --vendor-container-images --vendor-container-image arm-none-eabi=my/arm-gcc:latest

  Image keys are armclang, arm-none-eabi, keil-armcc, and iar-iccarm. You can
  pass KEY=IMAGE or KEY@LABEL=IMAGE. The predefined public arm-none-eabi
  nominees are wischner-arm-none-eabi-1_1_0, jafee-arm-none-eabi-15_2_rel1,
  jafee-arm-none-eabi-14_3_rel1, and gonzarub-arm-none-eabi-13_3. You can also
  configure images with SSTL_VENDOR_CONTAINER_ARMCLANG_IMAGE,
  SSTL_VENDOR_CONTAINER_ARM_NONE_EABI_IMAGE,
  SSTL_VENDOR_CONTAINER_KEIL_ARMCC_IMAGE, and
  SSTL_VENDOR_CONTAINER_IAR_ICCARM_IMAGE.

  Pull policy is controlled separately from --install-missing:
    --vendor-container-pull ask     Ask before pulling missing images.
    --vendor-container-pull never   Skip lanes whose images are absent.
    --vendor-container-pull always  Pull configured missing images without a prompt.

  Pull denial, unavailable approval, or network/pull failure records the
  dependent vendor image lanes as skipped in vendor_compiler_summary.yaml.
  --install-missing only covers runner backends/tooling such as Docker, Podman,
  WSL, CMake, and Ninja; it does not install proprietary compiler images.

Path handling:
  Runner commands use repository-relative paths where the backend/tool accepts
  them. WSL derives the checkout path from its inherited current directory.
  Docker/Podman bind mounts still use a runtime-derived host path internally,
  because the engine needs a concrete mount source; logs display it as .:/work.

Double-click behavior:
  The GUI opens. Click Auto Fallback (Recommended) for normal discovery,
  Local to avoid virtualized environments, or Recommend / Install to see
  setup commands. Prepare Backends can start installed Docker/Podman services
  or install WSL validation/coverage tools after confirmation. Coverage buttons
  sit directly under their matching backend buttons and can run auto or a
  strict selected backend.
  Strict backend buttons are disabled when their backend or required tooling is
  not available. Local and Local Coverage availability include the sanity
  probes described above, not just executable discovery.
"""

ANSI_RESET = "\033[0m"
ANSI = {
    "bold": "\033[1m",
    "dim": "\033[2m",
    "red": "\033[31m",
    "green": "\033[32m",
    "yellow": "\033[33m",
    "cyan": "\033[36m",
}


def color_enabled_from_mode(mode: str) -> bool:
    if mode == "always":
        return True
    if mode == "never":
        return False
    if os.environ.get("NO_COLOR"):
        return False
    if os.environ.get("TERM", "").lower() == "dumb":
        return False
    return sys.stdout.isatty()


def color_text(text: str, color: str, enabled: bool) -> str:
    if not enabled:
        return text
    return ANSI.get(color, "") + text + ANSI_RESET


def color_status(status: str, enabled: bool) -> str:
    if status == "pass":
        return color_text(status, "green", enabled)
    if status == "fail":
        return color_text(status, "red", enabled)
    return color_text(status, "yellow", enabled)


def color_pct(value: float, threshold: float, enabled: bool) -> str:
    text_value = pct_text(value)
    return color_text(text_value, "green" if value >= threshold else "red", enabled)


def color_diff(value: float, enabled: bool) -> str:
    text_value = f"{value:9.2f}%"
    if abs(value) < 0.005:
        return color_text(text_value, "dim", enabled)
    return color_text(text_value, "green" if value < 0.0 else "red", enabled)


@dataclass
class RunResult:
    name: str
    ok: bool
    returncode: int | None
    detail: str = ""


@dataclass
class CoverageFile:
    path: str
    lines_found: int = 0
    lines_hit: int = 0
    branches_found: int = 0
    branches_hit: int = 0

    @property
    def line_pct(self) -> float:
        return self.lines_hit / self.lines_found if self.lines_found else 0.0

    @property
    def branch_pct(self) -> float:
        return self.branches_hit / self.branches_found if self.branches_found else 0.0

    @property
    def status(self) -> str:
        line_ok = self.line_pct >= COVERAGE_LINE_MIN
        branch_ok = self.branch_pct >= COVERAGE_BRANCH_MIN if self.branches_found else True
        return "pass" if line_ok and branch_ok else "fail"


@dataclass
class CoverageReport:
    source: Path
    files: list[CoverageFile]

    @property
    def lines_found(self) -> int:
        return sum(item.lines_found for item in self.files)

    @property
    def lines_hit(self) -> int:
        return sum(item.lines_hit for item in self.files)

    @property
    def branches_found(self) -> int:
        return sum(item.branches_found for item in self.files)

    @property
    def branches_hit(self) -> int:
        return sum(item.branches_hit for item in self.files)

    @property
    def line_pct(self) -> float:
        return self.lines_hit / self.lines_found if self.lines_found else 0.0

    @property
    def branch_pct(self) -> float:
        return self.branches_hit / self.branches_found if self.branches_found else 0.0

    @property
    def status(self) -> str:
        return "pass" if self.line_pct >= COVERAGE_LINE_MIN and self.branch_pct >= COVERAGE_BRANCH_MIN else "fail"


class CommandTimeout(RuntimeError):
    pass


class Runner:
    def __init__(
        self,
        dry_run: bool = False,
        timeout_seconds: int = DEFAULT_TIMEOUT_SECONDS,
        no_output_timeout_seconds: int = NO_OUTPUT_TIMEOUT_SECONDS,
        color_enabled: bool = False,
    ) -> None:
        self.dry_run = dry_run
        self.timeout_seconds = timeout_seconds
        self.no_output_timeout_seconds = no_output_timeout_seconds
        self.color_enabled = color_enabled
        self.results: list[RunResult] = []
        self.cancelled = False
        self.in_expected_mutation_failure_section = False
        self.last_successful_backend: str | None = None
        self.wsl_distro = os.environ.get("SSTL_WSL_DISTRO", "").strip()
        self.resolved_wsl_distro: str | None = None
        self.resolved_wsl_coverage_distro: str | None = None

    def log(self, message: str) -> None:
        print(message, flush=True)

    def display_cmd(self, cmd: list[str]) -> str:
        return " ".join(display_command_arg(arg) for arg in cmd)

    def cancel(self) -> None:
        self.cancelled = True

    def run(self, name: str, cmd: list[str], cwd: Path, timeout: int | None = None) -> bool:
        self.log("")
        self.log("== " + name)
        self.log("+ " + self.display_cmd(cmd))
        if self.dry_run:
            self.results.append(RunResult(name, True, 0, "dry-run"))
            return True
        if self.cancelled:
            self.results.append(RunResult(name, False, None, "cancelled before start"))
            return False

        timeout = timeout or self.timeout_seconds
        start = time.monotonic()
        last_output = start
        proc = subprocess.Popen(
            cmd,
            cwd=str(cwd),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            bufsize=1,
        )
        assert proc.stdout is not None

        output_queue: "queue.Queue[str | None]" = queue.Queue()

        def reader() -> None:
            try:
                for line in proc.stdout:
                    output_queue.put(line)
            finally:
                output_queue.put(None)

        reader_thread = threading.Thread(target=reader, daemon=True)
        reader_thread.start()

        saw_eof = False
        detail = ""
        while True:
            now = time.monotonic()
            if self.cancelled:
                detail = "cancelled"
                self._terminate(proc)
                break
            if now - start > timeout:
                detail = f"timed out after {timeout} seconds"
                self._terminate(proc)
                break
            if now - last_output > self.no_output_timeout_seconds:
                detail = f"no output for {self.no_output_timeout_seconds} seconds"
                self._terminate(proc)
                break

            try:
                item = output_queue.get(timeout=0.25)
            except queue.Empty:
                if proc.poll() is not None and saw_eof:
                    break
                continue

            if item is None:
                saw_eof = True
                if proc.poll() is not None:
                    break
                continue

            last_output = time.monotonic()
            line = item.rstrip("\n")
            self._update_output_context(line)
            self.log(self._decorate_output_line(line))

        rc = proc.wait(timeout=5) if proc.poll() is not None else None
        ok = rc == 0 and not detail
        if detail and rc is None:
            rc = -1
        self.results.append(RunResult(name, ok, rc, detail))
        if ok:
            self.log("PASS: " + name)
        else:
            self.log("FAIL: " + name + (f" ({detail})" if detail else f" (exit {rc})"))
        return ok

    def _terminate(self, proc: subprocess.Popen[str]) -> None:
        if proc.poll() is not None:
            return
        try:
            proc.terminate()
            proc.wait(timeout=10)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass

    def _update_output_context(self, line: str) -> None:
        if line.startswith("[MUTATION] The following selected tests are expected to fail"):
            self.in_expected_mutation_failure_section = True
        elif line.startswith("[MUTATION] Mutant killed as expected") or line.startswith("[MUTATION] Mutant survived unexpectedly"):
            self.in_expected_mutation_failure_section = False

    def _decorate_output_line(self, line: str) -> str:
        if self.in_expected_mutation_failure_section:
            if "***Failed" in line or "assertion failed:" in line or re.search(r"tests failed out of", line):
                return "[expected mutant-kill failure] " + line
        return line

    def summary(self) -> int:
        self.log("")
        self.log("== Summary")
        if not self.results:
            self.log("No commands were run.")
            return 1
        ok = True
        for item in self.results:
            status = "PASS" if item.ok else "FAIL"
            status_text = color_text(status, "green" if item.ok else "red", self.color_enabled)
            extra = f" - {item.detail}" if item.detail else ""
            self.log(f"[{status_text}] {item.name}{extra}")
            ok = ok and item.ok
        return 0 if ok else 1


def run_capture(cmd: list[str], timeout: int = 20) -> str:
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True, errors="replace", timeout=timeout)
    except Exception:
        return ""
    return proc.stdout if proc.returncode == 0 else ""


def run_quiet_with_log(runner: Runner, name: str, cmd: list[str], cwd: Path, log_path: Path, timeout: int | None = None) -> bool:
    """Run a normally noisy command while preserving diagnostics in a file.

    LCOV/Geninfo can emit benign packaging warnings on stderr even when
    coverage succeeds. This helper keeps successful runs readable while still
    writing stdout and stderr to an artifact that is printed if the command
    fails or times out.
    """
    runner.log("")
    runner.log("== " + name)
    runner.log("+ " + " ".join(cmd))
    if runner.dry_run:
        runner.results.append(RunResult(name, True, 0, "dry-run"))
        return True
    if runner.cancelled:
        runner.results.append(RunResult(name, False, None, "cancelled before start"))
        return False

    log_path.parent.mkdir(parents=True, exist_ok=True)
    timeout = timeout or runner.timeout_seconds
    try:
        proc = subprocess.run(
            cmd,
            cwd=str(cwd),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            timeout=timeout,
        )
        output = proc.stdout or ""
        log_path.write_text(output, encoding="utf-8", errors="replace")
        ok = proc.returncode == 0
        runner.results.append(RunResult(name, ok, proc.returncode, "" if ok else f"diagnostics: {log_path}"))
        if ok:
            runner.log("PASS: " + name)
        else:
            runner.log(f"FAIL: {name} (exit {proc.returncode}; diagnostics: {log_path})")
            runner.log(log_path.read_text(encoding="utf-8", errors="replace") or "<empty diagnostics log>")
        return ok
    except subprocess.TimeoutExpired as exc:
        parts = []
        if exc.stdout:
            parts.append(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
        if exc.stderr:
            parts.append(exc.stderr if isinstance(exc.stderr, str) else exc.stderr.decode(errors="replace"))
        text = "".join(parts)
        log_path.write_text(text, encoding="utf-8", errors="replace")
        detail = f"timed out after {timeout} seconds; diagnostics: {log_path}"
        runner.results.append(RunResult(name, False, -1, detail))
        runner.log("FAIL: " + name + f" ({detail})")
        runner.log(log_path.read_text(encoding="utf-8", errors="replace") or "<empty diagnostics log>")
        return False


def find_vswhere() -> str | None:
    direct = find_on_path("vswhere")
    if direct:
        return direct
    if os.name != "nt":
        return None
    for root in windows_program_roots():
        candidate = root / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
        if candidate.exists():
            return str(candidate)
    return None


def visual_studio_install_roots() -> list[Path]:
    roots: list[Path] = []
    vswhere = find_vswhere()
    if vswhere:
        out = run_capture([vswhere, "-all", "-products", "*", "-property", "installationPath"])
        for line in out.splitlines():
            p = Path(line.strip())
            if p.exists() and p not in roots:
                roots.append(p)
    if os.name == "nt":
        for program_root in windows_program_roots():
            base = program_root / "Microsoft Visual Studio"
            if base.exists():
                for child in base.glob("*/*"):
                    if child.is_dir() and child not in roots:
                        roots.append(child)
    return roots


def find_cmake() -> str | None:
    roots = visual_studio_install_roots() if os.name == "nt" else []
    return find_named_executable("cmake", roots)


def find_ctest(cmake_path: str | None) -> str | None:
    direct = find_named_executable("ctest", visual_studio_install_roots() if os.name == "nt" else [])
    if direct:
        return direct
    if cmake_path:
        sibling = Path(cmake_path).with_name("ctest.exe" if os.name == "nt" else "ctest")
        if sibling.exists():
            return str(sibling)
    return None


def write_local_sanity_probe_project(source_dir: Path) -> None:
    source_dir.mkdir(parents=True, exist_ok=True)
    (source_dir / "CMakeLists.txt").write_text(
        "\n".join(
            [
                "cmake_minimum_required(VERSION 3.14)",
                "project(SSTLLocalSanity C CXX)",
                "option(SSTL_LOCAL_PROBE_COVERAGE \"Check GNU/Clang lcov coverage flags\" OFF)",
                "if(SSTL_LOCAL_PROBE_COVERAGE)",
                "  if(NOT CMAKE_C_COMPILER_ID MATCHES \"GNU|Clang\" OR NOT CMAKE_CXX_COMPILER_ID MATCHES \"GNU|Clang\")",
                "    message(FATAL_ERROR \"Local coverage requires GNU/Clang-style compilers for lcov capture\")",
                "  endif()",
                "  add_compile_options(--coverage -O0 -g)",
                "  add_link_options(--coverage)",
                "endif()",
                "add_executable(c_probe probe.c)",
                "set_property(TARGET c_probe PROPERTY C_STANDARD 99)",
                "set_property(TARGET c_probe PROPERTY C_STANDARD_REQUIRED ON)",
                "add_executable(cxx_probe probe.cpp)",
                "set_property(TARGET cxx_probe PROPERTY CXX_STANDARD 98)",
                "set_property(TARGET cxx_probe PROPERTY CXX_STANDARD_REQUIRED ON)",
                "",
            ]
        ),
        encoding="utf-8",
    )
    (source_dir / "probe.c").write_text("int main(void) { return 0; }\n", encoding="utf-8")
    (source_dir / "probe.cpp").write_text("int main() { return 0; }\n", encoding="utf-8")


def run_sanity_command(cmd: list[str], timeout: int = LOCAL_SANITY_TIMEOUT_SECONDS) -> tuple[bool, str]:
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
        return False, (output.strip() + f"\nTimed out after {timeout} seconds").strip()
    except Exception as exc:
        return False, str(exc)
    output = (proc.stdout or "").strip()
    return proc.returncode == 0, output or f"exit {proc.returncode}"


def last_sanity_line(output: str) -> str:
    lines = [line.strip() for line in output.splitlines() if line.strip()]
    return lines[-1] if lines else ""


def summarize_sanity_output(output: str) -> str:
    lines = [line.strip() for line in output.splitlines() if line.strip()]
    keywords = ("error", "fatal", "failed", "not found", "denied", "requires")
    for line in reversed(lines):
        lowered = line.lower()
        if lowered in {"-- configuring incomplete, errors occurred!", "build failed."}:
            continue
        if re.fullmatch(r"\d+\s+error\(s\)", line, re.IGNORECASE):
            continue
        if any(keyword in lowered for keyword in keywords):
            return line
    return lines[-1] if lines else "no diagnostic output"


def local_sanity_probe(cmake: str, ctest: str, coverage: bool = False) -> tuple[bool, str]:
    del ctest
    probe_parent = TEST_ROOT / "build"
    try:
        probe_parent.mkdir(parents=True, exist_ok=True)
        marker = probe_parent / (".local_probe_write_check_" + str(os.getpid() % 10000))
        marker.write_text("ok\n", encoding="utf-8")
        marker.unlink()
    except Exception as exc:
        return False, "testing/build cannot create and remove probe files: " + str(exc)
    try:
        probe_root = Path(tempfile.mkdtemp(prefix="sstl-lc-" if coverage else "sstl-lt-"))
    except Exception as exc:
        return False, "could not create local probe build directory: " + str(exc)
    try:
        source_dir = probe_root / "s"
        build_dir = probe_root / "b"
        write_local_sanity_probe_project(source_dir)
        configure_cmd = [
            cmake,
            "-S",
            str(source_dir),
            "-B",
            str(build_dir),
            "-DSSTL_LOCAL_PROBE_COVERAGE=" + ("ON" if coverage else "OFF"),
        ]
        ok, output = run_sanity_command(configure_cmd)
        if not ok:
            return False, "configure failed: " + summarize_sanity_output(output)
        ok, output = run_sanity_command([cmake, "--build", str(build_dir), "--config", "Debug"])
        if not ok:
            return False, "build failed: " + summarize_sanity_output(output)
        return True, "tiny C/C++ coverage probe built" if coverage else "tiny C/C++ probe built"
    finally:
        shutil.rmtree(probe_root, ignore_errors=True)


def executable_available(runner: Runner, exe: str, version_args: list[str] | None = None) -> bool:
    resolved = find_named_executable(exe)
    if not resolved:
        runner.log(f"{exe} executable was not found.")
        return False
    return runner.run(f"{exe} version", [resolved] + (version_args or ["--version"]), ROOT, timeout=60)


def display_command_arg(arg: str) -> str:
    return display_command_arg_for_root(ROOT, arg)


def container_engine_available(runner: Runner, engine: str, record: bool) -> bool:
    resolved = find_container_engine_executable(engine)
    if not resolved:
        runner.log(f"{engine} executable was not found.")
        return False
    if record:
        if not runner.run(f"{engine} version", [resolved, "--version"], ROOT, timeout=60):
            return False
        return run_quiet_with_log(
            runner,
            f"{engine} engine status",
            [resolved, "info"],
            ROOT,
            TEST_ROOT / "build" / "backend-probes" / (engine + "_info.log"),
            timeout=120,
        )
    ok, _, detail = container_engine_probe(engine)
    if ok:
        runner.log(f"{engine} engine is reachable.")
    else:
        runner.log(f"{engine} engine is not reachable: {detail}")
    return ok


def wait_for_container_engine(runner: Runner, engine: str, timeout: int) -> bool:
    if runner.dry_run:
        runner.results.append(RunResult(f"{engine} engine ready", True, 0, "dry-run"))
        return True
    deadline = time.monotonic() + timeout
    last_detail = ""
    while time.monotonic() < deadline:
        ok, _, detail = container_engine_probe(engine, timeout=10)
        if ok:
            runner.log(f"{engine} engine is reachable.")
            runner.results.append(RunResult(f"{engine} engine ready", True, 0, "reachable"))
            return True
        last_detail = detail
        time.sleep(3.0)
    runner.log(f"{engine} engine did not become reachable: {last_detail}")
    runner.results.append(RunResult(f"{engine} engine ready", False, None, last_detail or "timed out"))
    return False


def run_prepare_capture(runner: Runner, name: str, cmd: list[str], timeout: int = 300) -> tuple[bool, str]:
    runner.log("")
    runner.log("== " + name)
    runner.log("+ " + runner.display_cmd(cmd))
    if runner.dry_run:
        runner.results.append(RunResult(name, True, 0, "dry-run"))
        return True, ""
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
        detail = f"timed out after {timeout} seconds"
        runner.log(output)
        runner.log("FAIL: " + name + f" ({detail})")
        runner.results.append(RunResult(name, False, -1, detail))
        return False, output
    except Exception as exc:
        runner.log("FAIL: " + name + f" ({exc})")
        runner.results.append(RunResult(name, False, None, str(exc)))
        return False, ""
    output = proc.stdout or ""
    if output:
        runner.log(output.rstrip())
    ok = proc.returncode == 0
    runner.log(("PASS: " if ok else "FAIL: ") + name + ("" if ok else f" (exit {proc.returncode})"))
    runner.results.append(RunResult(name, ok, proc.returncode, ""))
    return ok, output


def container_mount_arg() -> str:
    return container_mount_arg_for_root(ROOT)


def run_container_lanes(runner: Runner, engine: str) -> bool:
    engine_path = find_container_engine_executable(engine) or engine
    build_root = "/tmp/sstl-validation"
    base_cache = [
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DSSTL_ROOT=..",
    ]

    def configure_lane(build_name: str, extra_cache: list[str]) -> str:
        args = " ".join(quote_sh(arg) for arg in base_cache + extra_cache)
        return "cmake -S . -B \"$build_root/" + build_name + "\" -G Ninja " + args + "; "

    def build_lane(build_name: str) -> str:
        return "cmake --build \"$build_root/" + build_name + "\"; "

    def test_lane(build_name: str, extra: str = "") -> str:
        return "ctest --test-dir \"$build_root/" + build_name + "\" --output-on-failure" + extra + "; "

    shell = (
        "set -e; "
        "apt-get update >/dev/null; "
        "apt-get install -y --no-install-recommends cmake ninja-build g++ clang libclang-rt-dev util-linux >/dev/null; "
        "test_src=/work/testing; "
        "build_root=" + quote_sh(build_root) + "; "
        "artifacts_dir=/work/artifacts; "
        "cd \"$test_src\"; "
        "rm -rf \"$build_root\"; "
        "mkdir -p \"$build_root\"; "
        + configure_lane("host-debug", ["-DSSTL_ERROR_POLICY=SSTL_RETURN"])
        + build_lane("host-debug")
        + test_lane("host-debug")
        + test_lane("host-debug", " -R runtime_interface_comparison -V")
        + "mkdir -p \"$artifacts_dir\"; "
        + "runtime_csv=$(find \"$build_root/host-debug\" -name "
        + quote_sh(RUNTIME_CSV_NAME)
        + " -print -quit); "
        + "if [ -n \"$runtime_csv\" ]; then cp \"$runtime_csv\" \"$artifacts_dir/"
        + RUNTIME_CSV_NAME
        + "\"; fi; "
        + configure_lane("host-panic", ["-DSSTL_ERROR_POLICY=SSTL_PANIC"])
        + build_lane("host-panic")
        + test_lane("host-panic")
        + configure_lane(
            "host-ub",
            [
                "-DSSTL_ERROR_POLICY=SSTL_UB",
                "-DCMAKE_C_FLAGS=-fsanitize=undefined -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=undefined -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=undefined",
            ],
        )
        + build_lane("host-ub")
        + test_lane("host-ub")
        + configure_lane(
            "host-asan-ubsan",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=gcc",
                "-DCMAKE_CXX_COMPILER=g++",
                "-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined",
            ],
        )
        + build_lane("host-asan-ubsan")
        + test_lane("host-asan-ubsan")
        + configure_lane(
            "host-msan",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=clang",
                "-DCMAKE_CXX_COMPILER=clang++",
                "-DCMAKE_C_FLAGS=-fsanitize=memory -fPIE -fno-omit-frame-pointer -fno-optimize-sibling-calls",
                "-DCMAKE_CXX_FLAGS=-fsanitize=memory -fPIE -fno-omit-frame-pointer -fno-optimize-sibling-calls",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=memory -pie",
                "-DSSTL_DISABLE_CPP_NOALLOC_AUDIT=ON",
                "-DSSTL_ENABLE_MSAN=ON",
                "-DSSTL_TEST_RUNNER=setarch x86_64 -R",
            ],
        )
        + build_lane("host-msan")
        + test_lane("host-msan")
        + configure_lane(
            "host-tsan",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=clang",
                "-DCMAKE_CXX_COMPILER=clang++",
                "-DCMAKE_C_FLAGS=-fsanitize=thread -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=thread -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread",
                "-DSSTL_DISABLE_CPP_NOALLOC_AUDIT=ON",
                "-DSSTL_TEST_RUNNER=setarch x86_64 -R",
            ],
        )
        + build_lane("host-tsan")
        + test_lane("host-tsan")
        + configure_lane("host-freestanding-probes", ["-DSSTL_ERROR_POLICY=SSTL_RETURN", "-DSSTL_ENABLE_FREESTANDING_PROBES=ON"])
        + build_lane("host-freestanding-probes")
        + configure_lane(
            "host-libfuzzer",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=clang",
                "-DCMAKE_CXX_COMPILER=clang++",
                "-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined",
                "-DSSTL_ENABLE_LIBFUZZER=ON",
            ],
        )
        + build_lane("host-libfuzzer")
        + "python3 scripts/mutation_smoke.py --root . --sstl-root .."
    )
    return runner.run(
        f"{engine} gcc validation lanes",
        [
            engine_path,
            "run",
            "--rm",
            "--security-opt",
            "seccomp=unconfined",
            "-v",
            container_mount_arg(),
            "-w",
            "/work/testing",
            DOCKER_IMAGE,
            "bash",
            "-lc",
            shell,
        ],
        ROOT,
        timeout=60 * 60,
    )


def wsl_installable_distro(runner: Runner) -> tuple[str, str | None] | None:
    if os.name != "nt":
        runner.log("WSL backend is only meaningful on Windows.")
        return None
    wsl = find_named_executable("wsl")
    if not wsl:
        runner.log("wsl executable was not found.")
        return None
    configured = runner.wsl_distro.strip()
    candidates: list[tuple[str, str | None]] = []
    if configured:
        candidates.append((configured, configured))
    else:
        default_distro = wsl_default_distro(wsl)
        if default_distro and not wsl_distro_is_internal(default_distro):
            candidates.append((default_distro, None))
        for distro in wsl_installed_distros(wsl):
            if not wsl_distro_is_internal(distro):
                candidates.append((distro, distro))
    seen: set[str] = set()
    for label, distro in candidates:
        key = distro or "<default>"
        if key in seen:
            continue
        seen.add(key)
        try:
            proc = subprocess.run(
                wsl_command(wsl, distro, "sh", "-lc", "uname -s && command -v apt-get"),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=60,
            )
        except Exception as exc:
            runner.log("WSL distro is not installable: " + label + f" ({exc})")
            continue
        detail = " ".join(line.strip() for line in decode_wsl_output(proc.stdout).splitlines() if line.strip())
        if proc.returncode == 0:
            runner.log("Selected WSL distro for preparation: " + label + " (" + detail + ")")
            return wsl, distro
        runner.log("WSL distro is not installable: " + label + " (" + detail + ")")
    runner.log("No Ubuntu/Debian-like WSL distro with apt-get was found for preparation.")
    return None


def probe_wsl_distro(wsl: str, distro: str | None) -> tuple[bool, str]:
    probe = (
        "uname -s; "
        "command -v sh >/dev/null 2>&1 || exit 4; "
        "test -d testing && test -d include || { echo 'checkout is not visible from WSL inherited cwd'; exit 3; }; "
        "if command -v cmake >/dev/null 2>&1 && command -v ninja >/dev/null 2>&1 && "
        "command -v gcc >/dev/null 2>&1 && command -v g++ >/dev/null 2>&1 && "
        "command -v clang >/dev/null 2>&1 && command -v clang++ >/dev/null 2>&1; then "
        "  echo ready; "
        "elif command -v apt-get >/dev/null 2>&1; then "
        "  echo 'missing cmake/ninja/gcc/g++/clang/clang++; install inside WSL with: " + WSL_TEST_TOOL_INSTALL + "'; "
        "  exit 3; "
        "else "
        "  echo 'missing cmake/ninja/clang and apt-get is not available'; "
        "  exit 3; "
        "fi"
    )
    try:
        proc = subprocess.run(
            wsl_command(wsl, distro, "sh", "-lc", probe),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=60,
        )
    except Exception as exc:
        return False, str(exc)
    output = " ".join(line.strip() for line in decode_wsl_output(proc.stdout).splitlines() if line.strip())
    return proc.returncode == 0, output or f"exit {proc.returncode}"


def probe_wsl_coverage_distro(wsl: str, distro: str | None) -> tuple[bool, str]:
    probe = (
        "uname -s; "
        "command -v sh >/dev/null 2>&1 || exit 4; "
        "test -d testing && test -d include || { echo 'checkout is not visible from WSL inherited cwd'; exit 3; }; "
        "if command -v cmake >/dev/null 2>&1 && command -v ninja >/dev/null 2>&1 && "
        "command -v gcc >/dev/null 2>&1 && command -v g++ >/dev/null 2>&1 && "
        "command -v lcov >/dev/null 2>&1; then "
        "  echo ready; "
        "elif command -v apt-get >/dev/null 2>&1; then "
        "  echo 'missing cmake/ninja/gcc/g++/lcov; install inside WSL with: " + WSL_COVERAGE_TOOL_INSTALL + "'; "
        "  exit 3; "
        "else "
        "  echo 'missing cmake/ninja/gcc/g++/lcov and apt-get is not available'; "
        "  exit 3; "
        "fi"
    )
    try:
        proc = subprocess.run(
            wsl_command(wsl, distro, "sh", "-lc", probe),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=60,
        )
    except Exception as exc:
        return False, str(exc)
    output = " ".join(line.strip() for line in decode_wsl_output(proc.stdout).splitlines() if line.strip())
    return proc.returncode == 0, output or f"exit {proc.returncode}"


def select_wsl_distro(runner: Runner) -> tuple[str, str | None] | None:
    if os.name != "nt":
        runner.log("WSL backend is only meaningful on Windows.")
        return None
    wsl = find_named_executable("wsl")
    if not wsl:
        runner.log("wsl executable was not found.")
        return None

    if runner.resolved_wsl_distro is not None:
        return wsl, runner.resolved_wsl_distro or None

    configured = runner.wsl_distro.strip()
    candidates: list[tuple[str, str | None]] = []
    if configured:
        candidates.append((configured, configured))
    else:
        default_distro = wsl_default_distro(wsl)
        if default_distro and wsl_distro_is_internal(default_distro):
            runner.log("Skipping internal default WSL distro: " + default_distro)
        else:
            candidates.append(("default" if not default_distro else default_distro, None))
        for distro in wsl_installed_distros(wsl):
            if wsl_distro_is_internal(distro):
                runner.log("Skipping internal WSL distro: " + distro)
                continue
            candidates.append((distro, distro))

    seen: set[str] = set()
    for label, distro in candidates:
        key = distro or "<default>"
        if key in seen:
            continue
        seen.add(key)
        ok, detail = probe_wsl_distro(wsl, distro)
        if ok:
            runner.resolved_wsl_distro = distro or ""
            runner.log("Selected WSL distro: " + label + " (" + detail + ")")
            return wsl, distro
        runner.log("WSL distro is not usable for SSTL lanes: " + label + " (" + detail + ")")

    if configured:
        runner.log("Configured WSL distro was not usable: " + configured)
    else:
        runner.log("No usable WSL distro found. Install Ubuntu/Debian or pass --wsl-distro for a suitable distro.")
    return None


def select_wsl_coverage_distro(runner: Runner) -> tuple[str, str | None] | None:
    if os.name != "nt":
        runner.log("WSL coverage backend is only meaningful on Windows.")
        return None
    wsl = find_named_executable("wsl")
    if not wsl:
        runner.log("wsl executable was not found.")
        return None

    if runner.resolved_wsl_coverage_distro is not None:
        return wsl, runner.resolved_wsl_coverage_distro or None

    configured = runner.wsl_distro.strip()
    candidates: list[tuple[str, str | None]] = []
    if configured:
        candidates.append((configured, configured))
    else:
        default_distro = wsl_default_distro(wsl)
        if default_distro and wsl_distro_is_internal(default_distro):
            runner.log("Skipping internal default WSL distro: " + default_distro)
        else:
            candidates.append(("default" if not default_distro else default_distro, None))
        for distro in wsl_installed_distros(wsl):
            if wsl_distro_is_internal(distro):
                runner.log("Skipping internal WSL distro: " + distro)
                continue
            candidates.append((distro, distro))

    seen: set[str] = set()
    for label, distro in candidates:
        key = distro or "<default>"
        if key in seen:
            continue
        seen.add(key)
        ok, detail = probe_wsl_coverage_distro(wsl, distro)
        if ok:
            runner.resolved_wsl_coverage_distro = distro or ""
            runner.log("Selected WSL coverage distro: " + label + " (" + detail + ")")
            return wsl, distro
        runner.log("WSL distro is not usable for SSTL coverage: " + label + " (" + detail + ")")

    if configured:
        runner.log("Configured WSL distro was not usable for coverage: " + configured)
    else:
        runner.log("No coverage-capable WSL distro found. Install lcov in Ubuntu/Debian or pass --wsl-distro.")
    return None


def wsl_available(runner: Runner, record: bool = True) -> bool:
    selected = select_wsl_distro(runner)
    if not selected:
        return False
    wsl, distro = selected
    cmd = wsl_command(
        wsl,
        distro,
        "sh",
        "-lc",
        "uname -s && test -d testing && test -d include && command -v sh && command -v cmake && command -v ninja && command -v gcc && command -v g++ && command -v clang && command -v clang++",
    )
    if record:
        return runner.run("wsl shell/tooling probe", cmd, ROOT, timeout=60)
    try:
        proc = subprocess.run(
            cmd,
            cwd=str(ROOT),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=60,
        )
    except Exception as exc:
        runner.log("WSL shell/tooling probe failed during discovery: " + str(exc))
        return False
    if proc.returncode != 0:
        detail = " ".join(line.strip() for line in decode_wsl_output(proc.stdout).splitlines() if line.strip())
        runner.log("WSL shell/tooling probe failed during discovery: " + (detail or f"exit {proc.returncode}"))
        return False
    return True


def wsl_coverage_available(runner: Runner, record: bool = True) -> bool:
    selected = select_wsl_coverage_distro(runner)
    if not selected:
        return False
    wsl, distro = selected
    cmd = wsl_command(
        wsl,
        distro,
        "sh",
        "-lc",
        "uname -s && test -d testing && test -d include && command -v sh && command -v cmake && command -v ninja && command -v gcc && command -v g++ && command -v lcov",
    )
    if record:
        return runner.run("wsl coverage tooling probe", cmd, ROOT, timeout=60)
    try:
        proc = subprocess.run(
            cmd,
            cwd=str(ROOT),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=60,
        )
    except Exception as exc:
        runner.log("WSL coverage tooling probe failed during discovery: " + str(exc))
        return False
    if proc.returncode != 0:
        detail = " ".join(line.strip() for line in decode_wsl_output(proc.stdout).splitlines() if line.strip())
        runner.log("WSL coverage tooling probe failed during discovery: " + (detail or f"exit {proc.returncode}"))
        return False
    return True


def publish_wsl_text_file(runner: Runner, wsl: str, distro: str | None, source: str, target: Path, name: str) -> bool:
    runner.log("")
    runner.log("== " + name)
    cmd = wsl_command(wsl, distro, "sh", "-lc", "cat " + quote_sh(source))
    runner.log("+ " + runner.display_cmd(cmd))
    if runner.dry_run:
        runner.results.append(RunResult(name, True, 0, "dry-run"))
        return True
    try:
        proc = subprocess.run(
            cmd,
            cwd=str(ROOT),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            timeout=120,
        )
    except subprocess.TimeoutExpired:
        runner.log("FAIL: " + name + " (timed out)")
        runner.results.append(RunResult(name, False, -1, "timed out"))
        return False
    except Exception as exc:
        runner.log("FAIL: " + name + f" ({exc})")
        runner.results.append(RunResult(name, False, None, str(exc)))
        return False
    if proc.returncode != 0:
        output = (proc.stdout or "").strip()
        if output:
            runner.log(output)
        runner.log("FAIL: " + name + f" (exit {proc.returncode})")
        runner.results.append(RunResult(name, False, proc.returncode, output))
        return False
    try:
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(proc.stdout, encoding="utf-8")
    except Exception as exc:
        runner.log("FAIL: " + name + f" ({exc})")
        runner.results.append(RunResult(name, False, None, str(exc)))
        return False
    runner.log("Wrote " + display_artifact_path(target))
    runner.log("PASS: " + name)
    runner.results.append(RunResult(name, True, 0, ""))
    return True


def wsl_temp_build_root(kind: str) -> str:
    return "/tmp/sstl-" + kind


def run_wsl_lanes(runner: Runner) -> bool:
    selected = select_wsl_distro(runner)
    if not selected:
        return False
    wsl, distro = selected
    if not wsl_clean_testing_build_dirs(
        runner,
        wsl,
        distro,
        [
            "build/host-debug",
            "build/host-panic",
            "build/host-ub",
            "build/host-asan-ubsan",
            "build/host-msan",
            "build/host-tsan",
            "build/host-freestanding-probes",
            "build/host-libfuzzer",
        ],
        "wsl build directory cleanup",
    ):
        return False
    wbuild = wsl_temp_build_root("validation")
    base_cache = [quote_sh("-DCMAKE_BUILD_TYPE=Debug"), quote_sh("-DSSTL_ROOT=..")]

    def configure_lane(build_name: str, extra_cache: list[str]) -> str:
        args = " ".join(base_cache + [quote_sh(arg) for arg in extra_cache])
        return "cmake -S . -B \"$build_root/" + build_name + "\" -G Ninja " + args + "; "

    def build_lane(build_name: str) -> str:
        return "cmake --build \"$build_root/" + build_name + "\"; "

    def test_lane(build_name: str, extra: str = "") -> str:
        return "ctest --test-dir \"$build_root/" + build_name + "\" --output-on-failure" + extra + "; "

    shell = (
        "set -e; "
        "repo_root=$(pwd -P); "
        "test_src=\"$repo_root/testing\"; "
        "build_root=" + quote_sh(wbuild) + "; "
        "artifacts_dir=\"$repo_root/artifacts\"; "
        "cd \"$test_src\"; "
        "if ! command -v cmake >/dev/null 2>&1 || ! command -v ninja >/dev/null 2>&1 || "
        "! command -v gcc >/dev/null 2>&1 || ! command -v g++ >/dev/null 2>&1 || "
        "! command -v clang >/dev/null 2>&1 || ! command -v clang++ >/dev/null 2>&1; then "
        "  echo 'WSL validation tools are missing. Install inside WSL with: " + WSL_TEST_TOOL_INSTALL + "'; "
        "  exit 3; "
        "fi; "
        "rm -rf \"$build_root\"; "
        "mkdir -p \"$build_root\"; "
        + configure_lane("host-debug", ["-DSSTL_ERROR_POLICY=SSTL_RETURN"])
        + build_lane("host-debug")
        + test_lane("host-debug")
        + test_lane("host-debug", " -R runtime_interface_comparison -V")
        + "mkdir -p \"$artifacts_dir\"; "
        + "runtime_csv=$(find \"$build_root/host-debug\" -name "
        + quote_sh(RUNTIME_CSV_NAME)
        + " -print -quit); "
        + "if [ -n \"$runtime_csv\" ]; then cp \"$runtime_csv\" \"$artifacts_dir/"
        + RUNTIME_CSV_NAME
        + "\"; fi; "
        + configure_lane("host-panic", ["-DSSTL_ERROR_POLICY=SSTL_PANIC"])
        + build_lane("host-panic")
        + test_lane("host-panic")
        + configure_lane(
            "host-ub",
            [
                "-DSSTL_ERROR_POLICY=SSTL_UB",
                "-DCMAKE_C_FLAGS=-fsanitize=undefined -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=undefined -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=undefined",
            ],
        )
        + build_lane("host-ub")
        + test_lane("host-ub")
        + configure_lane(
            "host-asan-ubsan",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=gcc",
                "-DCMAKE_CXX_COMPILER=g++",
                "-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined",
            ],
        )
        + build_lane("host-asan-ubsan")
        + test_lane("host-asan-ubsan")
        + configure_lane(
            "host-msan",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=clang",
                "-DCMAKE_CXX_COMPILER=clang++",
                "-DCMAKE_C_FLAGS=-fsanitize=memory -fPIE -fno-omit-frame-pointer -fno-optimize-sibling-calls",
                "-DCMAKE_CXX_FLAGS=-fsanitize=memory -fPIE -fno-omit-frame-pointer -fno-optimize-sibling-calls",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=memory -pie",
                "-DSSTL_DISABLE_CPP_NOALLOC_AUDIT=ON",
                "-DSSTL_ENABLE_MSAN=ON",
                "-DSSTL_TEST_RUNNER=setarch x86_64 -R",
            ],
        )
        + build_lane("host-msan")
        + test_lane("host-msan")
        + configure_lane(
            "host-tsan",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=clang",
                "-DCMAKE_CXX_COMPILER=clang++",
                "-DCMAKE_C_FLAGS=-fsanitize=thread -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=thread -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=thread",
                "-DSSTL_DISABLE_CPP_NOALLOC_AUDIT=ON",
                "-DSSTL_TEST_RUNNER=setarch x86_64 -R",
            ],
        )
        + build_lane("host-tsan")
        + test_lane("host-tsan")
        + configure_lane("host-freestanding-probes", ["-DSSTL_ERROR_POLICY=SSTL_RETURN", "-DSSTL_ENABLE_FREESTANDING_PROBES=ON"])
        + build_lane("host-freestanding-probes")
        + configure_lane(
            "host-libfuzzer",
            [
                "-DSSTL_ERROR_POLICY=SSTL_RETURN",
                "-DCMAKE_C_COMPILER=clang",
                "-DCMAKE_CXX_COMPILER=clang++",
                "-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined",
                "-DSSTL_ENABLE_LIBFUZZER=ON",
            ],
        )
        + build_lane("host-libfuzzer")
        + "python3 scripts/mutation_smoke.py --root . --sstl-root .. --output \"$build_root/mutation_summary.yaml\""
    )
    ok = runner.run("wsl gcc validation lanes", wsl_command(wsl, distro, "sh", "-lc", shell), ROOT, timeout=60 * 60)
    if ok:
        ok = publish_wsl_text_file(
            runner,
            wsl,
            distro,
            wbuild + "/mutation_summary.yaml",
            TEST_ROOT / "manifests" / "mutation_summary.yaml",
            "wsl mutation summary publish",
        )
    return ok


def wsl_clean_testing_build_dirs(
    runner: Runner,
    wsl: str,
    distro: str | None,
    dirs: list[str],
    name: str,
) -> bool:
    quoted_dirs = " ".join(quote_sh(path) for path in dirs)
    shell = (
        "set -e; "
        "test -d testing || { echo 'checkout is not visible from WSL inherited cwd'; exit 3; }; "
        "cd testing; "
        "set -- " + quoted_dirs + "; "
        "for d in \"$@\"; do "
        "  case \"$d\" in "
        "    build/*) if [ -e \"$d\" ]; then rm -rf -- \"$d\"; fi ;; "
        "    *) printf '%s\\n' \"refusing unsafe cleanup path: [$d]\"; exit 2 ;; "
        "  esac; "
        "done"
    )
    return runner.run(name, wsl_command(wsl, distro, "-u", "root", "sh", "-lc", shell), ROOT, timeout=10 * 60)


def quote_sh(value: str) -> str:
    return "'" + value.replace("'", "'\"'\"'") + "'"


def coverage_info_path() -> Path:
    return TEST_ROOT / "build" / "coverage" / COVERAGE_INFO_NAME


def coverage_raw_info_path() -> Path:
    return TEST_ROOT / "build" / "coverage" / "sstl_coverage.info"


def coverage_lcov_capture_log_path() -> Path:
    return TEST_ROOT / "build" / "coverage" / COVERAGE_LCOV_CAPTURE_LOG_NAME


def coverage_lcov_filter_log_path() -> Path:
    return TEST_ROOT / "build" / "coverage" / COVERAGE_LCOV_FILTER_LOG_NAME


def lcov_shell_runner() -> str:
    return (
        "lcov_filter_args=''; "
        "if lcov --help 2>&1 | grep -q -- '--filter'; then "
        "lcov_filter_args='--filter branch,exception,orphan'; "
        "fi; "
        "run_lcov_quiet() { "
        "log=\"$1\"; shift; "
        "if ! \"$@\" >\"$log\" 2>&1; then "
        "echo \"LCOV command failed; diagnostics follow from $log\"; "
        "cat \"$log\"; "
        "exit 1; "
        "fi; "
        "}; "
    )


def display_coverage_path(path: str) -> str:
    normalized = path.replace("\\", "/")
    if normalized.startswith("/work/"):
        return normalized[len("/work/") :]
    if ROOT.drive:
        root_rest = str(ROOT.resolve())[len(ROOT.drive) :].replace("\\", "/").lstrip("/")
        wsl_root = "/mnt/" + ROOT.drive.rstrip(":").lower() + "/" + root_rest
        if normalized.lower() == wsl_root.lower():
            return "."
        if normalized.lower().startswith(wsl_root.lower() + "/"):
            return normalized[len(wsl_root) + 1 :]
    try:
        p = Path(path)
        if p.is_absolute():
            return str(p.relative_to(ROOT)).replace("\\", "/")
    except Exception:
        pass
    return normalized


def display_artifact_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT)).replace("\\", "/")
    except Exception:
        normalized = str(path).replace("\\", "/")
        if normalized.startswith("/work/"):
            return normalized[len("/work/") :]
        return normalized


def publish_to_artifacts(path: Path, name: str | None = None) -> Path:
    ARTIFACT_ROOT.mkdir(parents=True, exist_ok=True)
    target = ARTIFACT_ROOT / (name or path.name)
    try:
        if path.resolve() != target.resolve():
            shutil.copy2(path, target)
    except Exception:
        if path != target:
            shutil.copy2(path, target)
    return target


def publish_newest_runtime_csv(runner: Runner | None = None) -> Path | None:
    source = newest_runtime_csv_artifact(include_artifacts=False)
    if not source:
        return None
    try:
        target = publish_to_artifacts(source, RUNTIME_CSV_NAME)
    except Exception as exc:
        if runner:
            runner.log(f"Could not copy runtime CSV into artifacts: {exc}")
        return source
    if runner:
        runner.log(f"Runtime CSV artifact: {display_artifact_path(target)}")
    return target


def find_coverage_artifacts() -> list[Path]:
    artifacts: list[Path] = []
    for root in (ARTIFACT_ROOT, TEST_ROOT):
        if not root.exists():
            continue
        try:
            for path in root.rglob(COVERAGE_INFO_NAME):
                if path.is_file() and path not in artifacts:
                    artifacts.append(path)
        except Exception:
            continue
    return sorted(artifacts, key=lambda p: p.stat().st_mtime if p.exists() else 0, reverse=True)


def newest_coverage_artifact() -> Path | None:
    artifacts = find_coverage_artifacts()
    return artifacts[0] if artifacts else None


def parse_lcov_info(path: Path) -> CoverageReport:
    by_path: dict[str, CoverageFile] = {}
    current: CoverageFile | None = None

    def flush() -> None:
        nonlocal current
        if not current:
            return
        key = current.path
        existing = by_path.get(key)
        if existing:
            existing.lines_found += current.lines_found
            existing.lines_hit += current.lines_hit
            existing.branches_found += current.branches_found
            existing.branches_hit += current.branches_hit
        else:
            by_path[key] = current
        current = None

    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if raw.startswith("SF:"):
            flush()
            display_path = display_coverage_path(raw.split(":", 1)[1])
            current = CoverageFile(display_path) if display_path.startswith(COVERAGE_SOURCE_PREFIXES) else None
        elif current and raw.startswith("LF:"):
            current.lines_found += int(raw.split(":", 1)[1])
        elif current and raw.startswith("LH:"):
            current.lines_hit += int(raw.split(":", 1)[1])
        elif current and raw.startswith("BRF:"):
            current.branches_found += int(raw.split(":", 1)[1])
        elif current and raw.startswith("BRH:"):
            current.branches_hit += int(raw.split(":", 1)[1])
        elif raw == "end_of_record":
            flush()
    flush()
    files = sorted(by_path.values(), key=lambda item: item.path)
    return CoverageReport(path, files)


def pct_text(value: float) -> str:
    return f"{value * 100.0:6.2f}%"


def coverage_report_lines(report: CoverageReport, color: bool = False) -> list[str]:
    overall_status = color_status(report.status, color)
    line_pct = color_pct(report.line_pct, COVERAGE_LINE_MIN, color)
    branch_pct = color_pct(report.branch_pct, COVERAGE_BRANCH_MIN, color)
    lines = [
        "",
        color_text("== Coverage report", "bold", color),
        f"Source: {display_artifact_path(report.source)}",
        f"Branch policy: {COVERAGE_BRANCH_POLICY}",
        (
            "Overall: "
            f"line {line_pct} ({report.lines_hit}/{report.lines_found}), "
            f"branch {branch_pct} ({report.branches_hit}/{report.branches_found}), "
            f"status {overall_status}"
        ),
        "",
        f"{'status':<6} {'line':>8} {'line hit/found':>16} {'branch':>8} {'branch hit/found':>18} file",
    ]
    for item in report.files:
        item_status = color_text(f"{item.status:<6}", "green" if item.status == "pass" else "red", color)
        item_line_pct = color_text(f"{pct_text(item.line_pct):>8}", "green" if item.line_pct >= COVERAGE_LINE_MIN else "red", color)
        item_branch_pct = (
            color_text(f"{pct_text(item.branch_pct):>8}", "green" if item.branch_pct >= COVERAGE_BRANCH_MIN else "red", color)
            if item.branches_found
            else color_text("   n/a", "dim", color)
        )
        lines.append(
            f"{item_status} "
            f"{item_line_pct} "
            f"{item.lines_hit:>7}/{item.lines_found:<8} "
            f"{item_branch_pct} "
            f"{item.branches_hit:>8}/{item.branches_found:<9} "
            f"{item.path}"
        )
    return lines


def write_coverage_summary_manifest(report: CoverageReport) -> None:
    out = TEST_ROOT / "manifests" / "coverage_summary.yaml"
    out.parent.mkdir(parents=True, exist_ok=True)
    capture_log = report.source.parent / COVERAGE_LCOV_CAPTURE_LOG_NAME
    filter_log = report.source.parent / COVERAGE_LCOV_FILTER_LOG_NAME
    rows = [
        f"generated_at_utc: {datetime.now(timezone.utc).isoformat()}",
        f"source: {display_artifact_path(report.source)}",
        f"branch_policy: {COVERAGE_BRANCH_POLICY}",
        "lcov_diagnostics:",
        f"  capture_log: {display_artifact_path(capture_log) if capture_log.exists() else ''}",
        f"  filter_log: {display_artifact_path(filter_log) if filter_log.exists() else ''}",
        f"line_coverage: {report.line_pct:.6f}",
        f"branch_coverage: {report.branch_pct:.6f}",
        f"line_min: {COVERAGE_LINE_MIN:.6f}",
        f"branch_min: {COVERAGE_BRANCH_MIN:.6f}",
        f"status: {report.status}",
        "files:",
    ]
    for item in report.files:
        rows.extend([
            f"  - path: {item.path}",
            f"    line_coverage: {item.line_pct:.6f}",
            f"    branch_coverage: {item.branch_pct:.6f}",
            f"    lines_hit: {item.lines_hit}",
            f"    lines_found: {item.lines_found}",
            f"    branches_hit: {item.branches_hit}",
            f"    branches_found: {item.branches_found}",
            f"    status: {item.status}",
        ])
    out.write_text("\n".join(rows) + "\n", encoding="utf-8")


def log_coverage_report(runner: Runner, path: Path | None = None, enforce_thresholds: bool = True, note: str = "") -> bool:
    artifact = path or newest_coverage_artifact()
    if not artifact:
        runner.log("")
        runner.log("== Coverage report")
        runner.log("No coverage artifact was found. Run with --coverage-only or use the GUI Coverage row.")
        runner.results.append(RunResult("coverage report", False, None, "no coverage artifact found"))
        return False
    report = parse_lcov_info(artifact)
    try:
        for log_name in (COVERAGE_LCOV_CAPTURE_LOG_NAME, COVERAGE_LCOV_FILTER_LOG_NAME):
            log_path = artifact.parent / log_name
            if log_path.exists():
                publish_to_artifacts(log_path, log_name)
        published = publish_to_artifacts(artifact, COVERAGE_INFO_NAME)
        report = parse_lcov_info(published)
    except Exception as exc:
        runner.log(f"Could not copy coverage artifact into artifacts: {exc}")
    write_coverage_summary_manifest(report)
    for line in coverage_report_lines(report, runner.color_enabled):
        runner.log(line)
    if note:
        runner.log("Note: " + note)
    if enforce_thresholds:
        detail = "thresholds passed" if report.status == "pass" else "thresholds failed"
        runner.results.append(RunResult("coverage report", report.status == "pass", 0 if report.status == "pass" else 1, detail))
    else:
        detail = "thresholds reported only" if report.status != "pass" else "thresholds passed"
        runner.results.append(RunResult("coverage report", True, 0, detail))
    return True


def publish_container_file(runner: Runner, engine_path: str, container_name: str, source: str, target: Path, name: str) -> bool:
    if runner.dry_run:
        return runner.run(
            name,
            [engine_path, "cp", container_name + ":" + source, str(target)],
            ROOT,
            timeout=120,
        )
    try:
        target.parent.mkdir(parents=True, exist_ok=True)
        temp_dir = Path(tempfile.mkdtemp(prefix="sstl-container-copy-"))
    except Exception as exc:
        runner.log("FAIL: " + name + f" ({exc})")
        runner.results.append(RunResult(name, False, None, str(exc)))
        return False
    temp_target = temp_dir / target.name
    ok = runner.run(
        name,
        [engine_path, "cp", container_name + ":" + source, str(temp_target)],
        ROOT,
        timeout=120,
    )
    if ok:
        try:
            target.write_bytes(temp_target.read_bytes())
            runner.log("Wrote " + display_artifact_path(target))
        except Exception as exc:
            runner.log("FAIL: " + name + " publish write (" + str(exc) + ")")
            runner.results.append(RunResult(name + " publish write", False, None, str(exc)))
            ok = False
    try:
        shutil.rmtree(temp_dir, ignore_errors=True)
    except Exception:
        pass
    return ok


def run_container_coverage(runner: Runner, engine: str) -> bool:
    engine_path = find_container_engine_executable(engine) or engine
    build_dir = "/tmp/sstl-coverage"
    container_name = "sstl-coverage-" + engine + "-" + str(os.getpid()) + "-" + str(int(time.time()))
    shell = (
        "set -e; "
        "apt-get update >/dev/null; "
        "apt-get install -y --no-install-recommends cmake ninja-build g++ lcov >/dev/null; "
        "build_dir=" + quote_sh(build_dir) + "; "
        "cd /work; "
        "rm -rf \"$build_dir\"; "
        "mkdir -p \"$build_dir\"; "
        "cmake -S testing -B \"$build_dir\" -G Ninja -DSSTL_ROOT=. -DSSTL_ENABLE_COVERAGE=ON; "
        "cmake --build \"$build_dir\"; "
        "ctest --test-dir \"$build_dir\" --output-on-failure; "
        + lcov_shell_runner()
        + "run_lcov_quiet \"$build_dir/"
        + COVERAGE_LCOV_CAPTURE_LOG_NAME
        + "\" lcov --capture --directory \"$build_dir\" --output-file \"$build_dir/sstl_coverage.info\" "
        + LCOV_BRANCH_RC_SHELL
        + "; "
        "run_lcov_quiet \"$build_dir/"
        + COVERAGE_LCOV_FILTER_LOG_NAME
        + "\" lcov --remove \"$build_dir/sstl_coverage.info\" '/usr/*' '*/testing/*' "
        "${lcov_filter_args} "
        "--output-file \"$build_dir/sstl_coverage.filtered.info\" "
        + LCOV_BRANCH_RC_SHELL
    )
    ok = runner.run(
        f"{engine} coverage lane",
        [
            engine_path,
            "run",
            "--name",
            container_name,
            "-v",
            container_mount_arg(),
            "-w",
            "/work",
            DOCKER_IMAGE,
            "bash",
            "-lc",
            shell,
        ],
        ROOT,
        timeout=60 * 60,
    )
    if ok:
        ok = publish_container_file(runner, engine_path, container_name, build_dir + "/sstl_coverage.info", coverage_raw_info_path(), f"{engine} publish raw coverage info")
        ok = publish_container_file(runner, engine_path, container_name, build_dir + "/sstl_coverage.filtered.info", coverage_info_path(), f"{engine} publish filtered coverage info") and ok
        ok = publish_container_file(
            runner,
            engine_path,
            container_name,
            build_dir + "/" + COVERAGE_LCOV_CAPTURE_LOG_NAME,
            coverage_lcov_capture_log_path(),
            f"{engine} publish coverage capture log",
        ) and ok
        ok = publish_container_file(
            runner,
            engine_path,
            container_name,
            build_dir + "/" + COVERAGE_LCOV_FILTER_LOG_NAME,
            coverage_lcov_filter_log_path(),
            f"{engine} publish coverage filter log",
        ) and ok
        if ok:
            if runner.dry_run:
                log_coverage_report(
                    runner,
                    enforce_thresholds=False,
                    note="Dry-run used the newest existing coverage artifact and did not gate thresholds.",
                )
            else:
                log_coverage_report(runner, coverage_info_path())
    remove_ok = runner.run(f"{engine} remove coverage container", [engine_path, "rm", "-f", container_name], ROOT, timeout=120)
    return ok and remove_ok


def run_wsl_coverage(runner: Runner) -> bool:
    selected = select_wsl_coverage_distro(runner)
    if not selected:
        return False
    wsl, distro = selected
    if not wsl_clean_testing_build_dirs(runner, wsl, distro, ["build/coverage"], "wsl coverage build directory cleanup"):
        return False
    wbuild = wsl_temp_build_root("coverage")
    shell = (
        "set -e; "
        "sstl_root=$(pwd -P); "
        "build_dir=" + quote_sh(wbuild) + "; "
        "cd \"$sstl_root\"; "
        "if ! command -v cmake >/dev/null 2>&1 || ! command -v ninja >/dev/null 2>&1 || "
        "! command -v gcc >/dev/null 2>&1 || ! command -v g++ >/dev/null 2>&1 || "
        "! command -v lcov >/dev/null 2>&1; then "
        "  echo 'WSL coverage tools are missing. Install inside WSL with: " + WSL_COVERAGE_TOOL_INSTALL + "'; "
        "  exit 3; "
        "fi; "
        "rm -rf \"$build_dir\"; "
        "mkdir -p \"$build_dir\"; "
        "cmake -S testing -B \"$build_dir\" -G Ninja -DSSTL_ROOT=. -DSSTL_ENABLE_COVERAGE=ON; "
        "cmake --build \"$build_dir\"; "
        "ctest --test-dir \"$build_dir\" --output-on-failure; "
        + lcov_shell_runner()
        + "run_lcov_quiet \"$build_dir/"
        + COVERAGE_LCOV_CAPTURE_LOG_NAME
        + "\" lcov --capture --directory \"$build_dir\" --output-file \"$build_dir/sstl_coverage.info\" "
        + LCOV_BRANCH_RC_SHELL
        + "; "
        "run_lcov_quiet \"$build_dir/"
        + COVERAGE_LCOV_FILTER_LOG_NAME
        + "\" lcov --remove \"$build_dir/sstl_coverage.info\" '/usr/*' '*/testing/*' "
        "${lcov_filter_args} "
        "--output-file \"$build_dir/sstl_coverage.filtered.info\" "
        + LCOV_BRANCH_RC_SHELL
    )
    ok = runner.run("wsl coverage lane", wsl_command(wsl, distro, "sh", "-lc", shell), ROOT, timeout=60 * 60)
    if ok:
        ok = publish_wsl_text_file(runner, wsl, distro, wbuild + "/sstl_coverage.info", coverage_raw_info_path(), "wsl publish raw coverage info")
        ok = publish_wsl_text_file(runner, wsl, distro, wbuild + "/sstl_coverage.filtered.info", coverage_info_path(), "wsl publish filtered coverage info") and ok
        ok = publish_wsl_text_file(
            runner,
            wsl,
            distro,
            wbuild + "/" + COVERAGE_LCOV_CAPTURE_LOG_NAME,
            coverage_lcov_capture_log_path(),
            "wsl publish coverage capture log",
        ) and ok
        ok = publish_wsl_text_file(
            runner,
            wsl,
            distro,
            wbuild + "/" + COVERAGE_LCOV_FILTER_LOG_NAME,
            coverage_lcov_filter_log_path(),
            "wsl publish coverage filter log",
        ) and ok
        if ok:
            if runner.dry_run:
                log_coverage_report(
                    runner,
                    enforce_thresholds=False,
                    note="WSL coverage uses the host distro compiler and reports thresholds without gating the command result.",
                )
            else:
                log_coverage_report(
                    runner,
                    coverage_info_path(),
                    enforce_thresholds=False,
                    note="WSL coverage uses the host distro compiler and reports thresholds without gating the command result.",
                )
    return ok


def run_local_coverage(runner: Runner) -> bool:
    cmake = find_cmake()
    ctest = find_ctest(cmake)
    lcov = find_named_executable("lcov")
    if not cmake or not ctest or not lcov:
        runner.log("Local coverage requires CMake, CTest, and lcov. Showing any existing coverage artifact instead.")
        return log_coverage_report(runner)
    build_dir = Path("testing") / "build" / "coverage"
    ok = runner.run(
        "local coverage configure",
        [cmake, "-S", "testing", "-B", str(build_dir), "-DSSTL_ROOT=.", "-DSSTL_ENABLE_COVERAGE=ON"],
        ROOT,
    )
    if not ok:
        return False
    ok = runner.run("local coverage build", [cmake, "--build", str(build_dir), "--config", "Debug"], ROOT)
    if not ok:
        return False
    ok = runner.run("local coverage tests", [ctest, "--test-dir", str(build_dir), "-C", "Debug", "--output-on-failure"], ROOT)
    if not ok:
        return False
    raw = build_dir / "sstl_coverage.info"
    filtered = build_dir / COVERAGE_INFO_NAME
    ok = run_quiet_with_log(
        runner,
        "local coverage capture",
        [lcov, "--capture", "--directory", str(build_dir), "--output-file", str(raw), *LCOV_BRANCH_RC_ARGS],
        ROOT,
        build_dir / COVERAGE_LCOV_CAPTURE_LOG_NAME,
    )
    if not ok:
        return False
    ok = run_quiet_with_log(
        runner,
        "local coverage filter",
        [lcov, "--remove", str(raw), "/usr/*", "*/testing/*", "--output-file", str(filtered), *LCOV_BRANCH_RC_ARGS],
        ROOT,
        build_dir / COVERAGE_LCOV_FILTER_LOG_NAME,
    )
    if ok:
        log_coverage_report(runner, filtered)
    return ok


def run_local_lane(runner: Runner, cmake: str, ctest: str, build_name: str, policy: str) -> bool:
    build_dir = Path("testing") / "build" / build_name
    ok = runner.run(
        f"local configure {policy}",
        [
            cmake,
            "-S",
            "testing",
            "-B",
            str(build_dir),
            "-DSSTL_ROOT=.",
            "-DSSTL_ERROR_POLICY=" + policy,
        ],
        ROOT,
    )
    if not ok:
        return False
    ok = runner.run(f"local build {policy}", [cmake, "--build", str(build_dir), "--config", "Debug"], ROOT)
    if not ok:
        return False
    ok = runner.run(f"local ctest {policy}", [ctest, "--test-dir", str(build_dir), "-C", "Debug", "--output-on-failure"], ROOT)
    if ok and policy == "SSTL_RETURN":
        ok = runner.run(
            "local runtime comparison report",
            [ctest, "--test-dir", str(build_dir), "-C", "Debug", "-R", "runtime_interface_comparison", "-V"],
            ROOT,
        )
    return ok


def run_local_lanes(runner: Runner) -> bool:
    cmake = find_cmake()
    ctest = find_ctest(cmake)
    if not cmake or not ctest:
        runner.log("CMake/CTest were not found. Cannot compile and run local C/C++ tests.")
        return False
    ok = True
    for build_name, policy in [
        ("host-local-return", "SSTL_RETURN"),
        ("host-local-panic", "SSTL_PANIC"),
        ("host-local-ub", "SSTL_UB"),
    ]:
        ok = run_local_lane(runner, cmake, ctest, build_name, policy) and ok
    return ok


def local_available(runner: Runner) -> bool:
    cmake = find_cmake()
    ctest = find_ctest(cmake)
    if not cmake or not ctest:
        runner.log("Local CMake/CTest toolchain was not found.")
        return False
    runner.log(f"Local CMake found: {cmake}")
    runner.log(f"Local CTest found: {ctest}")
    if runner.dry_run:
        runner.log("Local C/C++ sanity probe skipped in dry-run mode.")
        return True
    ok, detail = local_sanity_probe(cmake, ctest, coverage=False)
    if ok:
        runner.log("Local C/C++ sanity probe passed: " + detail)
        return True
    runner.log("Local C/C++ sanity probe failed: " + detail)
    return False


def local_coverage_available(runner: Runner) -> bool:
    cmake = find_cmake()
    ctest = find_ctest(cmake)
    lcov = find_named_executable("lcov")
    missing: list[str] = []
    if not cmake:
        missing.append("CMake")
    if not ctest:
        missing.append("CTest")
    if not lcov:
        missing.append("lcov")
    if missing:
        runner.log("Local coverage toolchain was not found: missing " + ", ".join(missing) + ".")
        return False
    runner.log(f"Local CMake found: {cmake}")
    runner.log(f"Local CTest found: {ctest}")
    runner.log(f"Local lcov found: {lcov}")
    if runner.dry_run:
        runner.log("Local coverage sanity probe skipped in dry-run mode.")
        return True
    ok, detail = local_sanity_probe(cmake, ctest, coverage=True)
    if ok:
        runner.log("Local coverage sanity probe passed: " + detail)
        return True
    runner.log("Local coverage sanity probe failed: " + detail)
    return False


def discover_backends(runner: Runner) -> list[str]:
    candidates: list[str] = []
    if container_engine_available(runner, "docker", record=False):
        candidates.append("docker")
    if container_engine_available(runner, "podman", record=False):
        candidates.append("podman")
    if wsl_available(runner, record=False):
        candidates.append("wsl")
    if local_available(runner):
        candidates.append("local")
    return candidates


def discover_coverage_backends(runner: Runner) -> list[str]:
    candidates: list[str] = []
    if container_engine_available(runner, "docker", record=False):
        candidates.append("docker")
    if container_engine_available(runner, "podman", record=False):
        candidates.append("podman")
    if wsl_coverage_available(runner, record=False):
        candidates.append("wsl")
    if local_coverage_available(runner):
        candidates.append("local")
    return candidates


def install_options() -> list[InstallOption]:
    if os.name == "nt":
        opts: list[InstallOption] = []
        winget = find_named_executable("winget")
        choco = find_named_executable("choco")
        if winget:
            opts.append(InstallOption(
                "Docker Desktop via winget",
                "docker",
                [[winget, "install", "--id", "Docker.DockerDesktop", "-e"]],
                [
                    "May require administrator approval, sign-in, or a reboot.",
                    "After installation, start Docker Desktop once before rerunning tests.",
                ],
            ))
            opts.append(InstallOption(
                "Podman Desktop via winget",
                "podman",
                [[winget, "install", "--id", "RedHat.Podman-Desktop", "-e"]],
                [
                    "Usually easier than manual Podman setup, but it may still require interactive prompts.",
                    "After installation, start Podman Desktop once or run podman machine init && podman machine start before rerunning tests.",
                ],
            ))
            opts.append(InstallOption(
                "Local CMake/Ninja/LLVM toolchain via winget",
                "local",
                [
                    [winget, "install", "--id", "Kitware.CMake", "-e"],
                    [winget, "install", "--id", "Ninja-build.Ninja", "-e"],
                    [winget, "install", "--id", "LLVM.LLVM", "-e"],
                ],
                [
                    "This avoids relying on a specific Visual Studio version.",
                    "If your project requires MSVC specifically, install any current Visual Studio Build Tools with C++ and CMake components.",
                ],
            ))
        if choco:
            opts.append(InstallOption(
                "Local CMake/Ninja/LLVM toolchain via Chocolatey",
                "local",
                [[choco, "install", "-y", "cmake", "ninja", "llvm"]],
                [
                    "Requires an elevated shell unless Chocolatey is configured otherwise.",
                    "If your project requires MSVC specifically, install any current Visual Studio Build Tools with C++ and CMake components.",
                ],
            ))
        opts.append(InstallOption(
            "WSL2 Ubuntu",
            "wsl",
            [[find_named_executable("wsl") or "wsl", "--install", "-d", "Ubuntu"]],
            [
                "May require administrator approval and a reboot.",
                    "After first Ubuntu launch, install validation and coverage tools inside WSL: " + WSL_TEST_TOOL_INSTALL,
            ],
        ))
        return opts
    if sys.platform == "darwin":
        opts = []
        brew = find_named_executable("brew")
        if brew:
            opts.append(InstallOption(
                "Local toolchain via Homebrew",
                "local",
                [[brew, "install", "cmake", "ninja", "gcc"]],
                ["Fastest local path on macOS when Homebrew is available."],
            ))
            opts.append(InstallOption(
                "Podman via Homebrew",
                "podman",
                [[brew, "install", "podman"]],
                ["Run podman machine init/start after install if prompted."],
            ))
        return opts + [
            InstallOption(
                "Docker Desktop manual install",
                "docker",
                [],
                ["Download and install Docker Desktop from https://www.docker.com/products/docker-desktop/."],
            )
        ]
    opts = []
    apt_get = find_named_executable("apt-get")
    dnf = find_named_executable("dnf")
    pacman = find_named_executable("pacman")
    sudo = find_named_executable("sudo") or "sudo"
    if apt_get:
        opts.append(InstallOption(
            "Local toolchain via apt",
            "local",
            [[sudo, apt_get, "update"], [sudo, apt_get, "install", "-y", "cmake", "ninja-build", "gcc", "g++", "python3"]],
            ["Best first choice on Debian/Ubuntu when sudo is configured."],
        ))
        opts.append(InstallOption(
            "Podman via apt",
            "podman",
            [[sudo, apt_get, "update"], [sudo, apt_get, "install", "-y", "podman"]],
            ["Rootless containers may need distro-specific setup."],
        ))
    if dnf:
        opts.append(InstallOption(
            "Local toolchain via dnf",
            "local",
            [[sudo, dnf, "install", "-y", "cmake", "ninja-build", "gcc", "gcc-c++", "python3"]],
            ["Best first choice on Fedora/RHEL-like systems when sudo is configured."],
        ))
        opts.append(InstallOption(
            "Podman via dnf",
            "podman",
            [[sudo, dnf, "install", "-y", "podman"]],
            ["Podman is commonly well supported on Fedora/RHEL-like systems."],
        ))
    if pacman:
        opts.append(InstallOption(
            "Local toolchain via pacman",
            "local",
            [[sudo, pacman, "-S", "--needed", "cmake", "ninja", "gcc", "python"]],
            ["Best first choice on Arch-like systems when sudo is configured."],
        ))
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
                    "After that, rerun tests or verify with: podman info",
                    "Diagnostic: " + detail,
                ]
            return [
                "Docker is installed, but the Docker engine is not reachable.",
                "Start Docker Desktop, wait until it reports the engine is running, then rerun tests.",
                "You can verify with: docker info",
                "Diagnostic: " + detail,
            ]

    if backend == "wsl":
        wsl = find_named_executable("wsl")
        if wsl and wsl_installed_distros(wsl):
            return [
                "WSL is installed, but no non-internal distro with required SSTL tools was found.",
                "Install tools inside your Ubuntu/Debian WSL distro, then rerun tests:",
                "  " + WSL_TEST_TOOL_INSTALL,
                "Verify inside WSL with:",
                "  command -v cmake && command -v ninja && command -v gcc && command -v g++ && command -v clang && command -v clang++ && command -v lcov",
            ]

    opts = install_options()
    if backend != "auto":
        opts = [opt for opt in opts if opt.backend == backend]
    if not opts:
        if backend == "auto":
            return [
                "No automatic installer was detected.",
                "Install one of: Docker/Podman, WSL2 on Windows, or a local CMake + C/C++ compiler + Ninja toolchain.",
            ]
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
        ok = runner.run("install: " + option.name, cmd, ROOT, timeout=60 * 60) and ok
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
    if assume_yes or runner.dry_run:
        return True
    if ask_yes_no(message, default=False):
        return True
    runner.log("Preparation declined: " + message)
    return False


def maybe_install_for_prepare(runner: Runner, backend: str, assume_yes_install: bool) -> bool:
    before = len(runner.results)
    ok = maybe_install_missing(runner, backend, assume_yes_install)
    if not ok and len(runner.results) == before:
        runner.results.append(RunResult("prepare " + backend, False, None, "not installed"))
    return ok


def prepare_docker_backend(runner: Runner, assume_yes_prepare: bool, install_missing: bool, assume_yes_install: bool) -> bool:
    ok, _, detail = container_engine_probe("docker", timeout=10)
    if ok:
        runner.log("Docker engine is already reachable.")
        runner.results.append(RunResult("prepare docker", True, 0, "already reachable"))
        return True
    docker = find_container_engine_executable("docker")
    if not docker:
        runner.log("Docker executable was not found.")
        if install_missing:
            return maybe_install_for_prepare(runner, "docker", assume_yes_install)
        runner.results.append(RunResult("prepare docker", False, None, "executable not found"))
        return False
    desktop = find_docker_desktop_executable()
    if not desktop:
        runner.log("Docker is installed, but Docker Desktop could not be found. Start the Docker daemon/Desktop manually.")
        runner.results.append(RunResult("prepare docker", False, None, detail))
        return False
    if not should_prepare(runner, "Start Docker Desktop and wait for the engine?", assume_yes_prepare):
        runner.results.append(RunResult("prepare docker", False, None, "declined"))
        return False
    runner.log("")
    runner.log("== start Docker Desktop")
    runner.log("+ " + desktop)
    if not runner.dry_run:
        try:
            subprocess.Popen([desktop], cwd=str(ROOT), close_fds=True)
        except Exception as exc:
            runner.log("FAIL: start Docker Desktop (" + str(exc) + ")")
            runner.results.append(RunResult("start Docker Desktop", False, None, str(exc)))
            return False
    runner.results.append(RunResult("start Docker Desktop", True, 0, "started" if not runner.dry_run else "dry-run"))
    return wait_for_container_engine(runner, "docker", timeout=180)


def prepare_podman_backend(runner: Runner, assume_yes_prepare: bool, install_missing: bool, assume_yes_install: bool) -> bool:
    ok, _, detail = container_engine_probe("podman", timeout=10)
    if ok:
        runner.log("Podman engine is already reachable.")
        runner.results.append(RunResult("prepare podman", True, 0, "already reachable"))
        return True
    podman = find_container_engine_executable("podman")
    if not podman:
        runner.log("Podman executable was not found.")
        if install_missing:
            return maybe_install_for_prepare(runner, "podman", assume_yes_install)
        runner.results.append(RunResult("prepare podman", False, None, "executable not found"))
        return False
    if not should_prepare(runner, "Initialize/start a Podman machine?", assume_yes_prepare):
        runner.results.append(RunResult("prepare podman", False, None, "declined"))
        return False
    list_ok, list_output = run_prepare_capture(runner, "podman machine list", [podman, "machine", "list"], timeout=120)
    if not list_ok:
        return False
    if not podman_machine_exists(list_output):
        if not run_prepare_capture(runner, "podman machine init", [podman, "machine", "init"], timeout=20 * 60)[0]:
            return False
    if not run_prepare_capture(runner, "podman machine start", [podman, "machine", "start"], timeout=20 * 60)[0]:
        return False
    return wait_for_container_engine(runner, "podman", timeout=180)


def prepare_wsl_backend(runner: Runner, assume_yes_prepare: bool, install_missing: bool, assume_yes_install: bool) -> bool:
    if runner.dry_run:
        wsl = find_named_executable("wsl")
        if not wsl:
            runner.log("wsl executable was not found.")
            runner.results.append(RunResult("prepare wsl", False, None, "wsl executable not found"))
            return False
        distro = runner.wsl_distro.strip() or None
        if not distro:
            for candidate in wsl_installed_distros(wsl):
                if not wsl_distro_is_internal(candidate):
                    distro = candidate
                    break
        return runner.run(
            "wsl install validation and coverage tools",
            wsl_command(wsl, distro, "-u", "root", "sh", "-lc", WSL_TEST_TOOL_INSTALL_ROOT),
            ROOT,
            timeout=60 * 60,
        )
    validation_selected = select_wsl_distro(runner)
    coverage_selected = select_wsl_coverage_distro(runner)
    if validation_selected and coverage_selected:
        runner.results.append(RunResult("prepare wsl", True, 0, "already ready"))
        return True
    installable = wsl_installable_distro(runner)
    if not installable:
        if install_missing:
            return maybe_install_for_prepare(runner, "wsl", assume_yes_install)
        runner.results.append(RunResult("prepare wsl", False, None, "no installable WSL distro"))
        return False
    if not should_prepare(runner, "Install cmake, ninja, gcc/g++, clang/clang++, lcov, and support tools inside WSL as root?", assume_yes_prepare):
        runner.results.append(RunResult("prepare wsl", False, None, "declined"))
        return False
    wsl, distro = installable
    ok = runner.run(
        "wsl install validation and coverage tools",
        wsl_command(wsl, distro, "-u", "root", "sh", "-lc", WSL_TEST_TOOL_INSTALL_ROOT),
        ROOT,
        timeout=60 * 60,
    )
    if not ok:
        return False
    runner.resolved_wsl_distro = None
    runner.resolved_wsl_coverage_distro = None
    return wsl_available(runner) and wsl_coverage_available(runner)


def prepare_local_backend(runner: Runner, assume_yes_prepare: bool, install_missing: bool, assume_yes_install: bool) -> bool:
    del assume_yes_prepare
    if local_available(runner):
        runner.results.append(RunResult("prepare local", True, 0, "already ready"))
        return True
    if install_missing:
        return maybe_install_for_prepare(runner, "local", assume_yes_install)
    runner.results.append(RunResult("prepare local", False, None, "toolchain not found"))
    return False


def prepare_selected_backends(
    runner: Runner,
    backend: str,
    assume_yes_prepare: bool,
    install_missing: bool = False,
    assume_yes_install: bool = False,
) -> int:
    targets = [backend] if backend != "auto" else ["docker", "podman", "wsl", "local"]
    ok = True
    for target in targets:
        if target == "docker":
            ok = prepare_docker_backend(runner, assume_yes_prepare, install_missing, assume_yes_install) and ok
        elif target == "podman":
            ok = prepare_podman_backend(runner, assume_yes_prepare, install_missing, assume_yes_install) and ok
        elif target == "wsl":
            ok = prepare_wsl_backend(runner, assume_yes_prepare, install_missing, assume_yes_install) and ok
        elif target == "local":
            ok = prepare_local_backend(runner, assume_yes_prepare, install_missing, assume_yes_install) and ok
    if not runner.results:
        runner.results.append(RunResult("prepare backends", ok, 0 if ok else 1, "no preparation commands were needed"))
    return runner.summary()


def ask_cli_backend(default: str) -> str:
    prompt = f"Backend [{default}] (auto/docker/podman/wsl/local): "
    try:
        answer = input(prompt).strip().lower()
    except EOFError:
        return "local"
    if not answer:
        return default
    return answer if answer in BACKENDS else default


def run_backend(runner: Runner, backend: str) -> bool:
    if backend == "docker":
        return container_engine_available(runner, "docker", record=True) and run_container_lanes(runner, "docker")
    if backend == "podman":
        return container_engine_available(runner, "podman", record=True) and run_container_lanes(runner, "podman")
    if backend == "wsl":
        return wsl_available(runner) and run_wsl_lanes(runner)
    if backend == "local":
        return local_available(runner) and run_local_lanes(runner)
    raise ValueError(backend)


def run_coverage_backend(runner: Runner, backend: str) -> bool:
    if backend == "docker":
        return container_engine_available(runner, "docker", record=True) and run_container_coverage(runner, "docker")
    if backend == "podman":
        return container_engine_available(runner, "podman", record=True) and run_container_coverage(runner, "podman")
    if backend == "wsl":
        return wsl_coverage_available(runner) and run_wsl_coverage(runner)
    if backend == "local":
        return local_coverage_available(runner) and run_local_coverage(runner)
    raise ValueError(backend)


def vendor_container_nominee_lines() -> list[str]:
    lines = ["Available vendor container nominees:"]
    for name, (key, label, image) in VENDOR_CONTAINER_NOMINEES.items():
        lines.append(f"  {name}: {key}@{label}={image}")
    return lines


def expand_vendor_container_nominees(use_all: bool, selected: list[str]) -> list[str]:
    names: list[str] = []
    if use_all:
        names.extend(VENDOR_CONTAINER_NOMINEES.keys())
    names.extend(selected)

    expanded: list[str] = []
    for name in names:
        if name not in VENDOR_CONTAINER_NOMINEES:
            valid = ", ".join(VENDOR_CONTAINER_NOMINEES.keys())
            raise ValueError("unknown vendor container nominee: " + name + ". Valid nominees: " + valid)
        key, label, image = VENDOR_CONTAINER_NOMINEES[name]
        expanded.append(f"{key}@{label}={image}")
    return expanded


def run_vendor_compiler_lanes(
    runner: Runner,
    enable_container_images: bool,
    container_images: list[str],
    container_engine: str,
    container_pull: str,
    container_pull_timeout: int,
) -> bool:
    cmd = [
        sys.executable,
        "-B",
        str(Path("testing") / "scripts" / "vendor_compiler_lanes.py"),
        "--root",
        "testing",
        "--sstl-root",
        ".",
        "--timeout",
        str(runner.timeout_seconds),
    ]
    if enable_container_images or container_images:
        cmd.append("--enable-container-images")
        cmd.extend(["--container-engine", container_engine])
        cmd.extend(["--container-pull", container_pull])
        cmd.extend(["--container-pull-timeout", str(container_pull_timeout)])
        for image in container_images:
            cmd.extend(["--container-image", image])
    return runner.run("vendor compiler lanes", cmd, ROOT, timeout=runner.timeout_seconds)


def resolve_vendor_container_engine(requested: str, backend: str) -> str:
    if requested:
        return requested
    if backend in ("docker", "podman"):
        return backend
    if container_engine_probe("docker", timeout=10)[0]:
        return "docker"
    if container_engine_probe("podman", timeout=10)[0]:
        return "podman"
    return "docker"


def run_coverage_selected(runner: Runner, backend: str) -> bool:
    if backend == "auto":
        backends = discover_coverage_backends(runner)
        if not backends:
            print_recommendations(runner)
            return log_coverage_report(runner)
        runner.log("")
        runner.log("Auto-selected coverage backend order: " + ", ".join(backends))
        for candidate in backends:
            runner.log("")
            runner.log("Trying coverage backend: " + candidate)
            if run_coverage_backend(runner, candidate):
                return True
            if candidate != backends[-1]:
                runner.log("")
                runner.log(f"Coverage backend {candidate} did not complete; trying next available backend.")
        return log_coverage_report(runner)
    return run_coverage_backend(runner, backend)


def run_selected(runner: Runner, backend: str, install_missing: bool = False, assume_yes_install: bool = False) -> int:
    if backend == "auto":
        backends = discover_backends(runner)
        if not backends:
            if install_missing and maybe_install_missing(runner, "auto", assume_yes_install):
                runner.log("Install command finished. Re-discovering backends.")
                backends = discover_backends(runner)
            if not backends:
                print_recommendations(runner)
                if not runner.results:
                    runner.results.append(RunResult("backend discovery", False, None, "no usable backend found"))
                return runner.summary()
        runner.log("")
        runner.log("Auto-selected backend order: " + ", ".join(backends))
        for candidate in backends:
            runner.log("")
            runner.log("Trying backend: " + candidate)
            before = len(runner.results)
            if run_backend(runner, candidate):
                runner.last_successful_backend = candidate
                publish_newest_runtime_csv(runner)
                return runner.summary()
            if candidate != backends[-1]:
                runner.log("")
                runner.log(f"Backend {candidate} did not complete; trying next available backend.")
            if len(runner.results) == before:
                runner.results.append(RunResult(candidate, False, None, "backend unavailable"))
        if install_missing:
            maybe_install_missing(runner, "auto", assume_yes_install)
        else:
            print_recommendations(runner)
    else:
        before = len(runner.results)
        if run_backend(runner, backend):
            runner.last_successful_backend = backend
            publish_newest_runtime_csv(runner)
        else:
            if len(runner.results) == before:
                runner.results.append(RunResult(backend + " backend", False, None, "backend unavailable"))
            if install_missing:
                if maybe_install_missing(runner, backend, assume_yes_install):
                    runner.log("Install command finished. Retrying backend: " + backend)
                    if run_backend(runner, backend):
                        runner.last_successful_backend = backend
                        publish_newest_runtime_csv(runner)
            else:
                runner.log("")
                runner.log(f"Strict backend '{backend}' failed. Choose Auto Fallback (Recommended) to try other available backends.")
                if backend == "wsl" and any(result.name == "wsl shell/tooling probe" and result.ok for result in runner.results):
                    runner.log("WSL tooling probe passed; this failure happened inside the validation lane, not during WSL installation/tool discovery.")
                else:
                    print_recommendations(runner, backend)
    return runner.summary()


def run_cli(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Compile and run SSTL C/C++ tests.")
    parser.add_argument("--backend", choices=BACKENDS, default=None, help="Backend to use. Local requires a tiny C/C++ CMake sanity build; local coverage also requires lcov/GNU-style coverage support. Default prompts in CLI and auto-selects in GUI.")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--yes-virtualized", action="store_true", help="Compatibility alias for --backend auto.")
    mode.add_argument("--no-virtualized", action="store_true", help="Compatibility alias for --backend local.")
    parser.add_argument("--wsl-distro", default=os.environ.get("SSTL_WSL_DISTRO", ""), help="WSL distro to use for the wsl backend, for example Ubuntu. Defaults to probing usable non-internal distros.")
    parser.add_argument("--dry-run", action="store_true", help="Print commands without executing them.")
    parser.add_argument("--install-missing", action="store_true", help="Print setup recommendations and ask before running a selected safe installer.")
    parser.add_argument("--yes-install", action="store_true", help="With --install-missing, run the selected install command without prompting.")
    parser.add_argument("--prepare-backends", action="store_true", help="Try to make the selected backend(s) available, for example by starting Docker Desktop, starting a Podman machine, or installing WSL validation/coverage tools.")
    parser.add_argument("--yes-prepare", action="store_true", help="With --prepare-backends, run preparation actions without prompting. Host installers still require --yes-install.")
    parser.add_argument("--coverage", action="store_true", help="After normal tests, generate and print an overall/per-file coverage report.")
    parser.add_argument("--coverage-only", action="store_true", help="Generate and print the coverage report without first running normal validation lanes. Local coverage requires lcov and a GNU/Clang-style coverage sanity probe.")
    parser.add_argument("--runtime-report", action="store_true", help="After normal tests, print an overall/per-comparison runtime CSV report.")
    parser.add_argument("--runtime-report-only", action="store_true", help="Print the newest runtime CSV report without first running normal validation lanes.")
    parser.add_argument("--quick-summary", action="store_true", help="After normal tests, print a compact status dashboard from latest artifacts.")
    parser.add_argument("--quick-summary-only", action="store_true", help="Print the compact status dashboard without first running normal validation lanes.")
    parser.add_argument("--vendor-container-images", dest="vendor_container_images", action="store_true", help="After normal tests, also run configured specialized vendor compiler container image probes.")
    parser.add_argument("--vendor-container-image", dest="vendor_container_image", action="append", default=[], metavar="KEY[@LABEL]=IMAGE", help="Vendor compiler image mapping; keys: armclang, arm-none-eabi, keil-armcc, iar-iccarm.")
    parser.add_argument("--vendor-container-nominees", dest="vendor_container_nominees", action="store_true", help="Use the four predefined public arm-none-eabi image nominees for initial validation.")
    parser.add_argument("--vendor-container-nominee", dest="vendor_container_nominee", action="append", choices=tuple(VENDOR_CONTAINER_NOMINEES.keys()), default=[], metavar="NAME", help="Use one predefined public arm-none-eabi image nominee.")
    parser.add_argument("--list-vendor-container-nominees", dest="list_vendor_container_nominees", action="store_true", help="Print predefined public vendor container image nominees and exit.")
    parser.add_argument("--vendor-container-engine", dest="vendor_container_engine", default=os.environ.get("SSTL_VENDOR_CONTAINER_ENGINE", ""), help="Docker-compatible engine for vendor compiler image probes. Defaults to the selected docker/podman backend when applicable.")
    parser.add_argument("--vendor-container-pull", dest="vendor_container_pull", choices=["ask", "never", "always"], default=os.environ.get("SSTL_VENDOR_CONTAINER_PULL", "ask"), help="Policy for missing configured vendor container images.")
    parser.add_argument("--vendor-container-pull-timeout", dest="vendor_container_pull_timeout", type=int, default=600, help="Per-image vendor container pull timeout in seconds.")
    parser.add_argument("--color", choices=["auto", "always", "never"], default="auto", help="Colorize CLI reports. Default: auto.")
    parser.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT_SECONDS, help="Per-command timeout in seconds.")
    parser.add_argument("--no-output-timeout", type=int, default=NO_OUTPUT_TIMEOUT_SECONDS, help="Abort a command after this many silent seconds.")
    args = parser.parse_args(argv)

    if args.list_vendor_container_nominees:
        print("\n".join(vendor_container_nominee_lines()))
        return 0

    runner = Runner(args.dry_run, args.timeout, args.no_output_timeout, color_enabled_from_mode(args.color))
    runner.wsl_distro = args.wsl_distro.strip()
    try:
        selected_vendor_container_images = args.vendor_container_image + expand_vendor_container_nominees(args.vendor_container_nominees, args.vendor_container_nominee)
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 2

    only_reports = args.runtime_report_only or args.coverage_only or args.quick_summary_only
    needs_backend = args.prepare_backends or args.coverage_only or not only_reports

    if needs_backend:
        if args.backend:
            backend = args.backend
        elif args.yes_virtualized:
            backend = "auto"
        elif args.no_virtualized:
            backend = "local"
        else:
            backend = ask_cli_backend(default="auto")
    else:
        backend = args.backend or "auto"

    if args.prepare_backends:
        return prepare_selected_backends(
            runner,
            backend,
            args.yes_prepare,
            install_missing=args.install_missing,
            assume_yes_install=args.yes_install,
        )

    if only_reports:
        if args.runtime_report_only:
            log_runtime_report(runner)
        if args.coverage_only:
            run_coverage_selected(runner, backend)
        if args.quick_summary_only:
            log_quick_summary(runner)
        return runner.summary()

    result = run_selected(runner, backend, args.install_missing, args.yes_install)
    ran_extra_report = False
    if result == 0 and (args.vendor_container_images or selected_vendor_container_images):
        vendor_engine = resolve_vendor_container_engine(args.vendor_container_engine, runner.last_successful_backend or backend)
        result = 0 if run_vendor_compiler_lanes(
            runner,
            args.vendor_container_images or bool(selected_vendor_container_images),
            selected_vendor_container_images,
            vendor_engine,
            args.vendor_container_pull,
            args.vendor_container_pull_timeout,
        ) else 1
        ran_extra_report = True
    if result == 0 and args.runtime_report:
        log_runtime_report(runner)
        ran_extra_report = True
    if result == 0 and args.coverage:
        run_coverage_selected(runner, backend)
        ran_extra_report = True
    if result == 0 and args.quick_summary:
        log_quick_summary(runner)
        ran_extra_report = True
    if ran_extra_report:
        return runner.summary()
    return result


def find_runtime_csv_artifacts(include_artifacts: bool = True) -> list[Path]:
    artifacts: list[Path] = []
    roots = (ARTIFACT_ROOT, TEST_ROOT) if include_artifacts else (TEST_ROOT,)
    for root in roots:
        if not root.exists():
            continue
        try:
            for path in root.rglob(RUNTIME_CSV_NAME):
                if path.is_file() and path not in artifacts:
                    artifacts.append(path)
        except Exception:
            continue
    def sort_key(path: Path) -> tuple[int, float]:
        parts = {part.lower() for part in path.parts}
        build_dir = path.parent.name.lower()
        if "artifacts" in parts:
            benchmark_priority = 3
        elif "coverage" in parts:
            benchmark_priority = 0
        elif build_dir in {"host-debug", "host-local-return", "codex-runtime-check"}:
            benchmark_priority = 2
        else:
            benchmark_priority = 1
        return (benchmark_priority, path.stat().st_mtime if path.exists() else 0.0)

    return sorted(artifacts, key=sort_key, reverse=True)


def newest_runtime_csv_artifact(include_artifacts: bool = True) -> Path | None:
    artifacts = find_runtime_csv_artifacts(include_artifacts=include_artifacts)
    return artifacts[0] if artifacts else None


def read_runtime_csv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


def human_bytes(value: float) -> str:
    units = ["B", "KiB", "MiB", "GiB"]
    size = float(value)
    unit = units[0]
    for unit in units:
        if size < 1024.0 or unit == units[-1]:
            break
        size /= 1024.0
    return f"{size:.1f} {unit}" if unit != "B" else f"{int(size)} B"


def runtime_float(row: dict[str, str], key: str) -> float:
    try:
        return float(row.get(key) or 0.0)
    except ValueError:
        return 0.0


def runtime_int(row: dict[str, str], key: str) -> int:
    try:
        return int(float(row.get(key) or 0))
    except ValueError:
        return 0


def runtime_family(interface: str) -> str:
    if interface.startswith("stl"):
        return "stl"
    return interface


def runtime_group_order(rows: list[dict[str, str]]) -> list[str]:
    order: list[str] = []
    for row in rows:
        group = row.get("group", "")
        if group not in order:
            order.append(group)
    return order


def runtime_rows_by_group(rows: list[dict[str, str]]) -> dict[str, list[dict[str, str]]]:
    grouped: dict[str, list[dict[str, str]]] = {}
    for row in rows:
        grouped.setdefault(row.get("group", ""), []).append(row)
    return grouped


def runtime_checksum_mismatches(rows: list[dict[str, str]]) -> list[str]:
    mismatches: list[str] = []
    grouped = runtime_rows_by_group(rows)
    for group in runtime_group_order(rows):
        checksums = {row.get("checksum", "") for row in grouped.get(group, []) if row.get("checksum", "")}
        if len(checksums) > 1:
            mismatches.append(group)
    return mismatches


def runtime_family_aggregates(rows: list[dict[str, str]]) -> list[dict[str, float | int | str]]:
    order: list[str] = []
    aggregates: dict[str, dict[str, float | int | str]] = {}
    for row in rows:
        family = runtime_family(row.get("interface", ""))
        if family not in aggregates:
            order.append(family)
            aggregates[family] = {
                "family": family,
                "cases": 0,
                "cpu_seconds": 0.0,
                "total_bytes": 0,
                "max_bytes": 0,
                "cpu_diff_sum": 0.0,
                "memory_diff_sum": 0.0,
            }
        item = aggregates[family]
        item["cases"] = int(item["cases"]) + 1
        item["cpu_seconds"] = float(item["cpu_seconds"]) + runtime_float(row, "cpu_seconds")
        total_bytes = runtime_int(row, "total_bytes")
        item["total_bytes"] = int(item["total_bytes"]) + total_bytes
        item["max_bytes"] = max(int(item["max_bytes"]), total_bytes)
        item["cpu_diff_sum"] = float(item["cpu_diff_sum"]) + runtime_float(row, "cpu_diff_percent")
        item["memory_diff_sum"] = float(item["memory_diff_sum"]) + runtime_float(row, "memory_diff_percent")
    for item in aggregates.values():
        cases = max(1, int(item["cases"]))
        item["avg_cpu_seconds"] = float(item["cpu_seconds"]) / cases
        item["avg_cpu_diff_percent"] = float(item["cpu_diff_sum"]) / cases
        item["avg_memory_diff_percent"] = float(item["memory_diff_sum"]) / cases
    return [aggregates[name] for name in order]


def runtime_manifest_path() -> Path:
    return TEST_ROOT / "manifests" / "runtime_summary.yaml"


def yaml_quote(value: object) -> str:
    text_value = str(value)
    return '"' + text_value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def write_runtime_summary_manifest(path: Path, rows: list[dict[str, str]]) -> Path:
    out = runtime_manifest_path()
    out.parent.mkdir(parents=True, exist_ok=True)
    mismatches = runtime_checksum_mismatches(rows)
    groups = runtime_group_order(rows)
    status = "pass" if rows and not mismatches else "fail"
    lines = [
        f"generated_at_utc: {datetime.now(timezone.utc).isoformat()}",
        f"source: {yaml_quote(display_artifact_path(path))}",
        f"status: {status}",
        f"group_count: {len(groups)}",
        f"row_count: {len(rows)}",
        "notes:",
        "  - memory_diff_percent uses the first row in each comparison group as the baseline denominator.",
        "  - Very large memory percentages are expected when a tiny view object is compared with a heap-backed STL proxy row.",
        "families:",
    ]
    for item in runtime_family_aggregates(rows):
        lines.extend(
            [
                f"  - family: {yaml_quote(item['family'])}",
                f"    cases: {item['cases']}",
                f"    cpu_seconds: {float(item['cpu_seconds']):.9f}",
                f"    avg_cpu_seconds: {float(item['avg_cpu_seconds']):.9f}",
                f"    total_bytes: {item['total_bytes']}",
                f"    max_bytes: {item['max_bytes']}",
                f"    avg_cpu_diff_percent: {float(item['avg_cpu_diff_percent']):.6f}",
                f"    avg_memory_diff_percent: {float(item['avg_memory_diff_percent']):.6f}",
            ]
        )
    lines.append("rows:")
    for row in rows:
        lines.extend(
            [
                f"  - group: {yaml_quote(row.get('group', ''))}",
                f"    interface: {yaml_quote(row.get('interface', ''))}",
                f"    object_bytes: {runtime_int(row, 'object_bytes')}",
                f"    storage_bytes: {runtime_int(row, 'storage_bytes')}",
                f"    total_bytes: {runtime_int(row, 'total_bytes')}",
                f"    cpu_seconds: {runtime_float(row, 'cpu_seconds'):.9f}",
                f"    cpu_diff_percent: {runtime_float(row, 'cpu_diff_percent'):.6f}",
                f"    memory_diff_percent: {runtime_float(row, 'memory_diff_percent'):.6f}",
                f"    checksum: {yaml_quote(row.get('checksum', ''))}",
            ]
        )
    out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return out


def runtime_report_lines(path: Path, rows: list[dict[str, str]], color: bool = False) -> list[str]:
    groups = runtime_group_order(rows)
    mismatches = runtime_checksum_mismatches(rows)
    status = "pass" if rows and not mismatches else "fail"
    total_cpu = sum(runtime_float(row, "cpu_seconds") for row in rows)
    max_bytes = max((runtime_int(row, "total_bytes") for row in rows), default=0)
    lines = [
        color_text("== Runtime report", "bold", color),
        f"Source: {display_artifact_path(path)}",
        (
            f"Comparisons: {len(groups)}, rows: {len(rows)}, "
            f"total CPU: {total_cpu:.6f} sec, max footprint: {human_bytes(max_bytes)}, "
            f"status {color_status(status, color)}"
        ),
        "Diff note: cpu/memory percentages are relative to the first row in each comparison group; tiny baselines can produce very large memory percentages.",
        "",
        color_text("Overall by family", "cyan", color),
        "family       cases   total cpu    avg cpu      footprint sum  max footprint  avg cpu diff  avg mem diff",
    ]
    for item in runtime_family_aggregates(rows):
        cases = int(item["cases"])
        family = str(item["family"])
        family_color = "green" if family in {"c-sstl", "cpp-sstl"} else "yellow"
        lines.append(
            f"{color_text(f'{family:<11}', family_color, color)} "
            f"{cases:5d} "
            f"{float(item['cpu_seconds']):11.6f} "
            f"{float(item['avg_cpu_seconds']):10.6f} "
            f"{human_bytes(float(item['total_bytes'])):>14} "
            f"{human_bytes(float(item['max_bytes'])):>14} "
            f"{color_diff(float(item['avg_cpu_diff_percent']), color):>13} "
            f"{color_diff(float(item['avg_memory_diff_percent']), color):>13}"
        )
    lines.extend(
        [
            "",
            color_text("Per comparison", "cyan", color),
            "group                                      interface    cpu sec    cpu diff     footprint     mem diff      checksum",
        ]
    )
    grouped = runtime_rows_by_group(rows)
    for group in groups:
        for row in grouped[group]:
            group_text = group if len(group) <= 40 else group[:37] + "..."
            interface = row.get("interface", "")
            interface_color = "green" if interface in {"c-sstl", "cpp-sstl"} else "yellow"
            lines.append(
                f"{group_text:<42} "
                f"{color_text(f'{interface:<11}', interface_color, color)} "
                f"{runtime_float(row, 'cpu_seconds'):9.6f} "
                f"{color_diff(runtime_float(row, 'cpu_diff_percent'), color)} "
                f"{human_bytes(runtime_int(row, 'total_bytes')):>12} "
                f"{color_diff(runtime_float(row, 'memory_diff_percent'), color):>13} "
                f"{row.get('checksum', '')}"
            )
    if mismatches:
        lines.extend(["", color_text("Checksum mismatches:", "red", color)])
        lines.extend(color_text("  " + group, "red", color) for group in mismatches)
    return lines


def log_runtime_report(runner: Runner, path: Path | None = None) -> bool:
    artifact = path or newest_runtime_csv_artifact()
    if not artifact:
        runner.log("== Runtime report")
        runner.log("No runtime CSV artifact was found. Run the validation lane first, or use --backend auto --runtime-report after a full run.")
        runner.results.append(RunResult("runtime report", False, None, "no runtime CSV artifact found"))
        return False
    try:
        rows = read_runtime_csv(artifact)
    except Exception as exc:
        runner.log("== Runtime report")
        runner.log(f"Could not read runtime CSV artifact: {artifact}")
        runner.log(str(exc))
        runner.results.append(RunResult("runtime report", False, None, "runtime CSV could not be read"))
        return False
    if not rows:
        runner.log("== Runtime report")
        runner.log(f"The runtime CSV artifact is empty: {artifact}")
        runner.results.append(RunResult("runtime report", False, None, "runtime CSV artifact was empty"))
        return False
    try:
        artifact = publish_to_artifacts(artifact, RUNTIME_CSV_NAME)
    except Exception as exc:
        runner.log(f"Could not copy runtime CSV into artifacts: {exc}")
    manifest = write_runtime_summary_manifest(artifact, rows)
    for line in runtime_report_lines(artifact, rows, runner.color_enabled):
        runner.log(line)
    runner.log("")
    runner.log(f"Runtime manifest: {manifest}")
    ok = not runtime_checksum_mismatches(rows)
    runner.results.append(RunResult("runtime report", ok, 0 if ok else 1, "checksum mismatch" if not ok else ""))
    return True


def command_results_status(results: list[RunResult]) -> tuple[str, str]:
    relevant = [item for item in results if item.name != "quick summary"]
    if not relevant:
        return "warn", "no command results in this invocation"
    failed = [item for item in relevant if not item.ok]
    if failed:
        return "fail", f"{len(relevant) - len(failed)}/{len(relevant)} command results passed"
    return "pass", f"{len(relevant)}/{len(relevant)} command results passed"


def coverage_weakness_score(item: CoverageFile) -> float:
    line_score = item.line_pct - COVERAGE_LINE_MIN
    branch_score = item.branch_pct - COVERAGE_BRANCH_MIN if item.branches_found else 0.0
    return min(line_score, branch_score)


def quick_summary_lines(results: list[RunResult], color: bool = False) -> tuple[list[str], str]:
    command_status, command_detail = command_results_status(results)
    coverage_artifact = newest_coverage_artifact()
    coverage_report: CoverageReport | None = None
    coverage_status = "warn"
    coverage_detail = "no coverage artifact found"
    if coverage_artifact:
        try:
            coverage_artifact = publish_to_artifacts(coverage_artifact, COVERAGE_INFO_NAME)
            coverage_report = parse_lcov_info(coverage_artifact)
            coverage_status = coverage_report.status
            coverage_detail = (
                f"line {color_pct(coverage_report.line_pct, COVERAGE_LINE_MIN, color)} "
                f"({coverage_report.lines_hit}/{coverage_report.lines_found}), "
                f"branch {color_pct(coverage_report.branch_pct, COVERAGE_BRANCH_MIN, color)} "
                f"({coverage_report.branches_hit}/{coverage_report.branches_found}), "
                f"{len(coverage_report.files)} files"
            )
        except Exception as exc:
            coverage_status = "fail"
            coverage_detail = f"could not parse coverage artifact: {exc}"

    runtime_artifact = newest_runtime_csv_artifact()
    runtime_rows: list[dict[str, str]] = []
    runtime_status = "warn"
    runtime_detail = "no runtime CSV artifact found"
    if runtime_artifact:
        try:
            runtime_artifact = publish_to_artifacts(runtime_artifact, RUNTIME_CSV_NAME)
            runtime_rows = read_runtime_csv(runtime_artifact)
            mismatches = runtime_checksum_mismatches(runtime_rows)
            runtime_status = "pass" if runtime_rows and not mismatches else "fail"
            groups = runtime_group_order(runtime_rows)
            total_cpu = sum(runtime_float(row, "cpu_seconds") for row in runtime_rows)
            max_bytes = max((runtime_int(row, "total_bytes") for row in runtime_rows), default=0)
            runtime_detail = (
                f"{len(groups)} comparisons, {len(runtime_rows)} rows, "
                f"total CPU {total_cpu:.6f} sec, max footprint {human_bytes(max_bytes)}"
            )
            if mismatches:
                runtime_detail += f", {len(mismatches)} checksum mismatch groups"
        except Exception as exc:
            runtime_status = "fail"
            runtime_detail = f"could not parse runtime artifact: {exc}"

    statuses = [command_status, coverage_status, runtime_status]
    overall_status = "fail" if "fail" in statuses else "warn" if "warn" in statuses else "pass"
    lines = [
        "",
        color_text("== Quick summary", "bold", color),
        f"Overall: {color_status(overall_status, color)}",
        f"Commands: {color_status(command_status, color)} - {command_detail}",
        f"Coverage: {color_status(coverage_status, color)} - {coverage_detail}",
        f"Runtime:  {color_status(runtime_status, color)} - {runtime_detail}",
    ]

    if coverage_artifact:
        lines.append(f"Coverage artifact: {coverage_artifact}")
    if runtime_artifact:
        lines.append(f"Runtime artifact:  {runtime_artifact}")

    if coverage_report and coverage_report.files:
        weak = sorted(coverage_report.files, key=lambda item: (coverage_weakness_score(item), item.path))[:5]
        lines.extend(["", color_text("Weakest coverage files", "cyan", color)])
        for item in weak:
            branch = (
                color_pct(item.branch_pct, COVERAGE_BRANCH_MIN, color)
                if item.branches_found
                else color_text("   n/a", "dim", color)
            )
            lines.append(
                f"{color_status(item.status, color):<6} "
                f"line {color_pct(item.line_pct, COVERAGE_LINE_MIN, color)} "
                f"branch {branch} "
                f"{item.path}"
            )

    if runtime_rows:
        lines.extend(["", color_text("Runtime family totals", "cyan", color)])
        for item in runtime_family_aggregates(runtime_rows):
            family = str(item["family"])
            family_color = "green" if family in {"c-sstl", "cpp-sstl"} else "yellow"
            lines.append(
                f"{color_text(f'{family:<9}', family_color, color)} "
                f"cases {int(item['cases']):2d}, "
                f"cpu {float(item['cpu_seconds']):.6f} sec, "
                f"max {human_bytes(float(item['max_bytes']))}, "
                f"avg cpu diff {color_diff(float(item['avg_cpu_diff_percent']), color)}, "
                f"avg mem diff {color_diff(float(item['avg_memory_diff_percent']), color)}"
            )

    return lines, overall_status


def log_quick_summary(runner: Runner) -> bool:
    lines, status = quick_summary_lines(runner.results, runner.color_enabled)
    for line in lines:
        runner.log(line)
    detail = "" if status == "pass" else "warnings present" if status == "warn" else "status failed"
    runner.results.append(RunResult("quick summary", status != "fail", 0 if status != "fail" else 1, detail))
    return status != "fail"


def run_gui() -> int:
    try:
        import tkinter as tk
        from tkinter import messagebox, scrolledtext
    except Exception:
        return run_cli(["--no-virtualized"])

    root = tk.Tk()
    root.title("SSTL Test Runner")
    root.geometry("920x640")

    output: "queue.Queue[str | tuple[str, int]]" = queue.Queue()
    active_runner: list[Runner | None] = [None]

    text = scrolledtext.ScrolledText(root, wrap=tk.WORD)
    attach_text_copy_context_menu(text)
    text.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

    controls = tk.Frame(root)
    controls.pack(fill=tk.X, padx=8, pady=(0, 8))

    def write(message: str) -> None:
        text.insert(tk.END, message)
        text.see(tk.END)

    class GuiRunner(Runner):
        def __init__(self) -> None:
            super().__init__(color_enabled=False)

        def log(self, message: str) -> None:
            output.put(message + "\n")

    class ProbeRunner(Runner):
        def __init__(self) -> None:
            super().__init__(color_enabled=False)
            self.messages: list[str] = []

        def log(self, message: str) -> None:
            self.messages.append(message)

    def probe_with_reason(check) -> tuple[bool, str]:
        runner = ProbeRunner()
        ok = check(runner)
        return ok, runner.messages[-1] if runner.messages else "probe did not provide a reason"

    strict_docker_available, _, strict_docker_reason = container_engine_probe("docker", timeout=10)
    strict_podman_available, _, strict_podman_reason = container_engine_probe("podman", timeout=10)
    strict_local_available, strict_local_reason = probe_with_reason(local_available)
    strict_local_coverage_available, strict_local_coverage_reason = probe_with_reason(local_coverage_available)
    strict_wsl_available = select_wsl_distro(ProbeRunner()) is not None
    strict_wsl_coverage_available = select_wsl_coverage_distro(ProbeRunner()) is not None

    def backend_button_text(label: str, available: bool = True) -> str:
        return label if available else label + " (Unavailable)"

    def coverage_button_text(label: str, available: bool = True) -> str:
        text = label + " Coverage"
        return text if available else text + " (Unavailable)"

    def worker(backend: str, install_missing: bool, coverage_only: bool = False, prepare: bool = False) -> None:
        runner = GuiRunner()
        active_runner[0] = runner
        if prepare:
            runner.log("GUI selected action: prepare backends")
            runner.log("GUI selected backend: " + backend)
            runner.log("Preparation can start installed backend services and install missing WSL validation/coverage tools.")
            code = prepare_selected_backends(runner, backend, True, install_missing=False, assume_yes_install=False)
            refresh_backend_availability()
        elif coverage_only:
            runner.log("GUI selected action: coverage report")
            runner.log("GUI selected backend: " + backend)
            run_coverage_selected(runner, backend)
            code = runner.summary()
        elif install_missing:
            runner.log("GUI selected action: install recommendations")
            runner.log("GUI install flow: recommendations will be printed here.")
            runner.log("For safety, run an installer from a terminal with:")
            runner.log("  python tools/sstl_run_tests.py --backend auto --install-missing")
            print_recommendations(runner)
            runner.results.append(RunResult("install recommendations", True, 0, "printed only; no commands run"))
            code = runner.summary()
        else:
            runner.log("GUI selected action: validation")
            runner.log("GUI selected backend: " + backend)
            code = run_selected(runner, backend, False, False)
        output.put(("prepare" if prepare else "coverage" if coverage_only else "install" if install_missing else "tests", code))

    def set_running(running: bool) -> None:
        auto_btn.config(state=tk.DISABLED if running else tk.NORMAL)
        docker_only_btn.config(
            text=backend_button_text("Docker", strict_docker_available),
            state=tk.DISABLED if running or not strict_docker_available else tk.NORMAL,
        )
        local_btn.config(
            text=backend_button_text("Local", strict_local_available),
            state=tk.DISABLED if running or not strict_local_available else tk.NORMAL,
        )
        podman_btn.config(
            text=backend_button_text("Podman", strict_podman_available),
            state=tk.DISABLED if running or not strict_podman_available else tk.NORMAL,
        )
        wsl_btn.config(
            text=backend_button_text("WSL", strict_wsl_available),
            state=tk.DISABLED if running or not strict_wsl_available else tk.NORMAL,
        )
        install_btn.config(state=tk.DISABLED if running else tk.NORMAL)
        prepare_btn.config(state=tk.DISABLED if running else tk.NORMAL)
        quick_summary_btn.config(state=tk.DISABLED if running else tk.NORMAL)
        coverage_auto_btn.config(state=tk.DISABLED if running else tk.NORMAL)
        coverage_docker_btn.config(
            text=coverage_button_text("Docker", strict_docker_available),
            state=tk.DISABLED if running or not strict_docker_available else tk.NORMAL,
        )
        coverage_podman_btn.config(
            text=coverage_button_text("Podman", strict_podman_available),
            state=tk.DISABLED if running or not strict_podman_available else tk.NORMAL,
        )
        coverage_wsl_btn.config(
            text=coverage_button_text("WSL", strict_wsl_coverage_available),
            state=tk.DISABLED if running or not strict_wsl_coverage_available else tk.NORMAL,
        )
        coverage_local_btn.config(
            text=coverage_button_text("Local", strict_local_coverage_available),
            state=tk.DISABLED if running or not strict_local_coverage_available else tk.NORMAL,
        )
        chart_btn.config(state=tk.DISABLED if running else tk.NORMAL)
        help_btn.config(state=tk.DISABLED if running else tk.NORMAL)
        cancel_btn.config(state=tk.NORMAL if running else tk.DISABLED)

    def start(backend: str, install_missing: bool = False, coverage_only: bool = False, prepare: bool = False) -> None:
        set_running(True)
        threading.Thread(target=worker, args=(backend, install_missing, coverage_only, prepare), daemon=True).start()

    def start_prepare() -> None:
        if not messagebox.askyesno(
            "Prepare Backends",
            "Try to make installed backends available now?\n\n"
            "This can start Docker Desktop, initialize or start a Podman machine, "
            "and install CMake/Ninja/GCC/Clang/LCOV inside a suitable WSL distro.",
            default=messagebox.NO,
        ):
            return
        start("auto", False, False, True)

    def refresh_backend_availability() -> None:
        nonlocal strict_docker_available, strict_docker_reason, strict_podman_available, strict_podman_reason, strict_local_available, strict_local_reason, strict_local_coverage_available, strict_local_coverage_reason, strict_wsl_available, strict_wsl_coverage_available
        strict_docker_available, _, strict_docker_reason = container_engine_probe("docker", timeout=10)
        strict_podman_available, _, strict_podman_reason = container_engine_probe("podman", timeout=10)
        strict_local_available, strict_local_reason = probe_with_reason(local_available)
        strict_local_coverage_available, strict_local_coverage_reason = probe_with_reason(local_coverage_available)
        strict_wsl_available = select_wsl_distro(ProbeRunner()) is not None
        strict_wsl_coverage_available = select_wsl_coverage_distro(ProbeRunner()) is not None

    def cancel() -> None:
        if active_runner[0]:
            active_runner[0].cancel()
            write("\nCancellation requested. The current subprocess will be terminated.\n")

    def show_help() -> None:
        messagebox.showinfo("SSTL Test Runner Help", HELP_TEXT)

    def show_quick_summary_window() -> None:
        last_results = list(active_runner[0].results) if active_runner[0] else []
        lines, status = quick_summary_lines(last_results, False)
        win = tk.Toplevel(root)
        win.title("SSTL Quick Summary")
        win.geometry("980x560")

        header_color = "#1f7a3f" if status == "pass" else "#a15c00" if status == "warn" else "#b42318"
        header = tk.Label(
            win,
            text=f"Quick summary status: {status}",
            anchor="w",
            font=("Segoe UI", 10, "bold"),
            fg=header_color,
        )
        header.pack(fill=tk.X, padx=10, pady=(8, 4))

        view = scrolledtext.ScrolledText(win, wrap=tk.WORD, height=24)
        attach_text_copy_context_menu(view)
        view.pack(fill=tk.BOTH, expand=True, padx=10, pady=(0, 10))
        view.tag_configure("heading", foreground="#1f4e79", font=("Consolas", 10, "bold"))
        view.tag_configure("pass", foreground="#1f7a3f")
        view.tag_configure("fail", foreground="#b42318")
        view.tag_configure("warn", foreground="#a15c00")
        view.tag_configure("normal", foreground="#202124")

        def tag_for_line(line: str) -> str:
            lowered = line.lower()
            if line.startswith("==") or line in {"Weakest coverage files", "Runtime family totals"}:
                return "heading"
            if "fail" in lowered or "status failed" in lowered:
                return "fail"
            if "warn" in lowered or "no " in lowered or "warning" in lowered:
                return "warn"
            if "pass" in lowered:
                return "pass"
            return "normal"

        for line in lines:
            view.insert(tk.END, line + "\n", tag_for_line(line))
        view.config(state=tk.DISABLED)
        set_minimum_window_size(win, 980, 560)

    def show_runtime_chart(path: Path | None = None, announce_errors: bool = True) -> None:
        csv_path = path or newest_runtime_csv_artifact()
        if not csv_path:
            if announce_errors:
                messagebox.showinfo("Runtime Chart", "No runtime CSV artifact was found yet. Run the runtime comparison first.")
            return
        try:
            rows = read_runtime_csv(csv_path)
        except Exception as exc:
            if announce_errors:
                messagebox.showerror("Runtime Chart", f"Could not read runtime CSV artifact:\n{csv_path}\n\n{exc}")
            return
        if not rows:
            if announce_errors:
                messagebox.showinfo("Runtime Chart", f"The runtime CSV artifact is empty:\n{csv_path}")
            return

        grouped: dict[str, list[dict[str, str]]] = {}
        order: list[str] = []
        for row in rows:
            group = row.get("group", "")
            if group not in grouped:
                grouped[group] = []
                order.append(group)
            grouped[group].append(row)

        max_cpu = max(float(row.get("cpu_seconds") or 0.0) for row in rows) or 1.0
        max_mem = max(float(row.get("total_bytes") or 0.0) for row in rows) or 1.0
        colors = {
            "c-sstl": "#2f80ed",
            "cpp-sstl": "#219653",
            "stl": "#f2994a",
        }

        win = tk.Toplevel(root)
        win.title("SSTL Runtime Comparison")
        win.geometry("1180x760")

        header = tk.Label(
            win,
            text=f"Runtime artifact: {csv_path}",
            anchor="w",
            font=("Segoe UI", 9),
        )
        header.pack(fill=tk.X, padx=10, pady=(8, 4))

        frame = tk.Frame(win)
        frame.pack(fill=tk.BOTH, expand=True)
        canvas = tk.Canvas(frame, bg="white")
        yscroll = tk.Scrollbar(frame, orient=tk.VERTICAL, command=canvas.yview)
        canvas.configure(yscrollcommand=yscroll.set)
        yscroll.pack(side=tk.RIGHT, fill=tk.Y)
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        width = 1120
        left = 24
        group_x = 34
        name_x = 390
        cpu_x = 520
        mem_x = 820
        bar_w = 210
        row_h = 28
        y = 24

        canvas.create_text(left, y, text="Group", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(name_x, y, text="Interface", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(cpu_x, y, text="CPU seconds", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(mem_x, y, text="Total memory", anchor="w", font=("Segoe UI", 9, "bold"))
        y += 28

        for group in order:
            canvas.create_text(group_x, y + 4, text=group, anchor="w", font=("Segoe UI", 9, "bold"))
            y += 24
            for row in grouped[group]:
                name = row.get("interface", "")
                cpu = float(row.get("cpu_seconds") or 0.0)
                mem = float(row.get("total_bytes") or 0.0)
                color = colors.get(name, "#6c757d")
                cpu_w = max(1, int((cpu / max_cpu) * bar_w))
                mem_w = max(1, int((mem / max_mem) * bar_w))

                canvas.create_text(name_x, y + 10, text=name, anchor="w", font=("Segoe UI", 9))
                canvas.create_rectangle(cpu_x, y + 2, cpu_x + bar_w, y + 18, outline="#dddddd", fill="#f8f8f8")
                canvas.create_rectangle(cpu_x, y + 2, cpu_x + cpu_w, y + 18, outline=color, fill=color)
                canvas.create_text(cpu_x + bar_w + 8, y + 10, text=f"{cpu:.6f}", anchor="w", font=("Segoe UI", 8))

                canvas.create_rectangle(mem_x, y + 2, mem_x + bar_w, y + 18, outline="#dddddd", fill="#f8f8f8")
                canvas.create_rectangle(mem_x, y + 2, mem_x + mem_w, y + 18, outline=color, fill=color)
                canvas.create_text(mem_x + bar_w + 8, y + 10, text=human_bytes(mem), anchor="w", font=("Segoe UI", 8))
                y += row_h
            y += 14

        legend_y = y + 8
        legend_x = left
        for name, color in colors.items():
            canvas.create_rectangle(legend_x, legend_y, legend_x + 16, legend_y + 16, outline=color, fill=color)
            canvas.create_text(legend_x + 22, legend_y + 8, text=name, anchor="w", font=("Segoe UI", 9))
            legend_x += 110
        y = legend_y + 36

        canvas.configure(scrollregion=(0, 0, width, y))
        set_minimum_window_size(win, 1180, 760)

    def show_coverage_report(path: Path | None = None, announce_errors: bool = True) -> None:
        coverage_path = path or newest_coverage_artifact()
        if not coverage_path:
            if announce_errors:
                messagebox.showinfo("Coverage Report", "No coverage artifact was found yet. Use the Coverage row to generate one.")
            return
        try:
            report = parse_lcov_info(coverage_path)
        except Exception as exc:
            if announce_errors:
                messagebox.showerror("Coverage Report", f"Could not read coverage artifact:\n{coverage_path}\n\n{exc}")
            return

        win = tk.Toplevel(root)
        win.title("SSTL Coverage Report")
        win.geometry("1180x760")

        overall = (
            f"Coverage artifact: {coverage_path}\n"
            f"Overall line {pct_text(report.line_pct)} ({report.lines_hit}/{report.lines_found}), "
            f"branch {pct_text(report.branch_pct)} ({report.branches_hit}/{report.branches_found}), "
            f"status {report.status}"
        )
        header = tk.Label(win, text=overall, anchor="w", justify=tk.LEFT, font=("Segoe UI", 9))
        header.pack(fill=tk.X, padx=10, pady=(8, 4))

        frame = tk.Frame(win)
        frame.pack(fill=tk.BOTH, expand=True)
        canvas = tk.Canvas(frame, bg="white")
        yscroll = tk.Scrollbar(frame, orient=tk.VERTICAL, command=canvas.yview)
        canvas.configure(yscrollcommand=yscroll.set)
        yscroll.pack(side=tk.RIGHT, fill=tk.Y)
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        width = 1120
        file_x = 24
        line_x = 560
        branch_x = 800
        bar_w = 190
        row_h = 28
        y = 24
        canvas.create_text(file_x, y, text="File", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(line_x, y, text="Line coverage", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(branch_x, y, text="Branch coverage", anchor="w", font=("Segoe UI", 9, "bold"))
        y += 28

        for item in report.files:
            color = "#219653" if item.status == "pass" else "#d64545"
            line_w = max(1, int(item.line_pct * bar_w))
            branch_w = max(1, int(item.branch_pct * bar_w)) if item.branches_found else 0
            file_text = item.path if len(item.path) <= 82 else "..." + item.path[-79:]
            canvas.create_text(file_x, y + 10, text=file_text, anchor="w", font=("Segoe UI", 8))

            canvas.create_rectangle(line_x, y + 2, line_x + bar_w, y + 18, outline="#dddddd", fill="#f8f8f8")
            canvas.create_rectangle(line_x, y + 2, line_x + line_w, y + 18, outline=color, fill=color)
            canvas.create_text(line_x + bar_w + 8, y + 10, text=f"{pct_text(item.line_pct)} {item.lines_hit}/{item.lines_found}", anchor="w", font=("Segoe UI", 8))

            canvas.create_rectangle(branch_x, y + 2, branch_x + bar_w, y + 18, outline="#dddddd", fill="#f8f8f8")
            if item.branches_found:
                canvas.create_rectangle(branch_x, y + 2, branch_x + branch_w, y + 18, outline=color, fill=color)
                branch_text = f"{pct_text(item.branch_pct)} {item.branches_hit}/{item.branches_found}"
            else:
                branch_text = "n/a"
            canvas.create_text(branch_x + bar_w + 8, y + 10, text=branch_text, anchor="w", font=("Segoe UI", 8))
            y += row_h

        canvas.configure(scrollregion=(0, 0, width, y + 24))
        set_minimum_window_size(win, 1180, 760)

    for column in range(12):
        controls.grid_columnconfigure(column, weight=0)
    controls.grid_columnconfigure(5, minsize=12)

    def place_button(button: "tk.Button", row: int, column: int, padx: tuple[int, int] = (0, 6)) -> None:
        button.grid(row=row, column=column, sticky="ew", padx=padx, pady=(0, 4))

    auto_btn = tk.Button(controls, text="Auto Fallback", command=lambda: start("auto"))
    place_button(auto_btn, 0, 0)
    local_btn = tk.Button(
        controls,
        text=backend_button_text("Local", strict_local_available),
        command=lambda: start("local"),
        state=tk.NORMAL if strict_local_available else tk.DISABLED,
    )
    place_button(local_btn, 0, 1)
    docker_only_btn = tk.Button(
        controls,
        text=backend_button_text("Docker", strict_docker_available),
        command=lambda: start("docker"),
        state=tk.NORMAL if strict_docker_available else tk.DISABLED,
    )
    place_button(docker_only_btn, 0, 2)
    podman_btn = tk.Button(
        controls,
        text=backend_button_text("Podman", strict_podman_available),
        command=lambda: start("podman"),
        state=tk.NORMAL if strict_podman_available else tk.DISABLED,
    )
    place_button(podman_btn, 0, 3)
    wsl_btn = tk.Button(
        controls,
        text=backend_button_text("WSL", strict_wsl_available),
        command=lambda: start("wsl"),
        state=tk.NORMAL if strict_wsl_available else tk.DISABLED,
    )
    place_button(wsl_btn, 0, 4)
    install_btn = tk.Button(controls, text="Recommend / Install", command=lambda: start("auto", True))
    place_button(install_btn, 0, 6)
    prepare_btn = tk.Button(controls, text="Prepare Backends", command=start_prepare)
    place_button(prepare_btn, 0, 7)
    quick_summary_btn = tk.Button(controls, text="Quick Summary", command=show_quick_summary_window)
    place_button(quick_summary_btn, 0, 8)
    chart_btn = tk.Button(controls, text="Runtime Chart", command=lambda: show_runtime_chart())
    place_button(chart_btn, 0, 9)
    help_btn = tk.Button(controls, text="Help", command=show_help)
    place_button(help_btn, 0, 10)
    cancel_btn = tk.Button(controls, text="Cancel", command=cancel, state=tk.DISABLED)
    place_button(cancel_btn, 0, 11, padx=(0, 0))

    coverage_auto_btn = tk.Button(controls, text=coverage_button_text("Auto Fallback"), command=lambda: start("auto", False, True))
    place_button(coverage_auto_btn, 1, 0)
    coverage_local_btn = tk.Button(
        controls,
        text=coverage_button_text("Local", strict_local_coverage_available),
        command=lambda: start("local", False, True),
        state=tk.NORMAL if strict_local_coverage_available else tk.DISABLED,
    )
    place_button(coverage_local_btn, 1, 1)
    coverage_docker_btn = tk.Button(
        controls,
        text=coverage_button_text("Docker", strict_docker_available),
        command=lambda: start("docker", False, True),
        state=tk.NORMAL if strict_docker_available else tk.DISABLED,
    )
    place_button(coverage_docker_btn, 1, 2)
    coverage_podman_btn = tk.Button(
        controls,
        text=coverage_button_text("Podman", strict_podman_available),
        command=lambda: start("podman", False, True),
        state=tk.NORMAL if strict_podman_available else tk.DISABLED,
    )
    place_button(coverage_podman_btn, 1, 3)
    coverage_wsl_btn = tk.Button(
        controls,
        text=coverage_button_text("WSL", strict_wsl_coverage_available),
        command=lambda: start("wsl", False, True),
        state=tk.NORMAL if strict_wsl_coverage_available else tk.DISABLED,
    )
    place_button(coverage_wsl_btn, 1, 4)

    write("Choose Auto Fallback (Recommended) to discover Docker, Podman, WSL, then local CMake/CTest.\n")
    write("Backend-specific buttons are strict and do not fall back if that backend is unavailable.\n")
    if not strict_docker_available:
        write("Docker is disabled: " + container_engine_summary("docker", strict_docker_reason) + ".\n")
    if not strict_podman_available:
        write("Podman is disabled: " + container_engine_summary("podman", strict_podman_reason) + ".\n")
    if not strict_wsl_available:
        write("WSL is disabled: no non-internal WSL distro with cmake, ninja, gcc/g++, and clang/clang++ was detected.\n")
    if strict_wsl_available and not strict_wsl_coverage_available:
        write("WSL Coverage is disabled: no non-internal WSL distro with cmake, ninja, gcc/g++, and lcov was detected. Prepare Backends can install the missing WSL coverage tool.\n")
    if not strict_local_available:
        write("Local is disabled: " + strict_local_reason + "\n")
    if not strict_local_coverage_available:
        write("Local Coverage is disabled: " + strict_local_coverage_reason + "\n")
    write("Local avoids creating/running virtualized environments and must pass a tiny C/C++ sanity build first.\n")
    write("Recommend / Install prints safe installer commands. Run them from a terminal to confirm installation.\n")
    write("Prepare Backends asks first, then can try to make disabled Docker/Podman/WSL buttons usable, including WSL coverage tools.\n")
    write("Click Help for terminal commands and backend descriptions.\n")
    write("Click Quick Summary for a compact dashboard from the latest command, coverage, and runtime artifacts.\n")
    write("Use the Coverage row to run auto coverage or force Docker, Podman, WSL, or local coverage.\n")
    write("Guardrails are active: command timeout and no-output watchdog.\n")

    set_minimum_window_size(root, 920, 640)

    def poll() -> None:
        try:
            while True:
                item = output.get_nowait()
                if isinstance(item, tuple):
                    mode, code = item
                    set_running(False)
                    if mode == "coverage":
                        latest_coverage = newest_coverage_artifact()
                        if latest_coverage:
                            write(f"\nCoverage artifact: {latest_coverage}\n")
                            show_coverage_report(latest_coverage, announce_errors=False)
                            if code == 0:
                                messagebox.showinfo("SSTL Test Runner", "Coverage report generated and opened.")
                            else:
                                messagebox.showwarning("SSTL Test Runner", "Coverage report opened. One or more coverage thresholds did not pass.")
                        else:
                            messagebox.showerror("SSTL Test Runner", "Coverage command finished, but no coverage artifact was found.")
                        continue
                    if mode == "install":
                        if code == 0:
                            messagebox.showinfo("SSTL Test Runner", "Install recommendations printed. No commands were run.")
                        else:
                            messagebox.showerror("SSTL Test Runner", "Could not print install recommendations.")
                        continue
                    if mode == "prepare":
                        if code == 0:
                            messagebox.showinfo("SSTL Test Runner", "Backend preparation finished. Availability buttons were refreshed.")
                        else:
                            messagebox.showwarning("SSTL Test Runner", "Backend preparation finished with issues. See the log for the backend-specific reason.")
                        continue
                    if code == 0:
                        latest_csv = newest_runtime_csv_artifact()
                        if latest_csv:
                            write(f"\nRuntime CSV artifact: {latest_csv}\n")
                            show_runtime_chart(latest_csv, announce_errors=False)
                            messagebox.showinfo("SSTL Test Runner", "Tests completed successfully. Runtime chart opened from the newest CSV artifact.")
                        else:
                            messagebox.showinfo("SSTL Test Runner", "Tests completed successfully.")
                    else:
                        messagebox.showerror("SSTL Test Runner", "One or more test lanes failed or timed out.")
                else:
                    write(item)
        except queue.Empty:
            root.after(100, poll)

    root.after(100, poll)
    root.mainloop()
    return 0


def main() -> int:
    if maybe_relaunch_windows_gui(ROOT, __file__, sys.argv[1:]):
        return 0
    # Arguments always mean CLI. This avoids accidental GUI launch in IDEs,
    # build tools, or automation shells whose stdout is not a real terminal.
    if len(sys.argv) > 1 or (terminal_like_launch() and parent_is_known_terminal()):
        return run_cli(sys.argv[1:])
    return run_gui()


if __name__ == "__main__":
    raise SystemExit(main())
