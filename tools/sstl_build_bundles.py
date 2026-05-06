#!/usr/bin/env python3
"""Generate all SSTL YAML bundle artifacts in the shared artifacts folder."""

from __future__ import annotations

import ast
import argparse
import hashlib
import io
import queue
import sys
import threading
import tokenize
from pathlib import Path

from sstl_tool_common import (
    attach_text_copy_context_menu,
    maybe_relaunch_windows_gui,
    parent_is_known_terminal,
    repo_root_from_script,
    set_buttons_enabled,
    set_minimum_window_size,
    terminal_like_launch,
)


ROOT = repo_root_from_script(__file__)
ARTIFACT_ROOT = ROOT / "artifacts"
INCLUDE_ROOT = ROOT / "include"
TEST_ROOT = ROOT / "testing"

IMPL_BUNDLE = ARTIFACT_ROOT / "sstl-bundle-impl.yaml"
TEST_BUNDLE = ARTIFACT_ROOT / "sstl-bundle-tests.yaml"
IMPL_NO_COMMENTS_BUNDLE = ARTIFACT_ROOT / "sstl-bundle-impl-no-comments.yaml"
TEST_NO_COMMENTS_BUNDLE = ARTIFACT_ROOT / "sstl-bundle-tests-no-comments.yaml"

CPP_EXTS = {".h", ".hpp", ".c", ".cpp", ".cc", ".cxx"}
PY_EXTS = {".py"}
HASH_COMMENT_EXTS = {".cmake", ".yaml", ".yml", ".toml", ".ini"}
HASH_COMMENT_NAMES = {"CMakeLists.txt"}

TEST_EXCLUDE_DIRS = {"build", "__pycache__"}
TEST_EXCLUDE_NAMES = {
    "sstl-bundle-tests.yaml",
    "sstl-bundle-tests-no-comments.yaml",
}

HELP_TEXT = """SSTL Bundle Builder

Terminal usage:
  python tools/sstl_build_bundles.py
  python tools/sstl_build_bundles.py --quiet
  python tools/sstl_build_bundles.py --verify
  python tools/sstl_build_bundles.py --verify-only
  python tools/sstl_build_bundles.py --gui

Double-click usage:
  Double-click this file. The GUI opens without requiring terminal steps. Click
  Generate Bundles to write all four YAML artifacts into artifacts/.

Generated artifacts:
  sstl-bundle-impl.yaml
  sstl-bundle-tests.yaml
  sstl-bundle-impl-no-comments.yaml
  sstl-bundle-tests-no-comments.yaml

The old split generators were consolidated into this script. Test bundles are
written to the same artifact folder as the implementation bundles. Use
--verify-only to validate existing bundle digests without regenerating files.
"""


def normalize_lf(text: str) -> str:
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    if not text.endswith("\n"):
        text += "\n"
    return text


def strip_c_like_comments(text: str) -> str:
    out: list[str] = []
    i = 0
    n = len(text)
    state = "normal"
    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if state == "normal":
            if ch == "/" and nxt == "/":
                i += 2
                while i < n and text[i] not in "\r\n":
                    i += 1
                continue
            if ch == "/" and nxt == "*":
                i += 2
                while i < n:
                    if text[i] == "\n":
                        out.append("\n")
                    if i + 1 < n and text[i] == "*" and text[i + 1] == "/":
                        i += 2
                        break
                    i += 1
                continue
            out.append(ch)
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            i += 1
        elif state == "string":
            out.append(ch)
            if ch == "\\" and i + 1 < n:
                out.append(text[i + 1])
                i += 2
                continue
            if ch == '"':
                state = "normal"
            i += 1
        else:
            out.append(ch)
            if ch == "\\" and i + 1 < n:
                out.append(text[i + 1])
                i += 2
                continue
            if ch == "'":
                state = "normal"
            i += 1
    return "".join(out)


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


