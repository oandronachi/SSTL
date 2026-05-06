#!/usr/bin/env python3
"""Remove generated SSTL paths by cleanup group.

Terminal usage:
  python tools/sstl_clean_generated.py --dry-run
  python tools/sstl_clean_generated.py --groups cache build --dry-run
  python tools/sstl_clean_generated.py --groups documentation --dry-run
  python tools/sstl_clean_generated.py --groups artifacts --dry-run
  python tools/sstl_clean_generated.py --groups artifacts build --dry-run
  python tools/sstl_clean_generated.py --groups all --yes
  python tools/sstl_clean_generated.py --gui

Double-click usage:
  Double-click this file. The GUI opens with cleanup groups for cache, build,
  documentation outputs, and full artifacts output. Click Scan to review the
  exact targets, then click Delete Listed Targets when ready.

Cleanup groups:
  cache          folders named __pycache__ or .pytest_cache
  build          folders named build
  documentation  generated Doxygen folders under artifacts/ and documentation/
  artifacts      every immediate child under artifacts/; leaves artifacts/
  all            cache + build + documentation

Guardrails:
  - deprecated folders are not traversed
  - every target must resolve inside the project root
  - source roots such as include/, testing/, tools/ and documentation/ are
    never deleted directly; artifacts/ is kept and only its children are purged
  - documentation cleanup is restricted to generated Doxygen output folders
  - CLI deletion asks for confirmation unless --yes is supplied
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import os
from pathlib import Path
import queue
import shutil
import sys
import threading

from sstl_tool_common import (
    ask_yes_no,
    attach_text_copy_context_menu,
    maybe_relaunch_windows_gui,
    parent_is_known_terminal,
    repo_root_from_script,
    set_buttons_enabled,
    set_minimum_window_size,
    set_widget_children_enabled,
    terminal_like_launch,
)


ROOT = repo_root_from_script(__file__)
ARTIFACT_ROOT = ROOT / "artifacts"

CACHE_DIR_NAMES = {"__pycache__", ".pytest_cache"}
BUILD_DIR_NAMES = {"build"}
STANDARD_GROUPS = ("cache", "build", "documentation")
GROUP_ORDER = ("cache", "build", "documentation", "artifacts")
GROUP_ALIASES = {
    "all": STANDARD_GROUPS,
    "cache": ("cache",),
    "caches": ("cache",),
    "pycache": ("cache",),
    "__pycache__": ("cache",),
    ".pytest_cache": ("cache",),
    "build": ("build",),
    "builds": ("build",),
    "documentation": ("documentation",),
    "docs": ("documentation",),
    "doc": ("documentation",),
    "doxygen": ("documentation",),
    "artifact": ("artifacts",),
    "artifacts": ("artifacts",),
    "artefact": ("artifacts",),
    "artefacts": ("artifacts",),
    "all-artifacts": ("artifacts",),
    "all_artifacts": ("artifacts",),
    "all-artefacts": ("artifacts",),
    "all_artefacts": ("artifacts",),
    "artifacts-build": ("artifacts", "build"),
    "artifacts+build": ("artifacts", "build"),
    "artifacts_build": ("artifacts", "build"),
    "artefacts-build": ("artifacts", "build"),
    "artefacts+build": ("artifacts", "build"),
    "artefacts_build": ("artifacts", "build"),
    "all-artifacts-build": ("artifacts", "build"),
    "all_artifacts_build": ("artifacts", "build"),
    "all-artefacts-build": ("artifacts", "build"),
    "all_artefacts_build": ("artifacts", "build"),
}
PROTECTED_ROOT_NAMES = {"include", "testing", "tools", "artifacts", "documentation"}

HELP_TEXT = """SSTL Generated Data Cleaner

Terminal examples:
  python tools/sstl_clean_generated.py --dry-run
  python tools/sstl_clean_generated.py --groups cache build --dry-run
  python tools/sstl_clean_generated.py --groups documentation --dry-run
  python tools/sstl_clean_generated.py --groups artifacts --dry-run
  python tools/sstl_clean_generated.py --groups artifacts build --dry-run
  python tools/sstl_clean_generated.py --groups all --yes

Double-click behavior:
  The GUI opens. Select cleanup groups, click Scan to list generated targets,
  then Delete Listed Targets when you are ready. A confirmation dialog is shown
  before deletion.

Groups:
  cache          __pycache__ and .pytest_cache folders
  build          build folders
  documentation  generated Doxygen folders under artifacts/ and documentation/
  artifacts      every immediate child under artifacts/; leaves artifacts/
  all            cache + build + documentation
  artifacts-build shortcut for artifacts + build

