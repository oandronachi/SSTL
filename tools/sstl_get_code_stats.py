#!/usr/bin/env python3
"""Report SSTL line and character statistics with comment/whitespace splits."""

from __future__ import annotations

import argparse
import ast
import csv
import io
import json
import os
import queue
import sys
import threading
import tokenize
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

from sstl_tool_common import (
    attach_text_copy_context_menu,
    maybe_relaunch_windows_gui,
    parent_is_known_terminal,
    repo_root_from_script,
    set_buttons_enabled,
    set_minimum_window_size,
    terminal_like_launch,
)


CPP_EXTS = {".h", ".hpp", ".c", ".cpp", ".cc", ".cxx"}
PY_EXTS = {".py"}
HASH_COMMENT_EXTS = {".cmake", ".yaml", ".yml", ".toml", ".ini"}
HASH_COMMENT_NAMES = {"CMakeLists.txt"}
HTML_COMMENT_EXTS = {".md", ".html", ".htm"}
TEST_TEXT_EXTS = {".md", ".yaml", ".yml", ".json", ".toml", ".ini"}
EXCLUDED_DIRS = {"build", "__pycache__", ".pytest_cache", "artifacts"}
EXCLUDED_BUNDLE_NAMES = {
    "sstl-bundle-impl.yaml",
    "sstl-bundle-tests.yaml",
    "sstl-bundle-impl-no-comments.yaml",
    "sstl-bundle-tests-no-comments.yaml",
}
DEFAULT_ROOT = repo_root_from_script(__file__)
ARTIFACT_ROOT = DEFAULT_ROOT / "artifacts"
DEFAULT_CSV = ARTIFACT_ROOT / "sstl-code-stats.csv"
DEFAULT_JSON = ARTIFACT_ROOT / "sstl-code-stats.json"

HELP_TEXT = """SSTL Code Stats

Terminal usage:
  python tools/sstl_get_code_stats.py
  python tools/sstl_get_code_stats.py --groups implementation_include_headers all_test_code_assets
  python tools/sstl_get_code_stats.py --format csv
  python tools/sstl_get_code_stats.py --format json

Double-click usage:
  Double-click this file. The GUI opens without requiring terminal steps. Select
  one or more groups, click Run Stats, then optionally export the same report to
  CSV or JSON. Click Graphic View to open a horizontal bar chart for the latest
  stats run.

Reported fields:
  - with comments: total lines/chars before comment stripping
  - without comments: total lines/chars after comment stripping
  - chars_with_ws: all characters, including spaces/newlines/tabs
  - chars_no_ws: non-whitespace characters only
  - whitespace_chars: whitespace character count
  - whitespace-only lines: blank or whitespace-only physical lines
  - comments only: lines/chars that belong to comments or Python docstrings
  - consolidated view: a compact table with line totals, total chars, non-ws
    chars, and whitespace chars first

Default exclusions:
  Deprecated folders, generated build outputs, caches, and the full artifacts/
  tree are excluded from the source-code accounting.

Generated artifacts:
  artifacts/sstl-code-stats.csv
  artifacts/sstl-code-stats.json
"""

DISPLAY_NAMES = {
    "implementation_include_headers": "Implementation code: include/sstl/**",
    "test_c_cpp_sources_and_support": "Test C/C++ sources + support",
    "test_python_scripts": "Test Python scripts",
    "test_cmake_config": "Test CMake config",
    "test_docs_and_manifests": "Test docs and manifests",
    "all_test_code_assets": "All test code assets",
    "all_test_package_assets": "All test package assets",
    "top_level_python_tooling": "Helper Python tooling: tools/**",
    "implementation_plus_all_test_code_assets": "Implementation + all test code assets",
    "implementation_plus_all_test_package_assets": "Implementation + all test package assets",
    "implementation_plus_test_code_plus_top_level_tooling": "Implementation + test code + top-level tooling",
}