def strip_python_comments(text: str) -> str:
    doc_lines = python_docstring_lines(text)
    tokens = []
    try:
        for tok in tokenize.generate_tokens(io.StringIO(text).readline):
            typ, _, start, _, _ = tok
            if typ in (tokenize.COMMENT, tokenize.ENCODING):
                continue
            if typ == tokenize.STRING and start[0] in doc_lines:
                continue
            tokens.append(tok)
        return tokenize.untokenize(tokens)
    except tokenize.TokenError:
        return "\n".join(line.split("#", 1)[0] for line in text.splitlines())


def strip_hash_comments(text: str) -> str:
    lines: list[str] = []
    for line in text.splitlines():
        state = "normal"
        cut = len(line)
        i = 0
        while i < len(line):
            ch = line[i]
            if state == "normal":
                if ch == "#":
                    cut = i
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
        lines.append(line[:cut].rstrip())
    return "\n".join(lines)


def strip_comments(path: Path, text: str) -> str:
    suffix = path.suffix.lower()
    if suffix in CPP_EXTS:
        return strip_c_like_comments(text)
    if suffix in PY_EXTS:
        return strip_python_comments(text)
    if suffix in HASH_COMMENT_EXTS or path.name in HASH_COMMENT_NAMES:
        return strip_hash_comments(text)
    return text


def read_file_payload(path: Path, strip: bool) -> bytes:
    data = path.read_bytes()
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        return data
    if strip:
        text = strip_comments(path, text)
    return normalize_lf(text).encode("utf-8")


def block_lines(text: str) -> list[str]:
    return ["      " + line for line in text.splitlines()]


def add_file(lines: list[str], root: Path, path: Path, strip: bool) -> None:
    rel = path.relative_to(root).as_posix()
    data = read_file_payload(path, strip)
    digest = hashlib.sha256(data).hexdigest()
    text = data.decode("utf-8")
    lines.extend([f"  - path: {rel}", f"    sha256: {digest}", "    content: |"])
    lines.extend(block_lines(text))


def implementation_files() -> list[Path]:
    return sorted(p for p in INCLUDE_ROOT.rglob("*") if p.is_file())


def include_test_file(path: Path) -> bool:
    rel = path.relative_to(TEST_ROOT).as_posix()
    parts = rel.split("/")
    if path.name in TEST_EXCLUDE_NAMES:
        return False
    if any(part.startswith(".") for part in parts):
        return False
    if any(part in TEST_EXCLUDE_DIRS for part in parts):
        return False
    return True


def test_files() -> list[Path]:
    return sorted(p for p in TEST_ROOT.rglob("*") if p.is_file() and include_test_file(p))


def render_impl_bundle(strip: bool) -> str:
    suffix = "-no-comments" if strip else ""
    lines = [
        "schema_version: 1",
        f"package_kind: sstl-bundle-impl{suffix}",
        f"package_id: sstl-bundle-impl-local{suffix}",
        "generator:",
        "  name: codex",
    ]
    if strip:
        lines.extend(["content_transform:", "  comments: stripped"])
    lines.extend([
        "scope:",
        "  included_roots:",
        "    - include/sstl",
        "implementation_profile:",
        "  language_profiles: [C99, C++03]",
        "  allocation_policy: static-storage-only",
        "  external_libraries: none",
        "  package_form: header-only",
        "files:",
    ])
    for path in implementation_files():
        add_file(lines, ROOT, path, strip)
    lines.extend([
        "extractor:",
        f"  virtual_root: sstl-bundle-impl{suffix}",
        "  newline: lf",
        "  verify_sha256_before_write: true",
    ])
    return "\n".join(lines) + "\n"


