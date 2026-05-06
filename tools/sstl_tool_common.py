#!/usr/bin/env python3
"""Shared helpers for SSTL command-line and Tkinter tools."""

from __future__ import annotations

import ctypes
from dataclasses import dataclass
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
from typing import Callable, Iterable, Sequence


INTERNAL_WSL_DISTRO_PREFIXES = (
    "docker-desktop",
    "podman-machine",
    "rancher-desktop",
)


@dataclass
class InstallOption:
    name: str
    backend: str
    commands: list[list[str]]
    notes: list[str]


def repo_root_from_script(script_file: str | Path) -> Path:
    script_dir = Path(script_file).resolve().parent
    return script_dir.parent if script_dir.name == "tools" else script_dir


def shell_quote(value: str) -> str:
    return "'" + value.replace("'", "'\"'\"'") + "'"


def display_command_arg_for_root(root: Path, arg: str) -> str:
    root_text = str(root)
    alt_root = root_text.replace("\\", "/")
    value = arg
    if value == root_text or value == alt_root:
        return "."
    if value.startswith(root_text + os.sep):
        return "." + value[len(root_text) :]
    if value.startswith(alt_root + "/"):
        return "." + value[len(alt_root) :]
    for sep in (":/work", ":/work:ro"):
        if value == root_text + sep or value == alt_root + sep:
            return "." + sep
    return value


def container_mount_arg_for_root(root: Path, mount_point: str = "/work", read_only: bool = False) -> str:
    # Docker/Podman need a concrete host bind source. Callers can redact this
    # through display_command_arg_for_root so logs stay checkout-name independent.
    suffix = ":ro" if read_only else ""
    return str(root) + ":" + mount_point + suffix


def ask_yes_no(prompt: str, default: bool = False) -> bool:
    suffix = " [Y/n] " if default else " [y/N] "
    try:
        answer = input(prompt + suffix).strip().lower()
    except EOFError:
        return False
    if not answer:
        return default
    return answer in ("y", "yes")


def terminal_like_launch() -> bool:
    return bool(sys.stdin and sys.stdin.isatty() and sys.stdout and sys.stdout.isatty())


def parent_process_name() -> str:
    if os.name != "nt":
        return ""
    try:
        parent_pid = os.getppid()
        process_query_limited_information = 0x1000
        kernel32 = ctypes.windll.kernel32
        handle = kernel32.OpenProcess(process_query_limited_information, False, parent_pid)
        if not handle:
            return ""
        try:
            size = ctypes.c_ulong(32768)
            buf = ctypes.create_unicode_buffer(size.value)
            if kernel32.QueryFullProcessImageNameW(handle, 0, buf, ctypes.byref(size)):
                return Path(buf.value).name.lower()
        finally:
            kernel32.CloseHandle(handle)
    except Exception:
        return ""
    return ""


def parent_is_known_terminal() -> bool:
    return parent_process_name() in {
        "cmd.exe",
        "powershell.exe",
        "pwsh.exe",
        "windowsterminal.exe",
        "wt.exe",
        "conhost.exe",
        "openconsole.exe",
        "terminal64.exe",
        "terminal.exe",
    }


def maybe_relaunch_windows_gui(root: Path, script_file: str | Path, argv: Sequence[str] | None = None) -> bool:
    if os.name != "nt" or (argv is not None and len(argv) > 0) or (argv is None and len(sys.argv) > 1):
        return False
    exe_name = Path(sys.executable).name.lower()
    if exe_name == "pythonw.exe":
        return False
    if parent_is_known_terminal():
        return False
    pythonw = Path(sys.executable).with_name("pythonw.exe")
    if not pythonw.exists():
        return False
    try:
        script_path = Path(script_file).resolve()
        subprocess.Popen([str(pythonw), str(script_path)], cwd=str(root), close_fds=True)
        return True
    except Exception:
        return False


def set_minimum_window_size(window: object, min_width: int, min_height: int, extra_width: int = 16) -> tuple[int, int]:
    update = getattr(window, "update_idletasks")
    update()
    req_width = int(getattr(window, "winfo_reqwidth")())
    req_height = int(getattr(window, "winfo_reqheight")())
    width = max(req_width + extra_width, min_width)
    height = max(req_height, min_height)
    getattr(window, "minsize")(width, height)
    getattr(window, "geometry")(f"{width}x{height}")
    return width, height