CONSOLIDATED_DEFAULT_GROUPS = {
    "implementation_include_headers",
    "test_c_cpp_sources_and_support",
    "test_python_scripts",
    "test_cmake_config",
    "all_test_code_assets",
    "implementation_plus_all_test_code_assets",
}


@dataclass
class Stats:
    group: str
    files: int = 0
    lines_with_comments: int = 0
    nonblank_lines_with_comments: int = 0
    chars_with_comments_with_ws: int = 0
    chars_with_comments_no_ws: int = 0
    whitespace_chars_with_comments: int = 0
    lines_without_comments: int = 0
    nonblank_lines_without_comments: int = 0
    chars_without_comments_with_ws: int = 0
    chars_without_comments_no_ws: int = 0
    whitespace_chars_without_comments: int = 0
    whitespace_only_lines: int = 0
    comment_lines: int = 0
    comment_only_lines: int = 0
    comment_chars_with_ws: int = 0
    comment_chars_no_ws: int = 0
    comment_whitespace_chars: int = 0

    def add(self, other: "Stats") -> None:
        for key, value in asdict(other).items():
            if key != "group":
                setattr(self, key, getattr(self, key) + value)


def normalize_text(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")


def line_count(text: str) -> int:
    if not text:
        return 0
    return text.count("\n") + (0 if text.endswith("\n") else 1)


def nonblank_line_count(text: str) -> int:
    return sum(1 for line in text.splitlines() if line.strip())


def whitespace_only_line_count(text: str) -> int:
    return sum(1 for line in text.splitlines() if not line.strip())


def mark_span(mask: list[bool], start: int, end: int) -> None:
    start = max(0, start)
    end = min(len(mask), end)
    for i in range(start, end):
        mask[i] = True


def offset_table(text: str) -> list[int]:
    offsets = [0]
    for index, ch in enumerate(text):
        if ch == "\n":
            offsets.append(index + 1)
    return offsets


def offset_from_position(offsets: list[int], line: int, col: int) -> int:
    if line <= 0:
        return col
    if line > len(offsets):
        return len(offsets)
    return offsets[line - 1] + col


def c_like_comment_mask(text: str) -> list[bool]:
    mask = [False] * len(text)
    i = 0
    n = len(text)
    state = "normal"
    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if state == "normal":
            if ch == "/" and nxt == "/":
                start = i
                i += 2
                while i < n and text[i] != "\n":
                    i += 1
                mark_span(mask, start, i)
                continue
            if ch == "/" and nxt == "*":
                start = i
                i += 2
                while i < n:
                    if i + 1 < n and text[i] == "*" and text[i + 1] == "/":
                        i += 2
                        break
                    i += 1
                mark_span(mask, start, i)
                continue
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            i += 1
        elif state == "string":
            if ch == "\\" and i + 1 < n:
                i += 2
                continue
            if ch == '"':
                state = "normal"
            i += 1
        else:
            if ch == "\\" and i + 1 < n:
                i += 2
                continue
            if ch == "'":
                state = "normal"
            i += 1
    return mask


def python_docstring_lines(text: str) -> set[int]:
    lines: set[int] = set()
    try:
        tree = ast.parse(text)
    except SyntaxError:
        return lines
    for node in ast.walk(tree):
        body = getattr(node, "body", None)
        if not body or not isinstance(body, list):
            continue
        first = body[0]
        value = getattr(first, "value", None)
        if isinstance(first, ast.Expr) and isinstance(value, ast.Constant) and isinstance(value.value, str):
            start = getattr(first, "lineno", None)
            end = getattr(first, "end_lineno", start)
            if start is not None and end is not None:
                lines.update(range(start, end + 1))
    return lines


def python_comment_mask(text: str) -> list[bool]:
    mask = [False] * len(text)
    offsets = offset_table(text)
    doc_lines = python_docstring_lines(text)
    try:
        for tok in tokenize.generate_tokens(io.StringIO(text).readline):
            tok_type, _, start, end, _ = tok
            if tok_type == tokenize.COMMENT or (tok_type == tokenize.STRING and start[0] in doc_lines):
                start_offset = offset_from_position(offsets, start[0], start[1])
                end_offset = offset_from_position(offsets, end[0], end[1])
                mark_span(mask, start_offset, end_offset)
    except tokenize.TokenError:
        for line_start in offsets:
            newline = text.find("\n", line_start)
            line_end = len(text) if newline == -1 else newline
            hash_index = text.find("#", line_start, line_end)
            if hash_index != -1:
                mark_span(mask, hash_index, line_end)
    return mask


def hash_comment_mask(text: str) -> list[bool]:
    mask = [False] * len(text)
    line_start = 0
    while line_start < len(text):
        newline = text.find("\n", line_start)
        line_end = len(text) if newline == -1 else newline
        state = "normal"
        i = line_start
        while i < line_end:
            ch = text[i]
            if state == "normal":
                if ch == "#":
                    mark_span(mask, i, line_end)
                    break
                if ch == '"':
                    state = "double"
                elif ch == "'":
                    state = "single"
            elif state == "double":
                if ch == "\\":
                    i += 1
                elif ch == '"':
                    state = "normal"
            elif state == "single":
                if ch == "\\":
                    i += 1
                elif ch == "'":
                    state = "normal"
            i += 1
        if newline == -1:
            break
        line_start = newline + 1
    return mask


def html_comment_mask(text: str) -> list[bool]:
    mask = [False] * len(text)
    cursor = 0
    while True:
        start = text.find("<!--", cursor)
        if start == -1:
            break
        end = text.find("-->", start + 4)
        stop = len(text) if end == -1 else end + 3
        mark_span(mask, start, stop)
        cursor = stop
    return mask


def comment_mask(path: Path, text: str) -> list[bool]:
    suffix = path.suffix.lower()
    if suffix in CPP_EXTS:
        return c_like_comment_mask(text)
    if suffix in PY_EXTS:
        return python_comment_mask(text)
    if suffix in HASH_COMMENT_EXTS or path.name in HASH_COMMENT_NAMES:
        return hash_comment_mask(text)
    if suffix in HTML_COMMENT_EXTS:
        return html_comment_mask(text)
    return [False] * len(text)


def without_comments(text: str, mask: list[bool]) -> str:
    chars: list[str] = []
    for ch, is_comment in zip(text, mask):
        if is_comment:
            if ch == "\n":
                chars.append(ch)
            continue
        chars.append(ch)
    return "".join(chars)


def masked_text(text: str, mask: list[bool]) -> str:
    return "".join(ch for ch, is_comment in zip(text, mask) if is_comment)


def comment_line_counts(text: str, mask: list[bool]) -> tuple[int, int]:
    comment_lines = 0
    comment_only_lines = 0
    offset = 0
    for line in text.splitlines(True):
        line_len = len(line)
        line_mask = mask[offset : offset + line_len]
        if any(line_mask):
            comment_lines += 1
            non_comment = "".join(ch for ch, is_comment in zip(line, line_mask) if not is_comment)
            if not non_comment.strip():
                comment_only_lines += 1
        offset += line_len
    return comment_lines, comment_only_lines


def file_stats(path: Path, group: str) -> Stats:
    text = normalize_text(path.read_text(encoding="utf-8", errors="replace"))
    mask = comment_mask(path, text)
    no_comments = without_comments(text, mask)
    comments = masked_text(text, mask)
    comment_lines, comment_only_lines = comment_line_counts(text, mask)

    return Stats(
        group=group,
        files=1,
        lines_with_comments=line_count(text),
        nonblank_lines_with_comments=nonblank_line_count(text),
        chars_with_comments_with_ws=len(text),
        chars_with_comments_no_ws=sum(1 for ch in text if not ch.isspace()),
        whitespace_chars_with_comments=sum(1 for ch in text if ch.isspace()),
        lines_without_comments=line_count(no_comments),
        nonblank_lines_without_comments=nonblank_line_count(no_comments),
        chars_without_comments_with_ws=len(no_comments),
        chars_without_comments_no_ws=sum(1 for ch in no_comments if not ch.isspace()),
        whitespace_chars_without_comments=sum(1 for ch in no_comments if ch.isspace()),
        whitespace_only_lines=whitespace_only_line_count(text),
        comment_lines=comment_lines,
        comment_only_lines=comment_only_lines,
        comment_chars_with_ws=len(comments),
        comment_chars_no_ws=sum(1 for ch in comments if not ch.isspace()),
        comment_whitespace_chars=sum(1 for ch in comments if ch.isspace()),
    )


def excluded(path: Path) -> bool:
    parts = {part.lower() for part in path.parts}
    if any("deprecated" in part for part in parts):
        return True
    if any(part in EXCLUDED_DIRS for part in parts):
        return True
    if "documentation" in parts and "doxygen" in parts:
        return True
    return path.name in EXCLUDED_BUNDLE_NAMES


def is_file_with_ext(path: Path, exts: set[str]) -> bool:
    return path.is_file() and not excluded(path) and path.suffix.lower() in exts


def is_cmake_file(path: Path) -> bool:
    return path.is_file() and not excluded(path) and path.name in HASH_COMMENT_NAMES


def collect_default_groups(root: Path) -> dict[str, list[Path]]:
    include_paths = [p for p in (root / "include").rglob("*") if is_file_with_ext(p, CPP_EXTS)]
    test_cpp_paths: list[Path] = []
    for subdir in (root / "testing" / "tests", root / "testing" / "test_support"):
        if subdir.exists():
            test_cpp_paths.extend(p for p in subdir.rglob("*") if is_file_with_ext(p, CPP_EXTS))
    test_python_paths = (
        [p for p in (root / "testing" / "scripts").glob("*.py") if is_file_with_ext(p, PY_EXTS)]
        if (root / "testing" / "scripts").exists()
        else []
    )
    test_cmake_paths = [root / "testing" / "CMakeLists.txt"] if is_cmake_file(root / "testing" / "CMakeLists.txt") else []
    test_docs_manifest_paths: list[Path] = []
    for subdir in (root / "testing" / "docs", root / "testing" / "manifests"):
        if subdir.exists():
            test_docs_manifest_paths.extend(p for p in subdir.rglob("*") if is_file_with_ext(p, TEST_TEXT_EXTS))
    top_level_tooling: list[Path] = []
    for subdir in (root, root / "tools"):
        if subdir.exists():
            top_level_tooling.extend(p for p in subdir.glob("*.py") if is_file_with_ext(p, PY_EXTS))

    test_code = test_cpp_paths + test_python_paths + test_cmake_paths
    test_package = test_code + test_docs_manifest_paths
    return {
        "implementation_include_headers": include_paths,
        "test_c_cpp_sources_and_support": test_cpp_paths,
        "test_python_scripts": test_python_paths,
        "test_cmake_config": test_cmake_paths,
        "test_docs_and_manifests": test_docs_manifest_paths,
        "all_test_code_assets": test_code,
        "all_test_package_assets": test_package,
        "top_level_python_tooling": top_level_tooling,
        "implementation_plus_all_test_code_assets": include_paths + test_code,
        "implementation_plus_all_test_package_assets": include_paths + test_package,
        "implementation_plus_test_code_plus_top_level_tooling": include_paths + test_code + top_level_tooling,
    }


def aggregate(group: str, paths: Iterable[Path]) -> Stats:
    total = Stats(group=group)
    for path in sorted(paths):
        total.add(file_stats(path, group))
    return total


def display_name(group: str) -> str:
    return DISPLAY_NAMES.get(group, group)


def fmt_int(value: int) -> str:
    return f"{value:,}"


def consolidated_table(stats: list[Stats]) -> str:
    headers = [
        "Category",
        "Files",
        "Lines incl. comments",
        "Chars incl. comments",
        "Non-ws chars incl. comments",
        "WS chars incl. comments",
        "Lines no comments",
        "Chars no comments",
        "Non-ws chars no comments",
        "WS chars no comments",
        "Comment-only lines",
        "Comment chars",
        "Comment chars no ws",
        "Comment ws chars",
    ]
    rows = [
        [
            display_name(item.group),
            fmt_int(item.files),
            fmt_int(item.nonblank_lines_with_comments),
            fmt_int(item.chars_with_comments_with_ws),
            fmt_int(item.chars_with_comments_no_ws),
            fmt_int(item.whitespace_chars_with_comments),
            fmt_int(item.nonblank_lines_without_comments),
            fmt_int(item.chars_without_comments_with_ws),
            fmt_int(item.chars_without_comments_no_ws),
            fmt_int(item.whitespace_chars_without_comments),
            fmt_int(item.comment_only_lines),
            fmt_int(item.comment_chars_with_ws),
            fmt_int(item.comment_chars_no_ws),
            fmt_int(item.comment_whitespace_chars),
        ]
        for item in stats
    ]
    widths = [len(header) for header in headers]
    for row in rows:
        for index, cell in enumerate(row):
            widths[index] = max(widths[index], len(cell))

    def render(row: list[str]) -> str:
        cells: list[str] = []
        for index, cell in enumerate(row):
            if index == 0:
                cells.append(cell.ljust(widths[index]))
            else:
                cells.append(cell.rjust(widths[index]))
        return "  ".join(cells)

    separator = "  ".join("-" * width for width in widths)
    lines = ["== Consolidated View", render(headers), separator]
    lines.extend(render(row) for row in rows)
    lines.append("")
    return "\n".join(lines)


def human_report(stats: list[Stats]) -> str:
    lines: list[str] = [consolidated_table(stats), "== Detailed View", ""]
    for item in stats:
        lines.append(display_name(item.group))
        lines.append(f"  group: {item.group}")
        lines.append(f"  files: {item.files}")
        lines.append(
            "  with comments: "
            f"lines={item.lines_with_comments}, nonblank_lines={item.nonblank_lines_with_comments}, "
            f"chars_with_ws={item.chars_with_comments_with_ws}, chars_no_ws={item.chars_with_comments_no_ws}, "
            f"whitespace_chars={item.whitespace_chars_with_comments}"
        )
        lines.append(
            "  without comments: "
            f"lines={item.lines_without_comments}, nonblank_lines={item.nonblank_lines_without_comments}, "
            f"chars_with_ws={item.chars_without_comments_with_ws}, chars_no_ws={item.chars_without_comments_no_ws}, "
            f"whitespace_chars={item.whitespace_chars_without_comments}"
        )
        lines.append(
            "  comments only: "
            f"lines={item.comment_lines}, comment_only_lines={item.comment_only_lines}, "
            f"chars_with_ws={item.comment_chars_with_ws}, chars_no_ws={item.comment_chars_no_ws}, "
            f"whitespace_chars={item.comment_whitespace_chars}"
        )
        lines.append(f"  whitespace-only lines: {item.whitespace_only_lines}")
        lines.append("")
    return "\n".join(lines)


def print_human(stats: list[Stats]) -> None:
    print(human_report(stats), end="")


def write_csv(path: Path, stats: list[Stats]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    rows = [asdict(item) for item in stats]
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()) if rows else list(asdict(Stats("")).keys()))
        writer.writeheader()
        writer.writerows(rows)


