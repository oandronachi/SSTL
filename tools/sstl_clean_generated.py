#!/usr/bin/env python3
"""Remove generated SSTL folders by cleanup group.

Terminal usage:
  python tools/sstl_clean_generated.py --dry-run
  python tools/sstl_clean_generated.py --groups cache build --dry-run
  python tools/sstl_clean_generated.py --groups documentation --dry-run
  python tools/sstl_clean_generated.py --groups all --yes
  python tools/sstl_clean_generated.py --gui

Double-click usage:
  Double-click this file. The GUI opens with cleanup groups for cache, build,
  and documentation outputs. Click Scan to review the exact targets, then click
  Delete Listed Targets when ready.

Cleanup groups:
  cache          folders named __pycache__ or .pytest_cache
  build          folders named build
  documentation  generated Doxygen folders under artifacts/ and documentation/
  all            every group above

Guardrails:
  - deprecated folders are not traversed
  - every target must resolve inside the project root
  - source roots such as include/, testing/, tools/, artifacts/, and
    documentation/ are never deleted directly
  - documentation cleanup is restricted to generated Doxygen output folders
  - CLI deletion asks for confirmation unless --yes is supplied
"""

from __future__ import annotations

import argparse
import ctypes
from dataclasses import dataclass
import os
from pathlib import Path
import queue
import shutil
import subprocess
import sys
import threading


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent if SCRIPT_DIR.name == "tools" else SCRIPT_DIR
ARTIFACT_ROOT = ROOT / "artifacts"

CACHE_DIR_NAMES = {"__pycache__", ".pytest_cache"}
BUILD_DIR_NAMES = {"build"}
GROUP_ORDER = ("cache", "build", "documentation")
GROUP_ALIASES = {
    "all": "all",
    "cache": "cache",
    "caches": "cache",
    "pycache": "cache",
    "__pycache__": "cache",
    ".pytest_cache": "cache",
    "build": "build",
    "builds": "build",
    "documentation": "documentation",
    "docs": "documentation",
    "doc": "documentation",
    "doxygen": "documentation",
}
PROTECTED_ROOT_NAMES = {"include", "testing", "tools", "artifacts", "documentation"}

HELP_TEXT = """SSTL Generated Data Cleaner

Terminal examples:
  python tools/sstl_clean_generated.py --dry-run
  python tools/sstl_clean_generated.py --groups cache build --dry-run
  python tools/sstl_clean_generated.py --groups documentation --dry-run
  python tools/sstl_clean_generated.py --groups all --yes

Double-click behavior:
  The GUI opens. Select cleanup groups, click Scan to list generated folders,
  then Delete Listed Targets when you are ready. A confirmation dialog is shown
  before deletion.

Groups:
  cache          __pycache__ and .pytest_cache folders
  build          build folders
  documentation  generated Doxygen folders under artifacts/ and documentation/
  all            every group above

The cleaner does not traverse deprecated folders and refuses to delete source
roots directly.
"""


@dataclass(frozen=True)
class CleanTarget:
    """One generated folder that can be safely removed."""

    group: str
    path: Path


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


def maybe_relaunch_windows_gui() -> bool:
    if os.name != "nt" or len(sys.argv) > 1:
        return False
    if Path(sys.executable).name.lower() == "pythonw.exe":
        return False
    if parent_is_known_terminal():
        return False
    pythonw = Path(sys.executable).with_name("pythonw.exe")
    if not pythonw.exists():
        return False
    try:
        subprocess.Popen([str(pythonw), str(Path(__file__).resolve())], cwd=str(ROOT), close_fds=True)
        return True
    except Exception:
        return False


def is_deprecated_part(path: Path) -> bool:
    return any("deprecated" in part.lower() for part in path.parts)


def inside_root(path: Path) -> bool:
    try:
        path.resolve().relative_to(ROOT.resolve())
        return True
    except ValueError:
        return False