def render_test_bundle(strip: bool) -> str:
    suffix = "-no-comments" if strip else ""
    local_only_text = (
        "Bundle is local-only and omits remote automation metadata."
        if strip
        else "Bundle is local-only and intentionally omits remote automation metadata."
    )
    implementation_text = (
        "Non-deprecated workspace supplies SSTL implementation headers under include/sstl."
        if strip
        else "Non-deprecated workspace now supplies SSTL implementation headers under include/sstl."
    )
    lines = [
        "schema_version: 1",
        f"package_kind: sstl-bundle-tests{suffix}",
        f"package_id: sstl-bundle-tests-local{suffix}",
        "generator:",
        "  name: codex",
    ]
    if strip:
        lines.extend(["content_transform:", "  comments: stripped"])
    lines.extend([
        "assumptions:",
        "  - id: ASSUME-IMPLEMENTATION",
        "    status: closed",
        f"    description: {implementation_text}",
        "  - id: ASSUME-LOCAL-ONLY",
        "    status: fixed",
        f"    description: {local_only_text}",
        "source_corpus:",
        "  supplied_docs: []",
        "files:",
    ])
    for path in test_files():
        add_file(lines, TEST_ROOT, path, strip)
    lines.extend([
        "completeness_policy:",
        "  compile_required_lanes: 1.0",
        "  api_traceability: 1.0",
        "  requirement_traceability: 1.0",
        "  noalloc_required_lanes: true",
        "  coverage:",
        "    line_min: 0.95",
        "    branch_min: 0.90",
        "extractor:",
        f"  virtual_root: sstl-bundle-tests{suffix}",
        "  newline: lf",
        "  verify_sha256_before_write: true",
    ])
    return "\n".join(lines) + "\n"


def write_bundles() -> list[Path]:
    ARTIFACT_ROOT.mkdir(parents=True, exist_ok=True)
    outputs = [
        (IMPL_BUNDLE, render_impl_bundle(False)),
        (TEST_BUNDLE, render_test_bundle(False)),
        (IMPL_NO_COMMENTS_BUNDLE, render_impl_bundle(True)),
        (TEST_NO_COMMENTS_BUNDLE, render_test_bundle(True)),
    ]
    written: list[Path] = []
    for path, text in outputs:
        path.write_text(text, encoding="utf-8")
        written.append(path)
    return written


def bundle_paths() -> list[Path]:
    return [
        IMPL_BUNDLE,
        TEST_BUNDLE,
        IMPL_NO_COMMENTS_BUNDLE,
        TEST_NO_COMMENTS_BUNDLE,
    ]