def write_json(path: Path, stats: list[Stats]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps([asdict(item) for item in stats], indent=2) + "\n", encoding="utf-8")


def compute_stats(root: Path, selected: list[str] | None = None) -> tuple[list[Stats], list[str]]:
    groups = collect_default_groups(root)
    names = selected or list(groups.keys())
    unknown = [name for name in names if name not in groups]
    if unknown:
        raise ValueError(
            "Unknown group(s): "
            + ", ".join(unknown)
            + "\nAvailable groups: "
            + ", ".join(groups)
        )
    return [aggregate(name, groups[name]) for name in names], list(groups.keys())


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Report SSTL line and character statistics.")
    parser.add_argument("--root", type=Path, default=DEFAULT_ROOT, help="Workspace root. Defaults to the parent of tools/.")
    parser.add_argument("--format", choices=("human", "json", "csv"), default="human", help="Output format.")
    parser.add_argument("--csv", type=Path, default=DEFAULT_CSV, help="CSV artifact path. Defaults to artifacts/sstl-code-stats.csv.")
    parser.add_argument("--json", type=Path, default=DEFAULT_JSON, help="JSON artifact path. Defaults to artifacts/sstl-code-stats.json.")
    parser.add_argument("--no-artifacts", action="store_true", help="Do not write the default CSV/JSON artifacts.")
    parser.add_argument("--gui", action="store_true", help="Open the Tkinter GUI.")
    parser.add_argument(
        "--groups",
        nargs="*",
        help="Optional group names to print. By default all built-in groups are printed.",
    )
    return parser.parse_args(argv)


