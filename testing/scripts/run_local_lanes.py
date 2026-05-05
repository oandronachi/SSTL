#!/usr/bin/env python3
"""Run local SSTL test presets in sequence without remote CI."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


DEFAULT_PRESETS = [
    "host-debug",
    "host-panic",
    "host-ub",
    "host-asan-ubsan",
    "host-coverage",
    "host-freestanding-probes",
]


def preset_build_dir(root: Path, preset: str) -> Path:
    return root / "build" / preset


def remove_stale_cache(root: Path, preset: str) -> None:
    """Remove a preset build tree when its cache points at another source dir.

    The test package was renamed during local development, and CMake refuses to
    reuse a build directory whose cache records the previous source path. This
    guard keeps the helper reproducible without deleting unrelated directories.
    """
    build_dir = preset_build_dir(root, preset)
    cache = build_dir / "CMakeCache.txt"
    if not cache.exists():
        return
    text = cache.read_text(encoding="utf-8", errors="replace")
    normalized_root = root.as_posix()
    if normalized_root in text or str(root) in text:
        return
    print(f"Removing stale CMake cache for preset {preset}: {build_dir}")
    shutil.rmtree(build_dir)


def run(cmd: list[str], cwd: Path) -> int:
    print("+", " ".join(cmd))
    return subprocess.call(cmd, cwd=str(cwd))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="Test package root")
    parser.add_argument("--sstl-root", required=True, help="Local SSTL implementation root")
    parser.add_argument("--preset", action="append", dest="presets", help="Preset to run; repeatable")
    parser.add_argument("--skip-tests", action="store_true")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    presets = args.presets or DEFAULT_PRESETS
    for preset in presets:
      remove_stale_cache(root, preset)
      configure = ["cmake", "--preset", preset, f"-DSSTL_ROOT={args.sstl_root}"]
      if run(configure, root) != 0:
        return 1
      if run(["cmake", "--build", "--preset", preset], root) != 0:
        return 1
      if not args.skip_tests and preset != "host-freestanding-probes":
        if run(["ctest", "--preset", preset], root) != 0:
          return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