def verify_bundle(path: Path) -> tuple[int, list[str]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    failures: list[str] = []
    count = 0
    i = 0
    while i < len(lines):
        if lines[i].startswith("  - path: "):
            virtual_path = lines[i].split(": ", 1)[1]
            if i + 2 >= len(lines) or not lines[i + 1].startswith("    sha256: ") or lines[i + 2] != "    content: |":
                failures.append(f"{virtual_path}: malformed file entry")
                i += 1
                continue
            expected = lines[i + 1].split(": ", 1)[1]
            i += 3
            content_lines: list[str] = []
            while i < len(lines) and not lines[i].startswith("  - path: ") and lines[i] not in {"extractor:", "completeness_policy:"}:
                line = lines[i]
                if line.startswith("      "):
                    content_lines.append(line[6:])
                    i += 1
                    continue
                if line == "":
                    content_lines.append("")
                    i += 1
                    continue
                break
            actual = hashlib.sha256(("\n".join(content_lines) + "\n").encode("utf-8")).hexdigest()
            if actual != expected:
                failures.append(f"{virtual_path}: {actual} != {expected}")
            count += 1
            continue
        i += 1
    return count, failures


def verify_bundles(paths: list[Path] | None = None) -> list[tuple[Path, int, list[str]]]:
    results: list[tuple[Path, int, list[str]]] = []
    for path in paths or bundle_paths():
        if not path.exists():
            results.append((path, 0, ["missing bundle file"]))
            continue
        count, failures = verify_bundle(path)
        results.append((path, count, failures))
    return results


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate all SSTL bundle YAML files in the shared artifacts folder.")
    parser.add_argument("--quiet", action="store_true", help="Do not print generated bundle paths.")
    parser.add_argument("--verify", action="store_true", help="Verify bundle sha256 entries after generation.")
    parser.add_argument("--verify-only", action="store_true", help="Verify existing bundle sha256 entries without regenerating.")
    parser.add_argument("--gui", action="store_true", help="Open the Tkinter GUI.")
    return parser.parse_args(argv)


def log_verification(results: list[tuple[Path, int, list[str]]]) -> int:
    ok = True
    for path, count, failures in results:
        status = "PASS" if not failures else "FAIL"
        print(f"{status}: {path} files={count} digest_failures={len(failures)}")
        for failure in failures[:10]:
            print(f"  {failure}")
        ok = ok and not failures
    return 0 if ok else 1


def run_cli(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.gui:
        return run_gui()
    if args.verify_only:
        return log_verification(verify_bundles())
    written = write_bundles()
    if not args.quiet:
        for path in written:
            print(path)
    if args.verify:
        return log_verification(verify_bundles(written))
    return 0


def run_gui() -> int:
    try:
        import tkinter as tk
        from tkinter import messagebox, scrolledtext
    except Exception:
        return run_cli([])

    root = tk.Tk()
    root.title("SSTL Bundle Builder")
    root.geometry("900x560")

    output: "queue.Queue[str | tuple[str, int]]" = queue.Queue()
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

    def generate_worker() -> None:
        try:
            written = write_bundles()
            output.put("Generated bundles:\n")
            for path in written:
                output.put(f"  {path}\n")
            results = verify_bundles(written)
            failures = sum(len(item[2]) for item in results)
            output.put("\nVerification:\n")
            for path, count, path_failures in results:
                status = "PASS" if not path_failures else "FAIL"
                output.put(f"  {status}: {path.name} files={count} digest_failures={len(path_failures)}\n")
            output.put(("generate", 0 if failures == 0 else 1))
        except Exception as exc:
            output.put(f"ERROR: {exc}\n")
            output.put(("generate", 1))

    def verify_worker() -> None:
        try:
            results = verify_bundles()
            failures = sum(len(item[2]) for item in results)
            output.put("Verification:\n")
            for path, count, path_failures in results:
                status = "PASS" if not path_failures else "FAIL"
                output.put(f"  {status}: {path.name} files={count} digest_failures={len(path_failures)}\n")
                for failure in path_failures[:10]:
                    output.put(f"    {failure}\n")
            output.put(("verify", 0 if failures == 0 else 1))
        except Exception as exc:
            output.put(f"ERROR: {exc}\n")
            output.put(("verify", 1))

    def start(kind: str) -> None:
        set_running(True)
        text.delete("1.0", tk.END)
        if kind == "generate":
            write("Generating all SSTL bundle YAML files...\n\n")
            threading.Thread(target=generate_worker, daemon=True).start()
        else:
            write("Verifying current SSTL bundle hashes...\n\n")
            threading.Thread(target=verify_worker, daemon=True).start()

    def show_help() -> None:
        messagebox.showinfo("SSTL Bundle Builder Help", HELP_TEXT)

    def poll() -> None:
        try:
            while True:
                item = output.get_nowait()
                if isinstance(item, tuple):
                    _, code = item
                    set_running(False)
                    if code == 0:
                        messagebox.showinfo("SSTL Bundle Builder", "Bundle operation completed successfully.")
                    else:
                        messagebox.showerror("SSTL Bundle Builder", "Bundle operation failed. See the log for details.")
                else:
                    write(item)
        except queue.Empty:
            root.after(100, poll)

    for label, command in (
        ("Generate Bundles", lambda: start("generate")),
        ("Verify Hashes", lambda: start("verify")),
        ("Help", show_help),
    ):
        button = tk.Button(controls, text=label, command=command)
        button.pack(side=tk.LEFT, padx=(0, 8))
        buttons.append(button)

    write(f"Click Generate Bundles to write all four YAML bundles into:\n{ARTIFACT_ROOT}\n")
    write("Click Verify Hashes to check current bundle sha256 entries.\n")
    write("CLI is also available; click Help for examples.\n")
    set_minimum_window_size(root, 900, 560)
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