The cleaner does not traverse deprecated folders and refuses to delete source
roots directly. The artifacts group purges the contents of artifacts/ without
removing the artifacts/ directory itself.
"""


@dataclass(frozen=True)
class CleanTarget:
    """One generated path that can be safely removed."""

    group: str
    path: Path


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


def is_artifact_child(path: Path) -> bool:
    try:
        rel = path.resolve().relative_to(ARTIFACT_ROOT.resolve())
    except ValueError:
        return False
    return len(rel.parts) > 0


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
    if target.group == "artifacts":
        return is_artifact_child(path)
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
        for group in mapped:
            if group not in normalized:
                normalized.append(group)
    if unknown:
        raise ValueError(
            "Unknown cleanup group(s): "
            + ", ".join(unknown)
            + "\nAvailable groups: all, cache, build, documentation, artifacts"
        )
    return normalized or list(STANDARD_GROUPS)


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


def discover_artifact_targets(groups: set[str]) -> list[CleanTarget]:
    if "artifacts" not in groups or not ARTIFACT_ROOT.exists():
        return []
    targets: list[CleanTarget] = []
    for path in sorted(ARTIFACT_ROOT.iterdir(), key=lambda item: item.name.lower()):
        target = CleanTarget("artifacts", path)
        if target_allowed(target):
            targets.append(target)
    return targets


def discover_targets(groups: list[str] | None = None) -> list[CleanTarget]:
    selected = set(groups or STANDARD_GROUPS)
    targets = discover_walk_targets(selected) + discover_documentation_targets(selected) + discover_artifact_targets(selected)
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
        "artifacts": "Artifacts",
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
            if target.path.is_dir() and not target.path.is_symlink():
                shutil.rmtree(target.path)
            else:
                target.path.unlink()
            log(f"Deleted [{target.group}]: {display_path(target.path)}")
        except Exception as exc:
            log(f"FAILED [{target.group}]: {display_path(target.path)} - {exc}")
            ok = False
    return ok


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Remove SSTL generated paths by cleanup group.")
    parser.add_argument("--dry-run", action="store_true", help="List targets without deleting them.")
    parser.add_argument("--yes", action="store_true", help="Delete without asking for CLI confirmation.")
    parser.add_argument("--gui", action="store_true", help="Open the Tkinter GUI.")
    parser.add_argument("--list-groups", action="store_true", help="List available cleanup groups and exit.")
    parser.add_argument(
        "--groups",
        nargs="+",
        default=["all"],
        help="Cleanup groups to scan/delete: all, cache, build, documentation, artifacts. Default: all.",
    )
    return parser.parse_args(argv)


def print_group_help() -> None:
    print("Available cleanup groups:")
    print("  all            cache + build + documentation")
    print("  cache          __pycache__ and .pytest_cache folders")
    print("  build          build folders")
    print("  documentation  generated Doxygen folders under artifacts/ and documentation/")
    print("  artifacts      every immediate child under artifacts/; leaves artifacts/")
    print("  artifacts-build  artifacts + build shortcut")


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
    if not args.yes and not ask_yes_no(f"\nDelete {len(targets)} generated target(s)?", default=False):
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
    target_groups: list[str] = []

    top = tk.Frame(root)
    top.pack(fill=tk.X, padx=8, pady=(8, 0))
    tk.Label(top, text="Cleanup groups", anchor="w", font=("Segoe UI", 10, "bold")).pack(anchor="w")

    group_vars: dict[str, "tk.BooleanVar"] = {}
    checks = tk.Frame(top)
    checks.pack(fill=tk.X, pady=(4, 0))
    for group in GROUP_ORDER:
        var = tk.BooleanVar(value=group in STANDARD_GROUPS)
        group_vars[group] = var
        tk.Checkbutton(checks, text=group_title(group), variable=var).pack(side=tk.LEFT, padx=(0, 12))

    text = scrolledtext.ScrolledText(root, wrap=tk.WORD)
    attach_text_copy_context_menu(text)
    text.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

    controls = tk.Frame(root)
    controls.pack(fill=tk.X, padx=8, pady=(0, 8))

    buttons: list["tk.Button"] = []

    def write(message: str) -> None:
        text.insert(tk.END, message)
        text.see(tk.END)

    def set_running(running: bool) -> None:
        set_buttons_enabled(buttons, not running)
        set_widget_children_enabled(checks, not running)

    def selected_groups() -> list[str]:
        return [group for group in GROUP_ORDER if group_vars[group].get()]

    def scan_worker(groups: list[str]) -> None:
        output.put(("targets", (groups, discover_targets(groups))))

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
        nonlocal targets, target_groups
        groups = selected_groups()
        if not groups:
            messagebox.showinfo("SSTL Generated Data Cleaner", "Select at least one cleanup group.")
            return
        if groups != target_groups:
            targets = discover_targets(groups)
            target_groups = list(groups)
            text.delete("1.0", tk.END)
            write("Generated cleanup targets:\n")
            if not targets:
                write("  none\n")
            for line in format_targets(targets):
                write(line + "\n")
        if not targets:
            messagebox.showinfo("SSTL Generated Data Cleaner", "No generated targets are currently listed.")
            return
        actual_groups = sorted({target.group for target in targets}, key=GROUP_ORDER.index)
        if not messagebox.askyesno(
            "Confirm Cleanup",
            f"Delete {len(targets)} generated target(s)?\n\nGroups: {', '.join(actual_groups)}\n\nSource folders are protected.",
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
        nonlocal targets, target_groups
        try:
            while True:
                kind, payload = output.get_nowait()
                set_running(False)
                if kind == "targets":
                    groups, discovered = payload  # type: ignore[misc]
                    target_groups = list(groups)
                    targets = list(discovered)
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
                    target_groups = selected_groups()
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
    write("Artifacts cleanup removes every immediate child under artifacts/ but keeps the artifacts/ folder.\n")
    write("Click Delete Listed Targets when you are ready; a confirmation dialog appears first.\n")
    write("CLI is also available; click Help for examples.\n")
    set_minimum_window_size(root, 980, 620)
    root.after(100, poll)
    root.mainloop()
    return 0


def main(argv: list[str] | None = None) -> int:
    actual_argv = sys.argv[1:] if argv is None else argv
    if actual_argv:
        return run_cli(actual_argv)
    if maybe_relaunch_windows_gui(ROOT, __file__, actual_argv):
        return 0
    if terminal_like_launch() and parent_is_known_terminal():
        return run_cli([])
    return run_gui()


if __name__ == "__main__":
    raise SystemExit(main())
