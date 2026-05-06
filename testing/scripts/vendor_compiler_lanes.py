#!/usr/bin/env python3
# test_id: package.vendor.compiler_lanes
# api_ids: [package.vendor.compiler_lanes]
# req_ids: [REQ-01, REQ-06]
# patterns: [VENDOR-COMPILER-LANES, FREESTANDING-COMPILE]
"""Best-effort vendor/cross compiler compile probes for SSTL public headers."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys


CONTAINER_IMAGE_ENV = {
    "armclang": "SSTL_VENDOR_CONTAINER_ARMCLANG_IMAGE",
    "arm-none-eabi": "SSTL_VENDOR_CONTAINER_ARM_NONE_EABI_IMAGE",
    "keil-armcc": "SSTL_VENDOR_CONTAINER_KEIL_ARMCC_IMAGE",
    "iar-iccarm": "SSTL_VENDOR_CONTAINER_IAR_ICCARM_IMAGE",
}


@dataclass
class Lane:
    name: str
    compiler: str
    language: str
    source: Path
    output: Path
    command: list[str] | None
    skip_reason: str
    note: str


@dataclass(frozen=True)
class ContainerImageSpec:
    key: str
    label: str
    image: str


def normalize(path: Path) -> Path:
    try:
        return path.resolve()
    except OSError:
        return path.absolute()


def yaml_scalar(value: str) -> str:
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def path_or_none(name: str) -> str | None:
    return shutil.which(name)


def quote_sh(value: str) -> str:
    return "'" + value.replace("'", "'\"'\"'") + "'"


def normalized_image_key(value: str) -> str:
    key = value.strip().lower().replace("_", "-")
    aliases = {
        "arm-none-eabi-gcc": "arm-none-eabi",
        "arm-none-eabi-g++": "arm-none-eabi",
        "gnu-arm-embedded": "arm-none-eabi",
        "armcc": "keil-armcc",
        "keil": "keil-armcc",
        "iar": "iar-iccarm",
        "iccarm": "iar-iccarm",
    }
    return aliases.get(key, key)


def normalized_image_label(value: str) -> str:
    label = re.sub(r"[^A-Za-z0-9_]+", "_", value.strip())
    label = re.sub(r"_+", "_", label).strip("_").lower()
    return label or "image"


def split_image_key_and_label(value: str) -> tuple[str, str]:
    raw_key, has_label, raw_label = value.partition("@")
    key = normalized_image_key(raw_key)
    label = normalized_image_label(raw_label) if has_label else key
    return key, label


def docker_image_config(overrides: list[str]) -> list[ContainerImageSpec]:
    images: dict[str, ContainerImageSpec] = {}
    for key, env_name in CONTAINER_IMAGE_ENV.items():
        value = os.environ.get(env_name, "").strip()
        if value:
            images[key] = ContainerImageSpec(key, key, value)
    for item in overrides:
        if "=" not in item:
            raise ValueError("--container-image must be KEY[@LABEL]=IMAGE")
        raw_key, image = item.split("=", 1)
        key, label = split_image_key_and_label(raw_key)
        if key not in CONTAINER_IMAGE_ENV:
            raise ValueError("unsupported vendor container image key: " + raw_key)
        images[label] = ContainerImageSpec(key, label, image.strip())
    return list(images.values())


def image_env_label(key: str) -> str:
    return CONTAINER_IMAGE_ENV[key]


def docker_engine_path(engine: str) -> str | None:
    return shutil.which(engine) or shutil.which(engine + ".exe") or (engine if Path(engine).exists() else None)


def docker_image_present(engine: str, image: str) -> bool:
    try:
        proc = subprocess.run(
            [engine, "image", "inspect", image],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=30,
        )
    except Exception:
        return False
    return proc.returncode == 0


def ask_to_pull_image(image: str) -> bool:
    if not sys.stdin.isatty():
        return False
    print("Container image is not present locally: " + image, flush=True)
    print("Pull this image for vendor compiler lanes? Type 'yes' to pull, anything else to skip.", flush=True)
    try:
        answer = input("> ").strip().lower()
    except EOFError:
        return False
    return answer == "yes"


def pull_docker_image(engine: str, image: str, timeout: int) -> tuple[bool, str]:
    try:
        proc = subprocess.run(
            [engine, "pull", image],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return False, "container image pull timed out: " + image
    except Exception as exc:
        return False, "container image pull failed to start for " + image + ": " + str(exc)
    if proc.returncode == 0:
        return True, "container image pulled: " + image
    output = "\n".join(proc.stdout.splitlines()[-8:])
    return False, "container image pull failed for " + image + ": " + output


def ensure_docker_images(engine: str, images: list[ContainerImageSpec], pull_policy: str, timeout: int) -> dict[str, str]:
    results: dict[str, str] = {}
    checked: dict[str, str] = {}
    for spec in sorted(images, key=lambda item: (item.key, item.label)):
        image = spec.image
        if not image:
            results[spec.label] = "container image reference is empty for " + spec.key
            continue
        if image in checked:
            results[spec.label] = checked[image]
            continue
        if docker_image_present(engine, image):
            checked[image] = "present"
            results[spec.label] = "present"
            continue
        if pull_policy == "never":
            checked[image] = "container image not present locally and pulling disabled: " + image
            results[spec.label] = checked[image]
            continue
        if pull_policy == "ask" and not ask_to_pull_image(image):
            if sys.stdin.isatty():
                checked[image] = "user declined container image pull: " + image
            else:
                checked[image] = "container image not present locally and pull approval unavailable: " + image
            results[spec.label] = checked[image]
            continue
        ok, detail = pull_docker_image(engine, image, timeout)
        checked[image] = "present" if ok else detail
        results[spec.label] = checked[image]
    return results


def docker_probe_command(engine: str, image: str, root: Path, args: list[str]) -> list[str]:
    # The engine needs a concrete host bind source; command_text() renders it
    # relative in the summary manifest.
    return [
        engine,
        "run",
        "--rm",
        "-v",
        str(root) + ":/work:ro",
        "-w",
        "/work",
        image,
    ] + args


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


def cmd_with_vcvars(vcvars: str, tool_args: list[str], batch_path: Path) -> list[str]:
    # cmd.exe requires the batch file and the following tool invocation to live
    # in one /c string so the environment changes remain active for cl.exe.
    lines = [
        "@echo off",
        "call " + subprocess.list2cmdline([vcvars]) + " >nul",
        "if errorlevel 1 exit /b %errorlevel%",
        subprocess.list2cmdline(tool_args),
    ]
    batch_path.write_text("\r\n".join(lines) + "\r\n", encoding="ascii")
    return ["cmd.exe", "/d", "/c", str(batch_path)]


def msvc_command(args: list[str], batch_path: Path) -> tuple[list[str] | None, str, str]:
    cl = path_or_none("cl")
    if cl:
        return [cl] + args, "cl", "MSVC cl discovered on PATH"
    vcvars = find_vcvars64()
    if vcvars:
        return cmd_with_vcvars(vcvars, ["cl"] + args, batch_path), "cl", "MSVC cl discovered through vcvars64.bat"
    return None, "cl", "cl.exe or Visual Studio C++ build tools not found"


def make_lanes(root: Path, sstl_root: Path) -> list[Lane]:
    build = root / "build" / "vendor-compiler-probes"
    build.mkdir(parents=True, exist_ok=True)

    cpp_public = root / "tests" / "cpp03" / "compile_probe" / "sstl_cxx03_probe.cpp"
    c_public = root / "tests" / "c99" / "compile_probe" / "sstl_c99_probe.c"
    cpp_free = root / "tests" / "freestanding" / "freestanding_cpp03_probe.cpp"
    c_free = root / "tests" / "freestanding" / "freestanding_c99_probe.c"
    include = str(sstl_root / "include")
    common = str(root / "test_support" / "common")

    lanes: list[Lane] = []

    msvc_cpp_args = [
        "/nologo",
        "/EHsc",
        "/D",
        "SSTL_ON_ERROR=SSTL_RETURN",
        "/D",
        "SSTL_TEST_ITERATOR_DEBUG=1",
        "/I",
        include,
        "/I",
        common,
        "/c",
        str(cpp_public),
        "/Fo" + str(build / "msvc_cpp_public.obj"),
    ]
    cmd, compiler, note = msvc_command(msvc_cpp_args, build / "msvc_cpp_public.cmd")
    lanes.append(Lane("msvc_cpp_public", compiler, "cpp", cpp_public, build / "msvc_cpp_public.obj", cmd, "" if cmd else note, note))

    msvc_c_args = [
        "/nologo",
        "/TC",
        "/std:c11",
        "/D",
        "SSTL_ON_ERROR=SSTL_RETURN",
        "/I",
        include,
        "/I",
        common,
        "/c",
        str(c_public),
        "/Fo" + str(build / "msvc_c_public.obj"),
    ]
    cmd, compiler, note = msvc_command(msvc_c_args, build / "msvc_c_public.cmd")
    lanes.append(Lane("msvc_c_public", compiler, "c", c_public, build / "msvc_c_public.obj", cmd, "" if cmd else note, note))

    armclang = path_or_none("armclang")
    armclang_skip = "" if armclang else "armclang not found"
    common_armclang = ["--target=arm-arm-none-eabi", "-mcpu=cortex-m3", "-ffreestanding", "-fsyntax-only", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include]
    lanes.append(Lane(
        "armclang_cpp_freestanding",
        "armclang",
        "cpp",
        cpp_free,
        build / "armclang_cpp_freestanding.o",
        ([armclang] + common_armclang + ["-std=c++03", "-fno-exceptions", "-fno-rtti", str(cpp_free)]) if armclang else None,
        armclang_skip,
        "ArmClang Cortex-M syntax-only C++03 freestanding probe",
    ))
    lanes.append(Lane(
        "armclang_c_freestanding",
        "armclang",
        "c",
        c_free,
        build / "armclang_c_freestanding.o",
        ([armclang] + common_armclang + ["-std=c99", str(c_free)]) if armclang else None,
        armclang_skip,
        "ArmClang Cortex-M syntax-only C99 freestanding probe",
    ))

    arm_gpp = path_or_none("arm-none-eabi-g++")
    arm_gcc = path_or_none("arm-none-eabi-gcc")
    lanes.append(Lane(
        "arm_none_eabi_gcc_cpp_freestanding",
        "arm-none-eabi-g++",
        "cpp",
        cpp_free,
        build / "arm_none_eabi_cpp_freestanding.o",
        ([arm_gpp, "-std=c++03", "-ffreestanding", "-fno-exceptions", "-fno-rtti", "-fsyntax-only", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, str(cpp_free)]) if arm_gpp else None,
        "" if arm_gpp else "arm-none-eabi-g++ not found",
        "GNU Arm Embedded C++03 freestanding syntax probe",
    ))
    lanes.append(Lane(
        "arm_none_eabi_gcc_c_freestanding",
        "arm-none-eabi-gcc",
        "c",
        c_free,
        build / "arm_none_eabi_c_freestanding.o",
        ([arm_gcc, "-std=c99", "-ffreestanding", "-fsyntax-only", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, str(c_free)]) if arm_gcc else None,
        "" if arm_gcc else "arm-none-eabi-gcc not found",
        "GNU Arm Embedded C99 freestanding syntax probe",
    ))

    armcc = path_or_none("armcc")
    lanes.append(Lane(
        "keil_armcc_cpp_freestanding",
        "armcc",
        "cpp",
        cpp_free,
        build / "keil_armcc_cpp_freestanding.o",
        ([armcc, "--cpp", "--cpu=Cortex-M3", "--no_exceptions", "--no_rtti", "-c", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, str(cpp_free), "-o", str(build / "keil_armcc_cpp_freestanding.o")]) if armcc else None,
        "" if armcc else "Keil armcc not found",
        "Keil armcc C++ freestanding compile probe",
    ))
    lanes.append(Lane(
        "keil_armcc_c_freestanding",
        "armcc",
        "c",
        c_free,
        build / "keil_armcc_c_freestanding.o",
        ([armcc, "--c99", "--cpu=Cortex-M3", "-c", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, str(c_free), "-o", str(build / "keil_armcc_c_freestanding.o")]) if armcc else None,
        "" if armcc else "Keil armcc not found",
        "Keil armcc C99 freestanding compile probe",
    ))

    iccarm = path_or_none("iccarm")
    lanes.append(Lane(
        "iar_iccarm_cpp_freestanding",
        "iccarm",
        "cpp",
        cpp_free,
        build / "iar_iccarm_cpp_freestanding.o",
        ([iccarm, "--silent", "--no_wrap_diagnostics", "--cpu=Cortex-M3", "--c++", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, str(cpp_free), "-o", str(build / "iar_iccarm_cpp_freestanding.o")]) if iccarm else None,
        "" if iccarm else "IAR iccarm not found",
        "IAR ARM C++ freestanding compile probe",
    ))
    lanes.append(Lane(
        "iar_iccarm_c_freestanding",
        "iccarm",
        "c",
        c_free,
        build / "iar_iccarm_c_freestanding.o",
        ([iccarm, "--silent", "--no_wrap_diagnostics", "--cpu=Cortex-M3", "--c99", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, str(c_free), "-o", str(build / "iar_iccarm_c_freestanding.o")]) if iccarm else None,
        "" if iccarm else "IAR iccarm not found",
        "IAR ARM C99 freestanding compile probe",
    ))

    return lanes


def lane_name_for_image(base_name: str, spec: ContainerImageSpec) -> str:
    if spec.label == spec.key:
        return base_name
    return base_name + "_" + normalized_image_label(spec.label)


def make_docker_lanes(root: Path, images: list[ContainerImageSpec], engine_name: str, pull_policy: str, pull_timeout: int) -> list[Lane]:
    build = root / "build" / "vendor-compiler-probes"
    build.mkdir(parents=True, exist_ok=True)

    cpp_free = root / "tests" / "freestanding" / "freestanding_cpp03_probe.cpp"
    c_free = root / "tests" / "freestanding" / "freestanding_c99_probe.c"
    cpp_in_container = "/work/testing/tests/freestanding/freestanding_cpp03_probe.cpp"
    c_in_container = "/work/testing/tests/freestanding/freestanding_c99_probe.c"
    include = "/work/include"
    engine = docker_engine_path(engine_name)
    image_status = ensure_docker_images(engine, images, pull_policy, pull_timeout) if engine else {}
    images_by_key: dict[str, list[ContainerImageSpec]] = {}
    for spec in images:
        images_by_key.setdefault(spec.key, []).append(spec)
    lanes: list[Lane] = []

    def configured_lane(
        key: str,
        lane_name: str,
        compiler: str,
        language: str,
        source: Path,
        args: list[str],
        note: str,
    ) -> None:
        specs = images_by_key.get(key, [])
        if not specs:
            lanes.append(Lane(lane_name, compiler, language, source, build / (lane_name + ".container"), None, image_env_label(key) + " not set", note))
            return
        for spec in specs:
            image = spec.image
            image_note = note + " using image " + image + " [" + spec.label + "]"
            image_lane_name = lane_name_for_image(lane_name, spec)
            if not image:
                lanes.append(Lane(image_lane_name, compiler, language, source, build / (image_lane_name + ".container"), None, "container image reference is empty for " + spec.key, image_note))
                continue
            if not engine:
                lanes.append(Lane(image_lane_name, compiler, language, source, build / (image_lane_name + ".container"), None, engine_name + " executable not found", image_note))
                continue
            status = image_status.get(spec.label, "container image not prepared: " + image)
            if status != "present":
                lanes.append(Lane(image_lane_name, compiler, language, source, build / (image_lane_name + ".container"), None, status, image_note))
                continue
            lanes.append(Lane(image_lane_name, compiler, language, source, build / (image_lane_name + ".container"), docker_probe_command(engine, image, root.parent, args), "", image_note))

    armclang_common = ["--target=arm-arm-none-eabi", "-mcpu=cortex-m3", "-ffreestanding", "-fsyntax-only", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include]
    configured_lane(
        "armclang",
        "container_armclang_cpp_freestanding",
        "armclang",
        "cpp",
        cpp_free,
        ["armclang"] + armclang_common + ["-std=c++03", "-fno-exceptions", "-fno-rtti", cpp_in_container],
        "Docker ArmClang Cortex-M syntax-only C++03 freestanding probe",
    )
    configured_lane(
        "armclang",
        "container_armclang_c_freestanding",
        "armclang",
        "c",
        c_free,
        ["armclang"] + armclang_common + ["-std=c99", c_in_container],
        "Docker ArmClang Cortex-M syntax-only C99 freestanding probe",
    )

    configured_lane(
        "arm-none-eabi",
        "container_arm_none_eabi_gcc_cpp_freestanding",
        "arm-none-eabi-g++",
        "cpp",
        cpp_free,
        ["arm-none-eabi-g++", "-std=c++03", "-ffreestanding", "-fno-exceptions", "-fno-rtti", "-fsyntax-only", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, cpp_in_container],
        "Docker GNU Arm Embedded C++03 freestanding syntax probe",
    )
    configured_lane(
        "arm-none-eabi",
        "container_arm_none_eabi_gcc_c_freestanding",
        "arm-none-eabi-gcc",
        "c",
        c_free,
        ["arm-none-eabi-gcc", "-std=c99", "-ffreestanding", "-fsyntax-only", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, c_in_container],
        "Docker GNU Arm Embedded C99 freestanding syntax probe",
    )

    configured_lane(
        "keil-armcc",
        "container_keil_armcc_cpp_freestanding",
        "armcc",
        "cpp",
        cpp_free,
        ["armcc", "--cpp", "--cpu=Cortex-M3", "--no_exceptions", "--no_rtti", "-c", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, cpp_in_container, "-o", "/tmp/sstl_keil_armcc_cpp.o"],
        "Docker Keil armcc C++ freestanding compile probe",
    )
    configured_lane(
        "keil-armcc",
        "container_keil_armcc_c_freestanding",
        "armcc",
        "c",
        c_free,
        ["armcc", "--c99", "--cpu=Cortex-M3", "-c", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, c_in_container, "-o", "/tmp/sstl_keil_armcc_c.o"],
        "Docker Keil armcc C99 freestanding compile probe",
    )

    configured_lane(
        "iar-iccarm",
        "container_iar_iccarm_cpp_freestanding",
        "iccarm",
        "cpp",
        cpp_free,
        ["iccarm", "--silent", "--no_wrap_diagnostics", "--cpu=Cortex-M3", "--c++", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, cpp_in_container, "-o", "/tmp/sstl_iar_iccarm_cpp.o"],
        "Docker IAR ARM C++ freestanding compile probe",
    )
    configured_lane(
        "iar-iccarm",
        "container_iar_iccarm_c_freestanding",
        "iccarm",
        "c",
        c_free,
        ["iccarm", "--silent", "--no_wrap_diagnostics", "--cpu=Cortex-M3", "--c99", "-DSSTL_ON_ERROR=SSTL_RETURN", "-I" + include, c_in_container, "-o", "/tmp/sstl_iar_iccarm_c.o"],
        "Docker IAR ARM C99 freestanding compile probe",
    )

    return lanes


def display_command_arg(arg: str, root: Path, sstl_root: Path) -> str:
    for base in (root, sstl_root):
        try:
            base_text = str(base.resolve())
        except OSError:
            base_text = str(base)
        alt_base = base_text.replace("\\", "/")
        for prefix in (base_text, alt_base):
            for suffix in (":/work", ":/work:ro"):
                if arg == prefix + suffix:
                    rel = os.path.relpath(base_text, root)
                    return ("." if rel == "." else rel.replace("\\", "/")) + suffix
            if arg == prefix:
                rel = os.path.relpath(base_text, root)
                return "." if rel == "." else rel.replace("\\", "/")
            if arg.startswith(prefix + os.sep):
                rel = os.path.relpath(arg, root)
                return rel.replace("\\", "/")
            if arg.startswith(alt_base + "/"):
                rel = os.path.relpath(arg.replace("/", os.sep), root)
                return rel.replace("\\", "/")
            if prefix in arg:
                rel = os.path.relpath(base_text, root).replace("\\", "/")
                label = "." if rel == "." else rel
                return arg.replace(prefix, label).replace("\\", "/")
    return arg


def command_text(command: list[str], root: Path, sstl_root: Path) -> str:
    displayed = [display_command_arg(arg, root, sstl_root) for arg in command]
    if os.name == "nt":
        return subprocess.list2cmdline(displayed)
    return " ".join(displayed)


def run_lane(lane: Lane, root: Path, sstl_root: Path, timeout: int) -> dict[str, str]:
    if lane.command is None:
        return {
            "name": lane.name,
            "compiler": lane.compiler,
            "language": lane.language,
            "source": str(lane.source.relative_to(root)).replace("\\", "/"),
            "status": "skipped",
            "detail": lane.skip_reason,
            "note": lane.note,
        }
    proc = subprocess.run(
        lane.command,
        cwd=str(root),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
        timeout=timeout,
    )
    output = "\n".join(proc.stdout.splitlines()[-20:])
    return {
        "name": lane.name,
        "compiler": lane.compiler,
        "language": lane.language,
        "source": str(lane.source.relative_to(root)).replace("\\", "/"),
        "status": "pass" if proc.returncode == 0 else "fail",
        "exit_code": str(proc.returncode),
        "command": command_text(lane.command, root, sstl_root),
        "detail": output,
        "note": lane.note,
    }


def write_summary(path: Path, results: list[dict[str, str]]) -> str:
    failed = sum(1 for item in results if item["status"] == "fail")
    passed = sum(1 for item in results if item["status"] == "pass")
    skipped = sum(1 for item in results if item["status"] == "skipped")
    status = "fail" if failed else "pass"
    lines = [
        f"status: {status}",
        f"generated_at_utc: {datetime.now(timezone.utc).isoformat()}",
        "scope: best-effort-installed-vendor-and-cross-compilers",
        f"lanes_total: {len(results)}",
        f"lanes_passed: {passed}",
        f"lanes_skipped: {skipped}",
        f"lanes_failed: {failed}",
        "lanes:",
    ]
    for item in results:
        lines.append("  - name: " + yaml_scalar(item["name"]))
        lines.append("    compiler: " + yaml_scalar(item["compiler"]))
        lines.append("    language: " + yaml_scalar(item["language"]))
        lines.append("    source: " + yaml_scalar(item["source"]))
        lines.append("    status: " + item["status"])
        if "exit_code" in item:
            lines.append("    exit_code: " + item["exit_code"])
        lines.append("    note: " + yaml_scalar(item.get("note", "")))
        detail = item.get("detail", "")
        if detail:
            lines.append("    detail: " + yaml_scalar(detail))
        command = item.get("command", "")
        if command:
            lines.append("    command: " + yaml_scalar(command))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return status


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="Test package root")
    parser.add_argument("--sstl-root", default="", help="SSTL implementation root")
    parser.add_argument("--output", default="", help="Summary manifest output")
    parser.add_argument("--timeout", type=int, default=120, help="Per-lane timeout in seconds")
    parser.add_argument("--enable-container-images", dest="enable_docker_images", action="store_true", help="Also run configured specialized vendor container image probes")
    parser.add_argument("--container-engine", dest="docker_engine", default=os.environ.get("SSTL_VENDOR_CONTAINER_ENGINE", "docker"), help="Docker-compatible engine for specialized image probes, such as docker or podman")
    parser.add_argument("--container-image", dest="docker_image", action="append", default=[], metavar="KEY[@LABEL]=IMAGE", help="Vendor container image mapping; keys: armclang, arm-none-eabi, keil-armcc, iar-iccarm")
    parser.add_argument("--container-pull", dest="docker_pull", choices=["ask", "never", "always"], default=os.environ.get("SSTL_VENDOR_CONTAINER_PULL", "ask"), help="Policy for missing configured vendor container images")
    parser.add_argument("--container-pull-timeout", dest="docker_pull_timeout", type=int, default=600, help="Per-image container pull timeout in seconds")
    args = parser.parse_args()

    root = normalize(Path(args.root))
    sstl_root = normalize(Path(args.sstl_root)) if args.sstl_root else normalize(root.parent)
    output = normalize(Path(args.output)) if args.output else root / "manifests" / "vendor_compiler_summary.yaml"

    try:
        docker_images = docker_image_config(args.docker_image)
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 2

    lanes = make_lanes(root, sstl_root)
    if args.enable_docker_images:
        lanes.extend(make_docker_lanes(root, docker_images, args.docker_engine, args.docker_pull, args.docker_pull_timeout))
    results = [run_lane(lane, root, sstl_root, args.timeout) for lane in lanes]
    status = write_summary(output, results)
    for item in results:
        print(f"{item['name']}: {item['status']} - {item.get('detail') or item.get('note')}")
    return 0 if status == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
