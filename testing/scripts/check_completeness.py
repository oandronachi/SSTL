#!/usr/bin/env python3
"""Fail-closed completeness checker for the SSTL test package."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from datetime import datetime, timezone


REQ_RE = re.compile(r"req_ids:\s*\[([^\]]*)\]|req_ids:\s*$")
API_RE = re.compile(r"api_ids:\s*\[([^\]]*)\]|api_ids:\s*$")
ID_RE = re.compile(r"-\s+([A-Z0-9_-]+|[a-zA-Z0-9_.-]+)")
CMAKE_TEST_SOURCE_RE = re.compile(
    r"add_sstl_(?:cpp03|c99|python)_test\s*\(\s*[^\s)]+\s+(\"[^\"]+\"|'[^']+'|[^\s)]+)",
    re.MULTILINE,
)


def split_inline_list(text: str) -> set[str]:
    return {part.strip().strip("'\"") for part in text.split(",") if part.strip()}


def parse_inline_list_value(raw: str) -> set[str]:
    stripped = raw.strip().strip("*/").strip()
    if stripped.startswith("[") and "]" in stripped:
        return split_inline_list(stripped.split("[", 1)[1].split("]", 1)[0])
    return set()


def collect_manifest_ids(path: Path, key: str) -> set[str]:
    text = path.read_text(encoding="utf-8")
    if key == "requirements":
        return set(re.findall(r"req_id:\s*([A-Z0-9_-]+)", text))
    if key == "apis":
        return set(re.findall(r"api_id:\s*([a-zA-Z0-9_.-]+)", text))
    raise ValueError(key)


def collect_nontrace_requirement_ids(path: Path) -> set[str]:
    text = path.read_text(encoding="utf-8")
    nontrace: set[str] = set()
    current: str | None = None
    for line in text.splitlines():
        match = re.search(r"req_id:\s*([A-Z0-9_-]+)", line)
        if match:
            current = match.group(1)
            continue
        if current and re.search(r"trace_required:\s*false\b", line):
            nontrace.add(current)
    return nontrace


def collect_test_refs(path: Path) -> tuple[set[str], set[str], list[str]]:
    reqs: set[str] = set()
    apis: set[str] = set()
    files: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if stripped.startswith("virtual_path:"):
          files.append(stripped.split(":", 1)[1].strip().strip("'\""))
        if "req_ids:" in stripped and "[" in stripped:
          reqs.update(split_inline_list(stripped.split("[", 1)[1].split("]", 1)[0]))
        if "api_ids:" in stripped and "[" in stripped:
          apis.update(split_inline_list(stripped.split("[", 1)[1].split("]", 1)[0]))
    return reqs, apis, files


def collect_test_manifest_entries(path: Path) -> dict[str, dict[str, object]]:
    entries: dict[str, dict[str, object]] = {}
    current: dict[str, object] | None = None
    for line in path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if stripped.startswith("- test_id:"):
            if current and "virtual_path" in current:
                entries[str(current["virtual_path"])] = current
            current = {
                "test_id": stripped.split(":", 1)[1].strip().strip("'\""),
                "req_ids": set(),
                "api_ids": set(),
                "patterns": set(),
            }
            continue
        if current is None or ":" not in stripped:
            continue
        key, raw = stripped.split(":", 1)
        key = key.strip()
        raw = raw.strip()
        if key == "virtual_path":
            current[key] = normalize_manifest_path(raw)
        elif key in {"req_ids", "api_ids", "patterns"}:
            current[key] = parse_inline_list_value(raw)
    if current and "virtual_path" in current:
        entries[str(current["virtual_path"])] = current
    return entries


def strip_comment_prefix(line: str) -> str:
    stripped = line.strip()
    if stripped.startswith("//"):
        stripped = stripped[2:].strip()
    elif stripped.startswith("#"):
        stripped = stripped[1:].strip()
    elif stripped.startswith("/*"):
        stripped = stripped[2:].strip()
    elif stripped.startswith("*"):
        stripped = stripped[1:].strip()
    if stripped.endswith("*/"):
        stripped = stripped[:-2].strip()
    return stripped


def collect_source_header_metadata(path: Path) -> dict[str, object]:
    metadata: dict[str, object] = {
        "req_ids": set(),
        "api_ids": set(),
        "patterns": set(),
    }
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines()[:80]:
        stripped = line.strip()
        if stripped.startswith("#include") or stripped.startswith("import "):
            break
        cleaned = strip_comment_prefix(stripped)
        if ":" not in cleaned:
            continue
        key, raw = cleaned.split(":", 1)
        key = key.strip()
        raw = raw.strip()
        if key == "test_id":
            metadata["test_id"] = raw.strip().strip("'\"")
        elif key in {"req_ids", "api_ids", "patterns"}:
            metadata[key] = parse_inline_list_value(raw)
    return metadata


def check_source_header_manifest_sync(root: Path, entries: dict[str, dict[str, object]]) -> list[str]:
    failures: list[str] = []
    for virtual_path, expected in sorted(entries.items()):
        path = root / virtual_path
        if not path.exists():
            continue
        source = collect_source_header_metadata(path)
        expected_test_id = str(expected.get("test_id", ""))
        source_test_id = str(source.get("test_id", ""))
        if source_test_id != expected_test_id:
            failures.append(f"source_header_test_id_mismatch:{virtual_path}:manifest={expected_test_id}:source={source_test_id or '<missing>'}")
        for key in ("req_ids", "api_ids", "patterns"):
            expected_values = set(expected.get(key, set()))
            source_values = set(source.get(key, set()))
            if source_values != expected_values:
                failures.append(
                    f"source_header_{key}_mismatch:{virtual_path}:manifest={sorted(expected_values)}:source={sorted(source_values)}"
                )
    return failures


def normalize_manifest_path(value: str) -> str:
    return value.strip().strip("'\"").replace("\\", "/")


def collect_cmake_registered_test_sources(path: Path) -> set[str]:
    text = path.read_text(encoding="utf-8", errors="replace")
    without_comments = "\n".join(line.split("#", 1)[0] for line in text.splitlines())
    return {normalize_manifest_path(match.group(1)) for match in CMAKE_TEST_SOURCE_RE.finditer(without_comments)}


def c_header_has_public_surface(path: Path) -> bool:
    text = path.read_text(encoding="utf-8", errors="replace")
    if re.search(r"#\s*define\s+SSTL_[A-Z0-9_]+_DECLARE", text):
        return True
    if re.search(r"\bsstl_(cmp|hash)_[a-zA-Z0-9_]+\b", text):
        return True
    return False


def check_c_header_surfaces(sstl_root: Path) -> list[str]:
    failures: list[str] = []
    c_root = sstl_root / "include" / "sstl" / "c"
    if not c_root.exists():
        return [f"c_headers_missing:{c_root}"]
    for header in sorted(c_root.glob("sstl_*.h")):
        if header.name in {"sstl_config.h", "sstl_version.h"}:
            continue
        if not c_header_has_public_surface(header):
            failures.append(f"c_header_without_public_surface:{header.relative_to(sstl_root)}")
    return failures


def run_capture(cmd: list[str], timeout: int = 20) -> str:
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True, errors="replace", timeout=timeout)
    except Exception:
        return ""
    return proc.stdout if proc.returncode == 0 else ""


def windows_program_roots() -> list[Path]:
    roots: list[Path] = []
    for key in ("ProgramFiles", "ProgramFiles(x86)", "ProgramW6432", "LocalAppData"):
        value = os.environ.get(key)
        if value:
            p = Path(value)
            if p.exists() and p not in roots:
                roots.append(p)
    return roots


def find_vswhere() -> str | None:
    direct = shutil.which("vswhere")
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


def find_named_executable(name: str, roots: list[Path] | None = None) -> str | None:
    direct = shutil.which(name)
    if direct:
        return direct
    executable_name = name + ".exe" if os.name == "nt" and not name.lower().endswith(".exe") else name
    for root in roots or []:
        likely = [
            root / executable_name,
            root / "bin" / executable_name,
            root / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "CMake" / "bin" / executable_name,
            root / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "Ninja" / executable_name,
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


def cmake_tool_env() -> tuple[str | None, str | None, dict[str, str]]:
    roots = visual_studio_install_roots() if os.name == "nt" else []
    cmake = find_named_executable("cmake", roots)
    ctest = find_named_executable("ctest", roots)
    ninja = find_named_executable("ninja", roots)
    env = dict(os.environ)
    if ninja:
        env["PATH"] = str(Path(ninja).parent) + os.pathsep + env.get("PATH", "")
    return cmake, ctest, env


def run(cmd: list[str], cwd: Path) -> int:
    resolved = list(cmd)
    env = None
    if resolved and resolved[0] == "cmake":
        cmake, _, env = cmake_tool_env()
        if not cmake:
            print("cmake executable was not found")
            return 127
        resolved[0] = cmake
    elif resolved and resolved[0] == "ctest":
        _, ctest, env = cmake_tool_env()
        if not ctest:
            print("ctest executable was not found")
            return 127
        resolved[0] = ctest
    print("+", " ".join(resolved))
    return subprocess.call(resolved, cwd=str(cwd), env=env)


def run_documentation_check(root: Path) -> list[str]:
    script = root / "scripts" / "check_docs.py"
    if not script.exists():
        return [f"missing_documentation_checker:{script}"]
    proc = subprocess.run(
        [sys.executable, str(script), "--root", str(root)],
        cwd=str(root),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
    )
    if proc.returncode == 0:
        return []
    details = [line.strip() for line in proc.stdout.splitlines() if line.strip()]
    return ["documentation_integrity_failed"] + [f"documentation_detail:{line}" for line in details]


def clear_completeness_build_dir(root: Path, build_dir: Path) -> None:
    allowed_parent = (root / "build").resolve()
    resolved_build = build_dir.resolve()
    try:
        resolved_build.relative_to(allowed_parent)
    except ValueError as exc:
        raise RuntimeError(f"refusing to clear build directory outside {allowed_parent}: {resolved_build}") from exc
    if build_dir.exists():
        shutil.rmtree(build_dir)


def write_completeness(root: Path, status: str, failed: list[str], open_assumptions: list[str]) -> None:
    out = root / "manifests" / "completeness.yaml"
    mutation_summary = "manifests/mutation_summary.yaml" if (root / "manifests" / "mutation_summary.yaml").exists() else "null"
    docker_validation = "manifests/docker_validation.yaml" if (root / "manifests" / "docker_validation.yaml").exists() else "null"
    misra_advisory_summary = "manifests/misra_advisory_summary.yaml" if (root / "manifests" / "misra_advisory_summary.yaml").exists() else "null"
    vendor_compiler_summary = "manifests/vendor_compiler_summary.yaml" if (root / "manifests" / "vendor_compiler_summary.yaml").exists() else "null"
    lines = [
        f"status: {status}",
        f"generated_at_utc: {datetime.now(timezone.utc).isoformat()}",
        "failed_checks:",
    ]
    lines += [f"  - {item}" for item in failed] or ["  []"]
    coverage_summary = "manifests/coverage_summary.yaml" if (root / "manifests" / "coverage_summary.yaml").exists() else "null"
    documentation_summary = "manifests/documentation_summary.yaml" if (root / "manifests" / "documentation_summary.yaml").exists() else "null"
    lines += [
        f"coverage_summary: {coverage_summary}",
        f"documentation_summary: {documentation_summary}",
        f"mutation_summary: {mutation_summary}",
        f"docker_validation: {docker_validation}",
        f"misra_advisory_summary: {misra_advisory_summary}",
        f"vendor_compiler_summary: {vendor_compiler_summary}",
        "open_assumptions:",
    ]
    lines += [f"  - {item}" for item in open_assumptions] or ["  []"]
    out.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="Extracted test package root")
    parser.add_argument("--sstl-root", default=os.environ.get("SSTL_ROOT", ""), help="SSTL implementation root")
    parser.add_argument("--run-cmake", action="store_true", help="Configure, build, and run host-debug preset")
    parser.add_argument("--allow-missing-coverage", action="store_true", help="Do not fail when coverage_summary.yaml is absent")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    failed: list[str] = []
    open_assumptions: list[str] = []

    req_manifest = root / "manifests" / "requirement_manifest.yaml"
    api_manifest = root / "manifests" / "api_manifest.yaml"
    test_manifest = root / "manifests" / "test_manifest.yaml"
    for required in (req_manifest, api_manifest, test_manifest):
        if not required.exists():
            failed.append(f"missing_manifest:{required}")

    if failed:
        write_completeness(root, "incomplete", failed, open_assumptions)
        return 1

    req_ids = collect_manifest_ids(req_manifest, "requirements")
    nontrace_req_ids = collect_nontrace_requirement_ids(req_manifest)
    api_ids = collect_manifest_ids(api_manifest, "apis")
    covered_reqs, covered_apis, virtual_files = collect_test_refs(test_manifest)
    manifest_entries = collect_test_manifest_entries(test_manifest)
    manifest_test_files = {normalize_manifest_path(item) for item in virtual_files}

    missing_req_coverage = sorted((req_ids - nontrace_req_ids) - covered_reqs)
    missing_api_coverage = sorted(api_ids - covered_apis)
    for req in missing_req_coverage:
        failed.append(f"requirement_without_test:{req}")
    for api in missing_api_coverage:
        failed.append(f"api_without_test:{api}")

    for req in sorted(covered_reqs - req_ids):
        failed.append(f"unknown_requirement_ref:{req}")
    for api in sorted(covered_apis - api_ids):
        failed.append(f"unknown_api_ref:{api}")

    for virtual_file in virtual_files:
        if not (root / virtual_file).exists():
            failed.append(f"missing_test_file:{virtual_file}")

    failed.extend(check_source_header_manifest_sync(root, manifest_entries))

    cmake_lists = root / "CMakeLists.txt"
    if not cmake_lists.exists():
        failed.append(f"missing_cmake_lists:{cmake_lists}")
    else:
        for cmake_test_source in sorted(collect_cmake_registered_test_sources(cmake_lists) - manifest_test_files):
            failed.append(f"cmake_test_without_manifest:{cmake_test_source}")

    sstl_root = Path(args.sstl_root).resolve() if args.sstl_root else None
    if not sstl_root:
        failed.append("sstl_root_not_supplied")
        open_assumptions.append("Local implementation root is unspecified.")
    elif not (sstl_root / "include" / "sstl").exists():
        failed.append(f"implementation_headers_missing:{sstl_root / 'include' / 'sstl'}")
    else:
        failed.extend(check_c_header_surfaces(sstl_root))

    coverage_summary = root / "manifests" / "coverage_summary.yaml"
    if not coverage_summary.exists():
        if not args.allow_missing_coverage:
            failed.append("missing_coverage_summary")
        else:
            open_assumptions.append("Coverage summary was not required for this local completeness run.")
    elif "status: pass" not in coverage_summary.read_text(encoding="utf-8", errors="replace"):
        failed.append("coverage_thresholds_not_passing")

    failed.extend(run_documentation_check(root))

    waivers = (root / "manifests" / "waivers.yaml").read_text(encoding="utf-8")
    if "status: open" in waivers:
        failed.append("open_waivers_present")
        open_assumptions.append("Target simulator and mutation lanes need implementation-specific configuration.")
    if "WAIVE-TARGET-SIM" in waivers and "status: closed" in waivers:
        docker_validation = root / "manifests" / "docker_validation.yaml"
        if not docker_validation.exists():
            failed.append("missing_docker_validation_evidence")
        elif "target_smoke: pass" not in docker_validation.read_text(encoding="utf-8"):
            failed.append("target_smoke_not_passing")
    if "WAIVE-MUTATION" in waivers and "status: closed" in waivers:
        mutation_summary = root / "manifests" / "mutation_summary.yaml"
        if not mutation_summary.exists():
            failed.append("missing_mutation_evidence")
        elif "status: pass" not in mutation_summary.read_text(encoding="utf-8"):
            failed.append("mutation_smoke_not_passing")

    if args.run_cmake and not failed:
        build_suffix = "windows" if os.name == "nt" else "posix"
        build_stamp = datetime.now(timezone.utc).strftime("%Y%m%d%H%M%S")
        build_dir = root / "build" / f"completeness-host-debug-{build_suffix}-{os.getpid()}-{build_stamp}"
        try:
            clear_completeness_build_dir(root, build_dir)
        except RuntimeError as exc:
            failed.append(f"cmake_build_dir_cleanup_refused:{exc}")
        if not failed and run(["cmake", "-S", str(root), "-B", str(build_dir), "-G", "Ninja", f"-DSSTL_ROOT={sstl_root}", "-DSSTL_ERROR_POLICY=SSTL_RETURN"], root) != 0:
            failed.append("cmake_configure_failed")
        elif not failed and run(["cmake", "--build", str(build_dir)], root) != 0:
            failed.append("cmake_build_failed")
        elif not failed and run(["ctest", "--test-dir", str(build_dir), "--output-on-failure"], root) != 0:
            failed.append("ctest_failed")

    misra_summary = root / "manifests" / "misra_advisory_summary.yaml"
    if not misra_summary.exists():
        failed.append("missing_misra_advisory_summary")
    elif "status: pass" not in misra_summary.read_text(encoding="utf-8", errors="replace"):
        failed.append("misra_advisory_scan_not_passing")

    vendor_compiler_summary = root / "manifests" / "vendor_compiler_summary.yaml"
    if not vendor_compiler_summary.exists():
        failed.append("missing_vendor_compiler_summary")
    elif "status: fail" in vendor_compiler_summary.read_text(encoding="utf-8", errors="replace"):
        failed.append("vendor_compiler_lanes_not_passing")

    status = "complete" if not failed else "incomplete"
    write_completeness(root, status, failed, open_assumptions)
    for item in failed:
        print("FAILED:", item)
    return 0 if status == "complete" else 1


if __name__ == "__main__":
    sys.exit(main())