def set_buttons_enabled(buttons: Iterable[object], enabled: bool) -> None:
    state = "normal" if enabled else "disabled"
    for button in buttons:
        getattr(button, "config")(state=state)


def set_widget_children_enabled(parent: object, enabled: bool) -> None:
    state = "normal" if enabled else "disabled"
    for child in getattr(parent, "winfo_children")():
        try:
            getattr(child, "config")(state=state)
        except Exception:
            pass


def attach_text_copy_context_menu(widget: object) -> None:
    """Attach a right-click Copy menu to a Tk Text-compatible widget."""

    import tkinter as tk

    menu = tk.Menu(widget, tearoff=0)

    def selected_text() -> str:
        try:
            return str(getattr(widget, "get")("sel.first", "sel.last"))
        except Exception:
            return ""

    def copy_selection() -> None:
        text = selected_text()
        if not text:
            return
        getattr(widget, "clipboard_clear")()
        getattr(widget, "clipboard_append")(text)

    menu.add_command(label="Copy", command=copy_selection)

    def show_menu(event: object) -> str:
        state = tk.NORMAL if selected_text() else tk.DISABLED
        menu.entryconfigure("Copy", state=state)
        try:
            menu.tk_popup(int(getattr(event, "x_root")), int(getattr(event, "y_root")))
        finally:
            menu.grab_release()
        return "break"

    getattr(widget, "bind")("<Button-3>", show_menu, add="+")
    getattr(widget, "bind")("<Control-Button-1>", show_menu, add="+")


def availability_label(label: str, available: bool) -> str:
    return label if available else label + " (Unavailable)"


def find_on_path(name: str) -> str | None:
    return shutil.which(name)


def windows_program_roots() -> list[Path]:
    roots: list[Path] = []
    for key in ("ProgramFiles", "ProgramFiles(x86)", "ProgramW6432", "LocalAppData"):
        value = os.environ.get(key)
        if value:
            path = Path(value)
            if path.exists() and path not in roots:
                roots.append(path)
    return roots


def find_named_executable(name: str, extra_roots: list[Path] | None = None) -> str | None:
    direct = find_on_path(name)
    if direct:
        return direct
    executable_name = name + ".exe" if os.name == "nt" and not name.lower().endswith(".exe") else name
    roots = extra_roots or []
    for root in roots:
        if not root.exists():
            continue
        likely = [
            root / executable_name,
            root / "bin" / executable_name,
            root / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "CMake" / "bin" / executable_name,
        ]
        for candidate in likely:
            if candidate.exists():
                return str(candidate)
        try:
            for candidate in root.rglob(executable_name):
                if candidate.is_file():
                    return str(candidate)
        except Exception:
            continue
    return None


def find_container_engine_executable(engine: str) -> str | None:
    direct = find_named_executable(engine)
    if direct:
        return direct
    if os.name != "nt":
        return None
    executable_name = engine + ".exe" if not engine.lower().endswith(".exe") else engine
    candidates: list[Path] = []
    for root in windows_program_roots():
        if engine == "docker":
            candidates.append(root / "Docker" / "Docker" / "resources" / "bin" / executable_name)
        elif engine == "podman":
            candidates.append(root / "RedHat" / "Podman" / executable_name)
            candidates.append(root / "Programs" / "RedHat" / "Podman" / executable_name)
            candidates.append(root / "Programs" / "Podman" / executable_name)
    for candidate in candidates:
        if candidate.exists():
            return str(candidate)
    return None


def find_docker_desktop_executable() -> str | None:
    if os.name != "nt":
        return None
    for root in windows_program_roots():
        candidate = root / "Docker" / "Docker" / "Docker Desktop.exe"
        if candidate.exists():
            return str(candidate)
    return None