def is_protected_direct_root(path: Path) -> bool:
    try:
        rel = path.resolve().relative_to(ROOT.resolve())
    except ValueError:
        return True
    return len(rel.parts) == 1 and rel.parts[0].lower() in PROTECTED_ROOT_NAMES


def generated_documentation_roots() -> set[Path]:
    return {
        (ARTIFACT_ROOT / "doxygen").resolve(),
        (ROOT / "documentation" / "doxygen").resolve(),
    }


def target_allowed(target: CleanTarget) -> bool:
    path = target.path
    if not inside_root(path):
        return False
    if is_deprecated_part(path):
        return False
    if is_protected_direct_root(path):
        return False
    if target.group == "cache":
        return path.name in CACHE_DIR_NAMES
    if target.group == "build":
        return path.name in BUILD_DIR_NAMES
    if target.group == "documentation":
        return path.resolve() in generated_documentation_roots()
    return False


def normalize_groups(raw_groups: list[str] | None) -> list[str]:
    requested = raw_groups or ["all"]
    normalized: list[str] = []
    unknown: list[str] = []
    for raw in requested:
        key = raw.strip().lower()
        mapped = GROUP_ALIASES.get(key)
        if not mapped:
            unknown.append(raw)
            continue
        if mapped == "all":
            normalized = list(GROUP_ORDER)
            break
        if mapped not in normalized:
            normalized.append(mapped)
    if unknown:
        raise ValueError(
            "Unknown cleanup group(s): "
            + ", ".join(unknown)
            + "\nAvailable groups: all, cache, build, documentation"
        )
    return normalized or list(GROUP_ORDER)


def discover_walk_targets(groups: set[str]) -> list[CleanTarget]:
    targets: list[CleanTarget] = []
    for dirpath, dirnames, _ in os.walk(ROOT):
        current = Path(dirpath)
        pruned: list[str] = []
        for dirname in dirnames:
            child = current / dirname
            lowered = dirname.lower()
            if "deprecated" in lowered:
                continue
            if "cache" in groups and dirname in CACHE_DIR_NAMES:
                target = CleanTarget("cache", child)
                if target_allowed(target):
                    targets.append(target)
                continue
            if "build" in groups and dirname in BUILD_DIR_NAMES:
                target = CleanTarget("build", child)
                if target_allowed(target):
                    targets.append(target)
                continue
            pruned.append(dirname)
        dirnames[:] = pruned
    return targets


def discover_documentation_targets(groups: set[str]) -> list[CleanTarget]:
    if "documentation" not in groups:
        return []
    targets: list[CleanTarget] = []
    for path in sorted(generated_documentation_roots(), key=lambda item: str(item).lower()):
        original = Path(path)
        target = CleanTarget("documentation", original)
        if original.exists() and target_allowed(target):
            targets.append(target)
    return targets


def discover_targets(groups: list[str] | None = None) -> list[CleanTarget]:
    selected = set(groups or GROUP_ORDER)
    targets = discover_walk_targets(selected) + discover_documentation_targets(selected)
    unique: dict[Path, CleanTarget] = {}
    for target in targets:
        unique[target.path.resolve()] = target
    return sorted(unique.values(), key=lambda item: (GROUP_ORDER.index(item.group), display_path(item.path).lower()))


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT.resolve())).replace("\\", "/")
    except ValueError:
        return str(path)


def group_title(group: str) -> str:
    return {
        "cache": "Cache",
        "build": "Build",
        "documentation": "Documentation",
    }.get(group, group)


def grouped_targets(targets: list[CleanTarget]) -> dict[str, list[CleanTarget]]:
    grouped: dict[str, list[CleanTarget]] = {group: [] for group in GROUP_ORDER}
    for target in targets:
        grouped.setdefault(target.group, []).append(target)
    return grouped


def format_targets(targets: list[CleanTarget]) -> list[str]:
    lines: list[str] = []
    grouped = grouped_targets(targets)
    for group in GROUP_ORDER:
        items = grouped.get(group, [])
        if not items:
            continue
        lines.append(f"{group_title(group)}:")
        for item in items:
            lines.append("  " + display_path(item.path))
    return lines


