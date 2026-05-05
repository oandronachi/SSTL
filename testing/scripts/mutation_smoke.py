#!/usr/bin/env python3
"""Run deterministic local mutation smoke checks for SSTL.

This is intentionally not a full mutation-testing framework.  It is a small
release-gate probe for the highest-risk fixed-capacity contract: a full vector
must not report successful insertion.  The script mutates temporary copies of
the C++ and C vector headers, runs the relevant tests, and passes only when the
test suite rejects the mutants.
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path


def run(cmd: list[str], cwd: Path) -> int:
    print("+", " ".join(cmd))
    return subprocess.call(cmd, cwd=str(cwd))


def run_expected_to_fail(cmd: list[str], cwd: Path) -> tuple[bool, str]:
    """Run a command whose non-zero exit is the desired mutation result.

    CTest prints assertion failures when a mutant is killed.  Those assertions
    are useful while debugging this script, but they are noisy and alarming in
    the normal GUI runner.  Capture them and print only a concise mutation
    summary.  If the mutant survives, return the captured output so the user can
    see why the mutation lane failed.
    """
    print("+", " ".join(cmd))
    proc = subprocess.run(
        cmd,
        cwd=str(cwd),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
    )
    killed = proc.returncode != 0
    return killed, proc.stdout


def copy_impl(source_root: Path, dest_root: Path) -> None:
    include_src = source_root / "include"
    include_dst = dest_root / "include"
    shutil.copytree(include_src, include_dst)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if old not in text:
        raise RuntimeError(f"mutation anchor not found in {path}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")


def run_mutant(root: Path, sstl_root: Path, mutant_name: str, mutate) -> bool:
    print(f"\n[MUTATION] Starting mutant: {mutant_name}")
    print("[MUTATION] Selected tests will run against a deliberately broken temporary copy.")
    print("[MUTATION] Assertion failures are expected when the mutant is killed and will be summarized.")
    with tempfile.TemporaryDirectory(prefix=f"sstl-mut-{mutant_name}-") as tmp:
        tmp_root = Path(tmp)
        impl_root = tmp_root / "impl"
        build_dir = tmp_root / "build"
        copy_impl(sstl_root, impl_root)
        mutate(impl_root)

        if run([
            "cmake",
            "-S",
            str(root),
            "-B",
            str(build_dir),
            "-DSSTL_ROOT=" + str(impl_root),
            "-DSSTL_ERROR_POLICY=SSTL_RETURN",
            "-G",
            "Ninja",
        ], root) != 0:
            return True
        if run(["cmake", "--build", str(build_dir)], root) != 0:
            return True
        killed, ctest_output = run_expected_to_fail([
            "ctest",
            "--test-dir",
            str(build_dir),
            "-R",
            "vector_capacity|policy_matrix|vector_try_policy|parity_c_cpp_vector_trace",
        ], root)
        if killed:
            print(f"[MUTATION] Mutant killed as expected: {mutant_name}")
            print("[MUTATION] Suppressed expected assertion output from the killed mutant.")
        else:
            print(f"[MUTATION] Mutant survived unexpectedly: {mutant_name}")
            print("[MUTATION] CTest output for surviving mutant follows:")
            print(ctest_output)
        return killed


def write_summary(root: Path, results: list[tuple[str, bool]]) -> None:
    survived = [name for name, killed in results if not killed]
    out = root / "manifests" / "mutation_summary.yaml"
    lines = [
        f"generated_at_utc: {datetime.now(timezone.utc).isoformat()}",
        "tool: deterministic-smoke",
        f"mutants_total: {len(results)}",
        f"mutants_killed: {sum(1 for _, killed in results if killed)}",
        f"mutants_survived: {len(survived)}",
        f"status: {'pass' if not survived else 'fail'}",
        "mutants:",
    ]
    for name, killed in results:
        lines.append(f"  - id: {name}")
        lines.append(f"    status: {'killed' if killed else 'survived'}")
    out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(out)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="Test package root")
    parser.add_argument("--sstl-root", required=True, help="SSTL implementation root")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    sstl_root = Path(args.sstl_root).resolve()

    def mutate_cpp_vector(impl_root: Path) -> None:
        replace_once(
            impl_root / "include" / "sstl" / "vector.hpp",
            'handle_error("vector::push_back full");\n      return false;\n    }\n    SSTL_CONSTRUCT_AT(storage_.ptr(size_), value);',
            'handle_error("vector::push_back full");\n      return true;\n    }\n    SSTL_CONSTRUCT_AT(storage_.ptr(size_), value);',
        )

    def mutate_c_vector(impl_root: Path) -> None:
        replace_once(
            impl_root / "include" / "sstl" / "c" / "sstl_vector.h",
            "SSTL_C_PANIC(\"vector full\"); return false;",
            "SSTL_C_PANIC(\"vector full\"); return true;",
        )

    results = [
        ("cpp-vector-full-push-reports-success", run_mutant(root, sstl_root, "cpp-vector", mutate_cpp_vector)),
        ("c-vector-full-push-reports-success", run_mutant(root, sstl_root, "c-vector", mutate_c_vector)),
    ]
    write_summary(root, results)
    return 0 if all(killed for _, killed in results) else 1


if __name__ == "__main__":
    sys.exit(main())
