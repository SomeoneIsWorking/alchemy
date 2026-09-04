#!/usr/bin/env python3
"""Format and lint the C++ Alchemy runtime surface."""

from __future__ import annotations

import argparse
import json
import pathlib
import shutil
import subprocess
import sys

CPP_SUFFIXES = {".cpp", ".hpp"}
RUNTIME_ROOTS = (
    pathlib.Path("include/alchemy/input"),
    pathlib.Path("src/input"),
)
TEST_FILES = (
    pathlib.Path("tests/test_input.cpp"),
    pathlib.Path("tests/test_sdl_input.cpp"),
)


def cpp_paths(root: pathlib.Path) -> list[pathlib.Path]:
    paths = [
        path
        for relative in RUNTIME_ROOTS
        for path in sorted((root / relative).rglob("*"))
        if path.is_file() and path.suffix in CPP_SUFFIXES
    ]
    paths.extend(root / relative for relative in TEST_FILES if (root / relative).is_file())
    return paths


def require_tool(name: str) -> str | None:
    executable = shutil.which(name)
    if executable is None:
        print(f"C++ quality: {name} is unavailable; install it to run this check")
    return executable


def run_format(root: pathlib.Path) -> int:
    executable = require_tool("clang-format")
    if executable is None:
        return 77
    paths = cpp_paths(root)
    completed = subprocess.run(
        [executable, "--dry-run", "--Werror", *map(str, paths)], check=False
    )
    if completed.returncode == 0:
        print(f"C++ format: {len(paths)} input runtime/test files passed")
    return completed.returncode


def run_tidy(root: pathlib.Path, build_dir: pathlib.Path) -> int:
    executable = require_tool("clang-tidy")
    if executable is None:
        return 77
    compile_commands = build_dir / "compile_commands.json"
    if not compile_commands.is_file():
        print(f"C++ tidy: compile database is missing: {compile_commands}")
        return 1
    compile_entries = json.loads(compile_commands.read_text(encoding="utf-8"))
    compiled_files = {
        pathlib.Path(entry["file"]).resolve()
        for entry in compile_entries
        if isinstance(entry, dict) and isinstance(entry.get("file"), str)
    }
    translation_units = [
        path
        for path in cpp_paths(root)
        if path.suffix == ".cpp" and path.resolve() in compiled_files
    ]
    if not translation_units:
        print("C++ tidy: compile database contains no Alchemy input translation units")
        return 1
    completed = subprocess.run(
        [executable, "-p", str(build_dir), "--quiet", *map(str, translation_units)],
        check=False,
    )
    if completed.returncode == 0:
        print(f"C++ tidy: {len(translation_units)} input runtime/test units passed")
    return completed.returncode


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser()
    parser.add_argument("check", choices=("format", "tidy"))
    parser.add_argument("--build-dir", type=pathlib.Path, default=root / "build")
    args = parser.parse_args()
    if args.check == "format":
        return run_format(root)
    return run_tidy(root, args.build_dir.resolve())


if __name__ == "__main__":
    sys.exit(main())