def run_cli(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.gui:
        return run_gui()
    root = args.root.resolve()
    try:
        stats, _ = compute_stats(root, args.groups)
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 2

    if args.format == "human":
        print_human(stats)
    elif args.format == "json":
        print(json.dumps([asdict(item) for item in stats], indent=2))
    else:
        writer = csv.DictWriter(sys.stdout, fieldnames=list(asdict(Stats("")).keys()), lineterminator="\n")
        writer.writeheader()
        writer.writerows(asdict(item) for item in stats)

    if not args.no_artifacts and args.csv:
        write_csv(args.csv, stats)
    if not args.no_artifacts and args.json:
        write_json(args.json, stats)
    if not args.no_artifacts and args.format == "human":
        print(f"\nArtifacts written:\n  {args.csv}\n  {args.json}")
    return 0


def run_gui() -> int:
    try:
        import tkinter as tk
        from tkinter import filedialog, messagebox, scrolledtext
    except Exception:
        return run_cli(["--root", str(DEFAULT_ROOT)])

    ARTIFACT_ROOT.mkdir(parents=True, exist_ok=True)
    root = tk.Tk()
    root.title("SSTL Code Stats")
    root.geometry("1040x720")

    output: "queue.Queue[str | tuple[str, object]]" = queue.Queue()
    selected_stats: list[Stats] = []
    groups = collect_default_groups(DEFAULT_ROOT)

    left = tk.Frame(root)
    left.pack(side=tk.LEFT, fill=tk.Y, padx=8, pady=8)
    right = tk.Frame(root)
    right.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(0, 8), pady=8)

    tk.Label(left, text="Groups", anchor="w", font=("Segoe UI", 10, "bold")).pack(fill=tk.X)
    group_vars: dict[str, "tk.BooleanVar"] = {}
    default_selected = CONSOLIDATED_DEFAULT_GROUPS
    for name in groups:
        var = tk.BooleanVar(value=name in default_selected)
        group_vars[name] = var
        tk.Checkbutton(left, text=name, variable=var, anchor="w", justify=tk.LEFT, wraplength=280).pack(fill=tk.X, anchor="w")

    text = scrolledtext.ScrolledText(right, wrap=tk.WORD)
    attach_text_copy_context_menu(text)
    text.pack(fill=tk.BOTH, expand=True)

    controls = tk.Frame(right)
    controls.pack(fill=tk.X, pady=(8, 0))

    def write(message: str) -> None:
        text.insert(tk.END, message)
        text.see(tk.END)

    def selected_group_names() -> list[str]:
        return [name for name, var in group_vars.items() if var.get()]

    def set_running(running: bool) -> None:
        set_buttons_enabled(buttons, not running)

    def worker(names: list[str]) -> None:
        if not names:
            output.put(("error", "Select at least one group."))
            return
        try:
            stats, _ = compute_stats(DEFAULT_ROOT, names)
            output.put(("stats", stats))
        except Exception as exc:
            output.put(("error", str(exc)))

    def run_stats() -> None:
        names = selected_group_names()
        text.delete("1.0", tk.END)
        write("Calculating SSTL code statistics...\n\n")
        set_running(True)
        threading.Thread(target=worker, args=(names,), daemon=True).start()

    def select_all() -> None:
        for var in group_vars.values():
            var.set(True)

    def select_core() -> None:
        for name, var in group_vars.items():
            var.set(name in default_selected)

    def clear_groups() -> None:
        for var in group_vars.values():
            var.set(False)

    def save_csv() -> None:
        if not selected_stats:
            messagebox.showinfo("SSTL Code Stats", "Run stats before exporting CSV.")
            return
        path = filedialog.asksaveasfilename(
            title="Save CSV",
            initialdir=str(ARTIFACT_ROOT),
            initialfile="sstl-code-stats.csv",
            defaultextension=".csv",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        )
        if path:
            write_csv(Path(path), selected_stats)
            messagebox.showinfo("SSTL Code Stats", f"CSV written:\n{path}")

    def save_json() -> None:
        if not selected_stats:
            messagebox.showinfo("SSTL Code Stats", "Run stats before exporting JSON.")
            return
        path = filedialog.asksaveasfilename(
            title="Save JSON",
            initialdir=str(ARTIFACT_ROOT),
            initialfile="sstl-code-stats.json",
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
        )
        if path:
            write_json(Path(path), selected_stats)
            messagebox.showinfo("SSTL Code Stats", f"JSON written:\n{path}")

    def show_graphic_view() -> None:
        if not selected_stats:
            messagebox.showinfo("SSTL Code Stats", "Run stats before opening the graphic view.")
            return

        win = tk.Toplevel(root)
        win.title("SSTL Code Stats Graphic View")
        win.geometry("1220x760")

        header = tk.Label(
            win,
            text="Lines and non-whitespace characters by selected category",
            anchor="w",
            font=("Segoe UI", 10, "bold"),
        )
        header.pack(fill=tk.X, padx=10, pady=(8, 4))

        frame = tk.Frame(win)
        frame.pack(fill=tk.BOTH, expand=True)
        canvas = tk.Canvas(frame, bg="white")
        yscroll = tk.Scrollbar(frame, orient=tk.VERTICAL, command=canvas.yview)
        canvas.configure(yscrollcommand=yscroll.set)
        yscroll.pack(side=tk.RIGHT, fill=tk.Y)
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        colors = {
            "with": "#2f80ed",
            "without": "#219653",
            "comments": "#f2994a",
        }
        max_lines = max(
            max(item.nonblank_lines_with_comments, item.nonblank_lines_without_comments, item.comment_only_lines)
            for item in selected_stats
        ) or 1
        max_chars = max(
            max(
                item.chars_with_comments_with_ws,
                item.chars_with_comments_no_ws,
                item.whitespace_chars_with_comments,
                item.chars_without_comments_with_ws,
                item.chars_without_comments_no_ws,
                item.whitespace_chars_without_comments,
                item.comment_chars_with_ws,
                item.comment_chars_no_ws,
                item.comment_whitespace_chars,
            )
            for item in selected_stats
        ) or 1

        left = 24
        category_x = 24
        metric_x = 430
        bar_x = 620
        value_x = 1000
        bar_width = 350
        row_h = 24
        group_gap = 22
        width = 1160
        y = 24

        canvas.create_text(category_x, y, text="Category", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(metric_x, y, text="Metric", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(bar_x, y, text="Relative scale", anchor="w", font=("Segoe UI", 9, "bold"))
        canvas.create_text(value_x, y, text="Value", anchor="w", font=("Segoe UI", 9, "bold"))
        y += 32

        def draw_bar(label: str, value: int, maximum: int, color: str) -> None:
            nonlocal y
            bar_len = max(1, int((value / maximum) * bar_width)) if value else 0
            canvas.create_text(metric_x, y + 9, text=label, anchor="w", font=("Segoe UI", 8))
            canvas.create_rectangle(bar_x, y + 2, bar_x + bar_width, y + 18, outline="#dddddd", fill="#f8f8f8")
            if bar_len:
                canvas.create_rectangle(bar_x, y + 2, bar_x + bar_len, y + 18, outline=color, fill=color)
            canvas.create_text(value_x, y + 9, text=fmt_int(value), anchor="w", font=("Segoe UI", 8))
            y += row_h

        for item in selected_stats:
            name = display_name(item.group)
            if len(name) > 58:
                name = name[:55] + "..."
            canvas.create_text(category_x, y + 9, text=name, anchor="w", font=("Segoe UI", 8, "bold"))
            draw_bar("lines incl. comments", item.nonblank_lines_with_comments, max_lines, colors["with"])
            draw_bar("lines no comments", item.nonblank_lines_without_comments, max_lines, colors["without"])
            draw_bar("comment-only lines", item.comment_only_lines, max_lines, colors["comments"])
            draw_bar("chars incl. comments", item.chars_with_comments_with_ws, max_chars, colors["with"])
            draw_bar("non-ws chars incl. comments", item.chars_with_comments_no_ws, max_chars, colors["with"])
            draw_bar("ws chars incl. comments", item.whitespace_chars_with_comments, max_chars, colors["with"])
            draw_bar("chars no comments", item.chars_without_comments_with_ws, max_chars, colors["without"])
            draw_bar("non-ws chars no comments", item.chars_without_comments_no_ws, max_chars, colors["without"])
            draw_bar("ws chars no comments", item.whitespace_chars_without_comments, max_chars, colors["without"])
            draw_bar("comment chars", item.comment_chars_with_ws, max_chars, colors["comments"])
            draw_bar("comment chars no ws", item.comment_chars_no_ws, max_chars, colors["comments"])
            draw_bar("comment ws chars", item.comment_whitespace_chars, max_chars, colors["comments"])
            y += group_gap

        legend_y = y + 6
        legend_x = left
        for key, label in (("with", "with comments"), ("without", "without comments"), ("comments", "comments only")):
            canvas.create_rectangle(legend_x, legend_y, legend_x + 16, legend_y + 16, outline=colors[key], fill=colors[key])
            canvas.create_text(legend_x + 22, legend_y + 8, text=label, anchor="w", font=("Segoe UI", 9))
            legend_x += 170
        y = legend_y + 36
        canvas.configure(scrollregion=(0, 0, width, y))
        set_minimum_window_size(win, 1220, 760)

    def show_help() -> None:
        messagebox.showinfo("SSTL Code Stats Help", HELP_TEXT)

    def poll() -> None:
        nonlocal selected_stats
        try:
            while True:
                kind, payload = output.get_nowait()
                set_running(False)
                if kind == "stats":
                    selected_stats = list(payload)  # type: ignore[arg-type]
                    text.delete("1.0", tk.END)
                    write(human_report(selected_stats))
                else:
                    messagebox.showerror("SSTL Code Stats", str(payload))
        except queue.Empty:
            root.after(100, poll)

    buttons: list["tk.Button"] = []
    for label, command in (
        ("Run Stats", run_stats),
        ("Core Groups", select_core),
        ("Select All", select_all),
        ("Clear", clear_groups),
        ("Graphic View", show_graphic_view),
        ("Save CSV", save_csv),
        ("Save JSON", save_json),
        ("Help", show_help),
    ):
        button = tk.Button(controls, text=label, command=command)
        button.pack(side=tk.LEFT, padx=(0, 8))
        buttons.append(button)

    write("Select groups and click Run Stats.\n")
    write("The same script also works from a terminal; click Help for examples.\n")
    set_minimum_window_size(root, 1040, 720)
    root.after(100, poll)
    root.mainloop()
    return 0


def main(argv: list[str] | None = None) -> int:
    actual_argv = sys.argv[1:] if argv is None else argv
    if actual_argv:
        return run_cli(actual_argv)
    if maybe_relaunch_windows_gui(DEFAULT_ROOT, __file__, actual_argv):
        return 0
    if terminal_like_launch() and parent_is_known_terminal():
        return run_cli([])
    return run_gui()


if __name__ == "__main__":
    raise SystemExit(main())
