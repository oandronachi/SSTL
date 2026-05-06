#!/usr/bin/env python3
# test_id: package.misra.advisory_scan
# api_ids: [package.misra.advisory_scan]
# req_ids: [REQ-06]
# patterns: [MISRA-ADVISORY-SCAN]
"""Run a dependency-light MISRA advisory scan over preprocessed C macro outputs."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys


LINE_MARKER_RE = re.compile(r'^\s*#(?:line)?\s+\d+\s+"([^"]+)"')
DEFAULT_SOURCES = [
    "tests/c99/compile_probe/sstl_c99_probe.c",
    "tests/c99/adapters/test_queue_stack_priority_queue.c",
    "tests/c99/algorithm/test_algorithm_contracts.c",
    "tests/c99/list/test_list_extended_ops.c",
    "tests/c99/map/test_flat_unordered_semantics.c",
    "tests/c99/zero_capacity/test_zero_capacity.c",
    "tests/regression/reg_zero_capacity_no_zero_length_arrays.c",
]

RULES = [
    (
        "MISRA-C-2012 Dir 4.12",
        re.compile(r"\b(?:malloc|calloc|realloc|free|aligned_alloc|posix_memalign|memalign)\s*\("),
        "dynamic allocation call in preprocessed SSTL C surface",
    ),
    (
        "MISRA-C-2012 Rule 15.1",
        re.compile(r"\bgoto\b"),
        "goto statement in preprocessed SSTL C surface",
    ),
    (
        "MISRA-C-2012 Rule 21.4",
        re.compile(r"\b(?:setjmp|longjmp)\s*\("),
        "setjmp/longjmp in preprocessed SSTL C surface",
    ),
]


def normalize(path: Path) -> Path:
    try:
        return path.resolve()
    except OSError:
        return path.absolute()


def is_relative_to(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
        return True
    except ValueError:
        return False


def yaml_scalar(value: str) -> str:
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def discover_sources(root: Path) -> list[Path]:
    sources: list[Path] = []
    for item in DEFAULT_SOURCES:
        path = root / item
        if path.exists():
            sources.append(path)
    return sources


def is_msvc_compiler(cc: str) -> bool:
    name = Path(cc).name.lower()
    return name in {"cl", "cl.exe"}


def find_vswhere() -> str | None:
    direct = shutil.which("vswhere")
    if direct:
        return direct
    for key in ("ProgramFiles(x86)", "ProgramFiles", "ProgramW6432"):
        base = os.environ.get(key)
        if not base:
            continue
        candidate = Path(base) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
        if candidate.exists():
            return str(candidate)
    return None


def find_vcvars64() -> str | None:
    vswhere = find_vswhere()
    if not vswhere:
        return None
    try:
        proc = subprocess.run(
            [
                vswhere,
                "-latest",
                "-products",
                "*",
                "-requires",
                "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                "-find",
                r"VC\Auxiliary\Build\vcvars64.bat",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            errors="replace",
            timeout=20,
        )
    except Exception:
        return None
    if proc.returncode != 0:
        return None
    for line in proc.stdout.splitlines():
        candidate = line.strip()
        if candidate and Path(candidate).exists():
            return candidate
    return None


def compiler_command(cc: str, root: Path, sstl_root: Path, source: Path) -> list[str]:
    includes = [
        sstl_root / "include",
        root / "test_support" / "c99",
        root / "test_support" / "common",
    ]
    if is_msvc_compiler(cc):
        return [
            cc,
            "/nologo",
            "/E",
            "/TC",
            "/D",
            "SSTL_ON_ERROR=SSTL_RETURN",
            *[item for include in includes for item in ("/I", str(include))],
            str(source),
        ]
    return [
        cc,
        "-E",
        "-std=c99",
        "-DSSTL_ON_ERROR=SSTL_RETURN",
        *["-I" + str(include) for include in includes],
        str(source),
    ]


def maybe_wrap_msvc_environment(cc: str, command: list[str], batch_path: Path) -> list[str]:
    if not is_msvc_compiler(cc) or os.environ.get("INCLUDE"):
        return command
    vcvars = find_vcvars64()
    if not vcvars:
        return command
    batch_path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "@echo off",
        "call " + subprocess.list2cmdline([vcvars]) + " >nul",
        "if errorlevel 1 exit /b %errorlevel%",
        subprocess.list2cmdline(command),
    ]
    batch_path.write_text("\r\n".join(lines) + "\r\n", encoding="ascii")
    return ["cmd.exe", "/d", "/c", str(batch_path)]


def preprocess_batch_path(root: Path, source: Path) -> Path:
    try:
        rel = source.relative_to(root)
    except ValueError:
        rel = Path(source.name)
    safe = re.sub(r"[^A-Za-z0-9_]+", "_", str(rel)).strip("_")
    return root / "build" / "misra-advisory-scan" / str(os.getpid()) / (safe + ".cmd")


def preprocess(cc: str, root: Path, sstl_root: Path, source: Path) -> tuple[int, str]:
    command = maybe_wrap_msvc_environment(cc, compiler_command(cc, root, sstl_root, source), preprocess_batch_path(root, source))
    proc = subprocess.run(
        command,
        cwd=str(root),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
    )
    return proc.returncode, proc.stdout


def local_path_from_marker(raw: str, current: Path | None) -> Path | None:
    marker = Path(raw)
    if marker.is_absolute():
        return normalize(marker)
    if current is not None and current.parent.exists():
        return normalize(current.parent / marker)
    return normalize(marker)


def scan_preprocessed(text: str, root: Path, sstl_root: Path, source: Path) -> list[dict[str, str]]:
    findings: list[dict[str, str]] = []
    current_file: Path | None = source
    local_roots = [
        normalize(sstl_root / "include" / "sstl" / "c"),
        normalize(root / "tests" / "c99"),
        normalize(root / "tests" / "regression"),
    ]

    for line_number, line in enumerate(text.splitlines(), 1):
        marker = LINE_MARKER_RE.match(line)
        if marker:
            current_file = local_path_from_marker(marker.group(1), current_file)
            continue
        if current_file is None:
            continue
        if not any(is_relative_to(current_file, local_root) for local_root in local_roots):
            continue
        for rule_id, pattern, message in RULES:
            if pattern.search(line):
                findings.append(
                    {
                        "rule": rule_id,
                        "message": message,
                        "source": str(source.relative_to(root)).replace("\\", "/"),
                        "expanded_file": str(current_file),
                        "preprocessed_line": str(line_number),
                        "text": line.strip()[:160],
                    }
                )
    return findings


def write_summary(path: Path, status: str, sources: list[Path], findings: list[dict[str, str]], failures: list[str], root: Path) -> None:
    lines = [
        f"status: {status}",
        f"generated_at_utc: {datetime.now(timezone.utc).isoformat()}",
        "scanner: preprocessor-pattern-advisory",
        "sources:",
    ]
    lines += [f"  - {yaml_scalar(str(source.relative_to(root)).replace(chr(92), '/'))}" for source in sources] or ["  []"]
    lines.append("failed_preprocess:")
    lines += [f"  - {yaml_scalar(item)}" for item in failures] or ["  []"]
    lines.append("findings:")
    if findings:
        for finding in findings:
            lines.append("  - rule: " + yaml_scalar(finding["rule"]))
            lines.append("    message: " + yaml_scalar(finding["message"]))
            lines.append("    source: " + yaml_scalar(finding["source"]))
            lines.append("    expanded_file: " + yaml_scalar(finding["expanded_file"]))
            lines.append("    preprocessed_line: " + finding["preprocessed_line"])
            lines.append("    text: " + yaml_scalar(finding["text"]))
    else:
        lines.append("  []")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="Test package root")
    parser.add_argument("--sstl-root", default="", help="SSTL implementation root")
    parser.add_argument("--cc", default="", help="C compiler used for preprocessing")
    parser.add_argument("--output", default="", help="Summary manifest output")
    args = parser.parse_args()

    root = normalize(Path(args.root))
    sstl_root = normalize(Path(args.sstl_root)) if args.sstl_root else normalize(root.parent)
    cc = args.cc or shutil.which("cc") or shutil.which("gcc") or shutil.which("clang") or ""
    output = normalize(Path(args.output)) if args.output else root / "manifests" / "misra_advisory_summary.yaml"
    sources = discover_sources(root)
    failures: list[str] = []
    findings: list[dict[str, str]] = []

    if not cc:
        failures.append("no_c_compiler_found")
    if not sources:
        failures.append("no_c_sources_found")

    if cc and sources:
        for source in sources:
            code, text = preprocess(cc, root, sstl_root, source)
            if code != 0:
                failures.append(f"{source.relative_to(root)}:preprocess_exit_{code}")
                continue
            findings.extend(scan_preprocessed(text, root, sstl_root, source))

    status = "pass" if not failures and not findings else "fail"
    write_summary(output, status, sources, findings, failures, root)
    if failures:
        for failure in failures:
            print("MISRA advisory preprocess failure:", failure)
    if findings:
        for finding in findings:
            print(
                "MISRA advisory finding:",
                finding["rule"],
                finding["source"],
                "preprocessed line",
                finding["preprocessed_line"],
                finding["message"],
            )
    return 0 if status == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
