"""Portable Alchemy build and executed-test contract shared by local and hosted runs."""

from __future__ import annotations

import platform
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

REQUIRED_TESTS = {
    "file_io",
    "enbaya",
    "igb_image",
    "scene_options",
    "viewer_config",
    "input",
    "sdl_input",
    "xmlb_roundtrip",
    "mua_corpus_selftest",
    "structure",
    "structure_selftest",
    "python_lint",
    "cpp_format",
    "cpp_tidy",
    "verification_policy",
}


def compiler_pair(system: str) -> tuple[str, str]:
    if system == "Windows":
        return "clang-cl", "clang-cl"
    if system == "Darwin":
        return "/usr/bin/clang", "/usr/bin/clang++"
    if system == "Linux":
        return "clang", "clang++"
    raise ValueError(f"No Alchemy desktop verification contract for {system}")


def check_results(report: Path) -> int:
    cases = ET.parse(report).getroot().findall(".//testcase")
    observed = {case.attrib["name"] for case in cases}
    missing = REQUIRED_TESTS - observed
    if missing:
        raise ValueError(f"CTest omitted required tests: {', '.join(sorted(missing))}")
    for case in cases:
        name = case.attrib["name"]
        if case.find("failure") is not None or case.find("error") is not None:
            raise ValueError(f"CTest failure: {name}")
        if case.find("skipped") is not None and name not in {"igb_image_real", "enbaya_real"}:
            raise ValueError(f"Required Alchemy test was skipped: {name}")
    return len(cases)


def verify(root: Path, build: Path, fetch_sdl: bool, jobs: int) -> None:
    if not build.is_relative_to(root / "build") or build == root / "build":
        raise ValueError("Verification build must be a child of the repository build directory")
    cc, cxx = compiler_pair(platform.system())
    for name in (cc, cxx, "cmake", "ninja", "clang-format", "clang-tidy", "ruff"):
        if shutil.which(name) is None:
            raise FileNotFoundError(f"Alchemy verification requires {name}")
    command = [
        "cmake",
        "-S",
        str(root),
        "-B",
        str(build),
        "-G",
        "Ninja",
        "-DCMAKE_BUILD_TYPE=Debug",
        f"-DCMAKE_C_COMPILER={cc}",
        f"-DCMAKE_CXX_COMPILER={cxx}",
        f"-DPython3_EXECUTABLE={sys.executable}",
        "-DALCHEMY_REQUIRE_SDL=ON",
        f"-DALCHEMY_FETCH_SDL={'ON' if fetch_sdl else 'OFF'}",
    ]
    if platform.system() == "Darwin":
        sdk = subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()
        command.append(f"-DCMAKE_OSX_SYSROOT={sdk}")
    subprocess.run(command, cwd=root, check=True)
    subprocess.run(["cmake", "--build", str(build), "--parallel", str(jobs)], check=True)
    report = build / "ctest-results.xml"
    subprocess.run(
        [
            "ctest",
            "--test-dir",
            str(build),
            "--output-on-failure",
            "--output-junit",
            str(report),
        ],
        cwd=root,
        check=True,
    )
    count = check_results(report)
    print(f"Alchemy verification: {count} test records checked; only real-asset corpus may skip")