def delete_targets(targets: list[CleanTarget], log=print) -> bool:
    ok = True
    for target in targets:
        if not target_allowed(target):
            log(f"SKIP unsafe target: {target.group}: {target.path}")
            ok = False
            continue
        if not target.path.exists():
            log(f"SKIP missing: {display_path(target.path)}")
            continue
        try:
            shutil.rmtree(target.path)
            log(f"Deleted [{target.group}]: {display_path(target.path)}")
        except Exception as exc:
            log(f"FAILED [{target.group}]: {display_path(target.path)} - {exc}")
            ok = False
    return ok


def ask_yes_no(prompt: str, default: bool = False) -> bool:
    suffix = " [Y/n] " if default else " [y/N] "
    try:
        answer = input(prompt + suffix).strip().lower()
    except EOFError:
        return False
    if not answer:
        return default
    return answer in {"y", "yes"}


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Remove SSTL generated folders by cleanup group.")
    parser.add_argument("--dry-run", action="store_true", help="List targets without deleting them.")
    parser.add_argument("--yes", action="store_true", help="Delete without asking for CLI confirmation.")
    parser.add_argument("--gui", action="store_true", help="Open the Tkinter GUI.")
    parser.add_argument("--list-groups", action="store_true", help="List available cleanup groups and exit.")
    parser.add_argument(
        "--groups",
        nargs="+",
        default=["all"],
        help="Cleanup groups to scan/delete: all, cache, build, documentation. Default: all.",
    )
    return parser.parse_args(argv)


def print_group_help() -> None:
    print("Available cleanup groups:")
    print("  all            cache + build + documentation")
    print("  cache          __pycache__ and .pytest_cache folders")
    print("  build          build folders")
    print("  documentation  generated Doxygen folders under artifacts/ and documentation/")


