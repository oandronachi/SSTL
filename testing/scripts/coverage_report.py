#!/usr/bin/env python3
"""Summarize local coverage output and enforce SSTL release thresholds.

This helper is intentionally small and dependency-free because it is called
from CMake targets as well as from ad-hoc developer shells. It accepts the
coverage formats produced by common C/C++ toolchains, prints an overall report,
prints per-file detail when the input format contains it, and writes a YAML
manifest that spreadsheet tools and validation scripts can consume.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path


DEFAULT_SOURCE_PREFIXES = ("include/sstl/",)
ANSI_RESET = "\033[0m"
ANSI = {
    "bold": "\033[1m",
    "dim": "\033[2m",
    "red": "\033[31m",
    "green": "\033[32m",
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
    return color_text(status, "green" if status == "pass" else "red", enabled)


def color_pct(value: float, threshold: float, enabled: bool) -> str:
    return color_text(pct_text(value), "green" if value >= threshold else "red", enabled)


@dataclass
class CoverageFile:
    """Coverage counters for one source file."""

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

    def status(self, line_min: float, branch_min: float) -> str:
        line_ok = self.line_pct >= line_min
        branch_ok = self.branch_pct >= branch_min if self.branches_found else True
        return "pass" if line_ok and branch_ok else "fail"


@dataclass
class CoverageReport:
    """Coverage counters grouped into an overall report."""

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

    def status(self, line_min: float, branch_min: float) -> str:
        return "pass" if self.line_pct >= line_min and self.branch_pct >= branch_min else "fail"


def display_coverage_path(path: str) -> str:
    """Normalize a compiler-emitted path into a stable source-tree display path."""

    normalized = path.replace("\\", "/")
    marker = "include/sstl/"
    index = normalized.find(marker)
    if index >= 0:
        return normalized[index:]
    return normalized


def wanted_source(display_path: str, source_prefixes: tuple[str, ...]) -> bool:
    """Return true when a coverage entry belongs to the source surface we report."""

    return display_path.startswith(source_prefixes)


def parse_llvm_cov_json(path: Path, source_prefixes: tuple[str, ...]) -> CoverageReport:
    """Parse llvm-cov export JSON, preserving per-file counters."""

    data = json.loads(path.read_text(encoding="utf-8"))
    files: list[CoverageFile] = []
    for entry in data["data"][0].get("files", []):
        display_path = display_coverage_path(entry.get("filename", ""))
        if not wanted_source(display_path, source_prefixes):
            continue
        summary = entry["summary"]
        lines = summary["lines"]
        branches = summary["branches"]
        files.append(
            CoverageFile(
                display_path,
                int(lines.get("count", 0)),
                int(lines.get("covered", 0)),
                int(branches.get("count", 0)),
                int(branches.get("covered", 0)),
            )
        )
    return CoverageReport(path, sorted(files, key=lambda item: item.path))


def parse_lcov_info(path: Path, source_prefixes: tuple[str, ...]) -> CoverageReport:
    """Parse an LCOV info file into source-only per-file counters."""

    by_path: dict[str, CoverageFile] = {}
    current: CoverageFile | None = None
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if raw.startswith("SF:"):
            display_path = display_coverage_path(raw.split(":", 1)[1])
            current = CoverageFile(display_path) if wanted_source(display_path, source_prefixes) else None
            if current:
                by_path.setdefault(display_path, current)
                current = by_path[display_path]
        elif current and raw.startswith("LF:"):
            current.lines_found += int(raw.split(":", 1)[1])
        elif current and raw.startswith("LH:"):
            current.lines_hit += int(raw.split(":", 1)[1])
        elif current and raw.startswith("BRF:"):
            current.branches_found += int(raw.split(":", 1)[1])
        elif current and raw.startswith("BRH:"):
            current.branches_hit += int(raw.split(":", 1)[1])
        elif raw == "end_of_record":
            current = None
    return CoverageReport(path, [by_path[key] for key in sorted(by_path)])


def parse_gcov_summary(path: Path) -> CoverageReport:
    """Parse a gcovr/gcov text summary.

    Plain summary files do not contain per-file counters, so the report keeps a
    synthetic overall row. LCOV or llvm-cov JSON should be preferred when a real
    per-file table is needed.
    """

    text = path.read_text(encoding="utf-8", errors="replace")
    line_match = re.search(r"lines\.+:\s*([0-9.]+)%", text)
    branch_match = re.search(r"branches\.+:\s*([0-9.]+)%", text)
    line_pct = float(line_match.group(1)) / 100.0 if line_match else 0.0
    branch_pct = float(branch_match.group(1)) / 100.0 if branch_match else 0.0
    synthetic = CoverageFile(
        "overall-summary-only",
        10000,
        int(round(line_pct * 10000.0)),
        10000,
        int(round(branch_pct * 10000.0)),
    )
    return CoverageReport(path, [synthetic])


def pct_text(value: float) -> str:
    return f"{value * 100.0:6.2f}%"


def report_lines(report: CoverageReport, line_min: float, branch_min: float, color: bool = False) -> list[str]:
    """Format an overall plus per-file report for terminal output."""

    status = report.status(line_min, branch_min)
    lines = [
        color_text("== Coverage report", "bold", color),
        f"Source: {report.source}",
        (
            "Overall: line "
            f"{color_pct(report.line_pct, line_min, color)} ({report.lines_hit}/{report.lines_found}), "
            "branch "
            f"{color_pct(report.branch_pct, branch_min, color)} ({report.branches_hit}/{report.branches_found}), "
            f"status {color_status(status, color)}"
        ),
        "",
        "status     line   line hit/found   branch   branch hit/found file",
    ]
    for item in report.files:
        item_status = item.status(line_min, branch_min)
        status_field = color_text(f"{item_status:<7}", "green" if item_status == "pass" else "red", color)
        line_field = color_text(f"{pct_text(item.line_pct):>8}", "green" if item.line_pct >= line_min else "red", color)
        branch_field = (
            color_text(f"{pct_text(item.branch_pct):>8}", "green" if item.branch_pct >= branch_min else "red", color)
            if item.branches_found
            else color_text("   n/a", "dim", color)
        )
        lines.append(
            f"{status_field} "
            f"{line_field} "
            f"{item.lines_hit:7}/{item.lines_found:<7} "
            f"{branch_field} "
            f"{item.branches_hit:7}/{item.branches_found:<7} "
            f"{item.path}"
        )
    return lines


def manifest_artifact_path(root: Path, path: Path) -> str:
    """Return a portable artifact path for generated YAML manifests."""

    try:
        return str(path.resolve().relative_to(root.resolve())).replace("\\", "/")
    except Exception:
        return str(path).replace("\\", "/")


def write_manifest(root: Path, report: CoverageReport, line_min: float, branch_min: float) -> Path:
    """Write the machine-readable coverage manifest used by validation tooling."""

    out = root / "manifests" / "coverage_summary.yaml"
    out.parent.mkdir(parents=True, exist_ok=True)
    rows = [
        f"generated_at_utc: {datetime.now(timezone.utc).isoformat()}",
        f"source: {manifest_artifact_path(root, report.source)}",
        f"line_coverage: {report.line_pct:.6f}",
        f"branch_coverage: {report.branch_pct:.6f}",
        f"line_min: {line_min:.6f}",
        f"branch_min: {branch_min:.6f}",
        f"status: {report.status(line_min, branch_min)}",
        "files:",
    ]
    for item in report.files:
        rows.extend(
            [
                f"  - path: {item.path}",
                f"    line_coverage: {item.line_pct:.6f}",
                f"    branch_coverage: {item.branch_pct:.6f}",
                f"    lines_hit: {item.lines_hit}",
                f"    lines_found: {item.lines_found}",
                f"    branches_hit: {item.branches_hit}",
                f"    branches_found: {item.branches_found}",
                f"    status: {item.status(line_min, branch_min)}",
            ]
        )
    out.write_text("\n".join(rows) + "\n", encoding="utf-8")
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="Test package root")
    parser.add_argument("--input", required=True, help="llvm-cov JSON, lcov .info, or gcov summary")
    parser.add_argument("--format", choices=["llvm-json", "lcov", "gcov-summary"], required=True)
    parser.add_argument("--line-min", type=float, default=0.95)
    parser.add_argument("--branch-min", type=float, default=0.90)
    parser.add_argument("--color", choices=["auto", "always", "never"], default="auto", help="Colorize CLI output. Default: auto.")
    parser.add_argument(
        "--source-prefix",
        action="append",
        default=None,
        help="Source path prefix to include in per-file reports. May be repeated.",
    )
    args = parser.parse_args()

    source = Path(args.input)
    source_prefixes = tuple(args.source_prefix or DEFAULT_SOURCE_PREFIXES)
    if args.format == "llvm-json":
        report = parse_llvm_cov_json(source, source_prefixes)
    elif args.format == "lcov":
        report = parse_lcov_info(source, source_prefixes)
    else:
        report = parse_gcov_summary(source)

    color = color_enabled_from_mode(args.color)
    for line in report_lines(report, args.line_min, args.branch_min, color):
        print(line)
    out = write_manifest(Path(args.root), report, args.line_min, args.branch_min)
    print(f"\nManifest: {out}")
    return 0 if report.status(args.line_min, args.branch_min) == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