def container_engine_probe(engine: str, timeout: int = 20) -> tuple[bool, str, str]:
    resolved = find_container_engine_executable(engine)
    if not resolved:
        return False, "", f"{engine}.exe was not found" if os.name == "nt" else f"{engine} executable was not found"
    try:
        proc = subprocess.run(
            [resolved, "info"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return False, resolved, f"{engine} engine status timed out"
    except Exception as exc:
        return False, resolved, f"{engine} engine status could not be checked: {exc}"
    if proc.returncode == 0:
        return True, resolved, f"{engine} engine is reachable"
    detail = " ".join(line.strip() for line in proc.stdout.splitlines() if line.strip())
    return False, resolved, detail or f"{engine} info exited with {proc.returncode}"


def container_engine_summary(engine: str, detail: str) -> str:
    lowered = detail.lower()
    if "was not found" in lowered:
        return f"{engine} executable was not found"
    if "cannot connect" in lowered or "unable to connect" in lowered or "socket" in lowered:
        if engine == "podman":
            return "Podman is installed, but its machine/socket is not reachable"
        if engine == "docker":
            return "Docker is installed, but its daemon/Desktop is not reachable"
    if "timed out" in lowered:
        return f"{engine} engine status timed out"
    return f"{engine} engine is not reachable"


def wait_for_container_engine(
    engine: str,
    timeout: int,
    log: Callable[[str], None] | None = None,
    probe_timeout: int = 10,
) -> tuple[bool, str]:
    deadline = time.monotonic() + timeout
    last_detail = ""
    while time.monotonic() < deadline:
        ok, _, detail = container_engine_probe(engine, timeout=probe_timeout)
        if ok:
            if log:
                log(f"{engine} engine is reachable.")
            return True, detail
        last_detail = detail
        time.sleep(3.0)
    if log:
        log(f"{engine} engine did not become reachable: {last_detail}")
    return False, last_detail or "timed out"


def podman_machine_exists(output: str) -> bool:
    for line in output.splitlines():
        stripped = line.strip()
        if not stripped or stripped.lower().startswith("name"):
            continue
        return True
    return False


def decode_wsl_output(data: bytes) -> str:
    if b"\x00" in data:
        text = data.decode("utf-16le", errors="ignore")
    else:
        text = data.decode("utf-8", errors="replace")
    return text.replace("\x00", "")


def wsl_command(wsl: str, distro: str | None, *args: str) -> list[str]:
    cmd = [wsl]
    if distro:
        cmd.extend(["-d", distro])
    rest = list(args)
    while rest and rest[0] in ("-u", "--user"):
        if len(rest) < 2:
            break
        cmd.extend([rest.pop(0), rest.pop(0)])
    if rest:
        cmd.append("--exec")
        cmd.extend(rest)
    return cmd


def wsl_distro_is_internal(name: str, prefixes: Sequence[str] = INTERNAL_WSL_DISTRO_PREFIXES) -> bool:
    lowered = name.strip().lower()
    return any(lowered.startswith(prefix) for prefix in prefixes)


def wsl_installed_distros(wsl: str) -> list[str]:
    try:
        proc = subprocess.run(
            [wsl, "--list", "--quiet"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            timeout=20,
        )
    except Exception:
        return []
    if proc.returncode != 0:
        return []
    names: list[str] = []
    for line in decode_wsl_output(proc.stdout).splitlines():
        name = line.strip().replace("\r", "")
        if name and name not in names:
            names.append(name)
    return names


def wsl_default_distro(wsl: str) -> str:
    try:
        proc = subprocess.run(
            [wsl, "--status"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            timeout=20,
        )
    except Exception:
        return ""
    if proc.returncode != 0:
        return ""
    for line in decode_wsl_output(proc.stdout).splitlines():
        if ":" not in line:
            continue
        key, value = line.split(":", 1)
        if key.strip().lower() == "default distribution":
            return value.strip()
    return ""


def wsl_candidate_distros(wsl: str, configured: str = "") -> list[tuple[str, str | None]]:
    candidates: list[tuple[str, str | None]] = []
    if configured:
        return [(configured, configured)]
    default_distro = wsl_default_distro(wsl)
    if default_distro and not wsl_distro_is_internal(default_distro):
        candidates.append((default_distro, None))
    elif not default_distro:
        candidates.append(("default", None))
    for distro in wsl_installed_distros(wsl):
        if not wsl_distro_is_internal(distro):
            candidates.append((distro, distro))

    unique: list[tuple[str, str | None]] = []
    seen: set[str] = set()
    for label, distro in candidates:
        key = distro or "<default>"
        if key not in seen:
            unique.append((label, distro))
            seen.add(key)
    return unique