def run_cli(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.gui:
        return run_gui()
    if args.list_groups:
        print_group_help()
        return 0
    try:
        groups = normalize_groups(args.groups)
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 2

    targets = discover_targets(groups)
    print("SSTL generated cleanup targets:")
    print("Groups: " + ", ".join(groups))
    if not targets:
        print("  none")
        return 0
    for line in format_targets(targets):
        print(line)

    if args.dry_run:
        print("\nDry run only; nothing was deleted.")
        return 0
    if not args.yes and not ask_yes_no(f"\nDelete {len(targets)} generated folder(s)?", default=False):
        print("Cleanup declined; nothing was deleted.")
        return 0
    return 0 if delete_targets(targets) else 1


def run_gui() -> int:
    try:
        import tkinter as tk
        from tkinter import messagebox, scrolledtext
    except Exception:
        return run_cli(["--dry-run"])

    root = tk.Tk()
    root.title("SSTL Generated Data Cleaner")
    root.geometry("980x620")

    output: "queue.Queue[tuple[str, object]]" = queue.Queue()
    targets: list[CleanTarget] = []

    top = tk.Frame(root)
    top.pack(fill=tk.X, padx=8, pady=(8, 0))
    tk.Label(top, text="Cleanup groups", anchor="w", font=("Segoe UI", 10, "bold")).pack(anchor="w")

    group_vars: dict[str, "tk.BooleanVar"] = {}
    checks = tk.Frame(top)
    checks.pack(fill=tk.X, pady=(4, 0))
    for group in GROUP_ORDER:
        var = tk.BooleanVar(value=True)
        group_vars[group] = var
        tk.Checkbutton(checks, text=group_title(group), variable=var).pack(side=tk.LEFT, padx=(0, 12))

    text = scrolledtext.ScrolledText(root, wrap=tk.WORD)
    text.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

    controls = tk.Frame(root)
    controls.pack(fill=tk.X, padx=8, pady=(0, 8))

    buttons: list["tk.Button"] = []

    def write(message: str) -> None:
        text.insert(tk.END, message)
        text.see(tk.END)

    def set_running(running: bool) -> None:
        for button in buttons:
            button.config(state=tk.DISABLED if running else tk.NORMAL)
        for child in checks.winfo_children():
            child.config(state=tk.DISABLED if running else tk.NORMAL)

    def selected_groups() -> list[str]:
        return [group for group in GROUP_ORDER if group_vars[group].get()]

    def scan_worker(groups: list[str]) -> None:
        output.put(("targets", discover_targets(groups)))

    def delete_worker(selected: list[CleanTarget]) -> None:
        lines: list[str] = []

        def collect(message: str) -> None:
            lines.append(message)

        ok = delete_targets(selected, collect)
        output.put(("deleted", (ok, lines)))

    def start_scan() -> None:
        groups = selected_groups()
        if not groups:
            messagebox.showinfo("SSTL Generated Data Cleaner", "Select at least one cleanup group.")
            return
        set_running(True)
        text.delete("1.0", tk.END)
        write("Scanning cleanup groups: " + ", ".join(groups) + "\n\n")
        threading.Thread(target=scan_worker, args=(groups,), daemon=True).start()

    def start_delete() -> None:
        if not targets:
            messagebox.showinfo("SSTL Generated Data Cleaner", "No generated folders are currently listed.")
            return
        groups = sorted({target.group for target in targets}, key=GROUP_ORDER.index)
        if not messagebox.askyesno(
            "Confirm Cleanup",
            f"Delete {len(targets)} generated folder(s)?\n\nGroups: {', '.join(groups)}\n\nSource folders are protected.",
        ):
            return
        set_running(True)
        threading.Thread(target=delete_worker, args=(list(targets),), daemon=True).start()

    def select_all() -> None:
        for var in group_vars.values():
            var.set(True)

    def clear_groups() -> None:
        for var in group_vars.values():
            var.set(False)

    def show_help() -> None:
        messagebox.showinfo("SSTL Generated Data Cleaner Help", HELP_TEXT)

    def poll() -> None:
        nonlocal targets
        try:
            while True:
                kind, payload = output.get_nowait()
                set_running(False)
                if kind == "targets":
                    targets = list(payload)  # type: ignore[arg-type]
                    text.delete("1.0", tk.END)
                    write("Generated cleanup targets:\n")
                    if not targets:
                        write("  none\n")
                    for line in format_targets(targets):
                        write(line + "\n")
                else:
                    ok, lines = payload  # type: ignore[misc]
                    text.delete("1.0", tk.END)
                    for line in lines:
                        write(line + "\n")
                    if ok:
                        messagebox.showinfo("SSTL Generated Data Cleaner", "Cleanup completed.")
                    else:
                        messagebox.showwarning("SSTL Generated Data Cleaner", "Cleanup completed with one or more skips/failures.")
                    targets = discover_targets(selected_groups())
        except queue.Empty:
            root.after(100, poll)

    for label, command in (
        ("Scan", start_scan),
        ("Delete Listed Targets", start_delete),
        ("Select All", select_all),
        ("Clear", clear_groups),
        ("Help", show_help),
    ):
        button = tk.Button(controls, text=label, command=command)
        button.pack(side=tk.LEFT, padx=(0, 8))
        buttons.append(button)

    write("Select cleanup groups, then click Scan to review generated targets.\n")
    write("Documentation cleanup removes generated Doxygen folders only.\n")
    write("Click Delete Listed Targets when you are ready; a confirmation dialog appears first.\n")
    write("CLI is also available; click Help for examples.\n")
    root.after(100, poll)
    root.mainloop()
    return 0


def main(argv: list[str] | None = None) -> int:
    actual_argv = sys.argv[1:] if argv is None else argv
    if actual_argv:
        return run_cli(actual_argv)
    if maybe_relaunch_windows_gui():
        return 0
    if terminal_like_launch() and parent_is_known_terminal():
        return run_cli([])
    return run_gui()


if __name__ == "__main__":
    raise SystemExit(main())
