#!/usr/bin/env python3
"""Enforce Alchemy subsystem source-file ownership boundaries."""

from __future__ import annotations

import argparse
import pathlib
import re
import tempfile

SOURCE_SUFFIXES = {".c", ".h", ".py"}
DEFAULT_LIMIT = 500
LEGACY_LIMITS = {
    pathlib.Path("src/igb.c"): 800,
    pathlib.Path("src/igb_mesh.c"): 566,
}


def source_paths(root: pathlib.Path):
    for top in ("apps", "src", "tests", "tools"):
        directory = root / top
        if not directory.exists():
            continue
        for path in sorted(directory.rglob("*")):
            if "raven-formats" in path.parts:
                continue
            if path.is_file() and path.suffix in SOURCE_SUFFIXES:
                yield path


def violations(root: pathlib.Path) -> list[tuple[pathlib.Path, int, int]]:
    found = []
    for path in source_paths(root):
        relative = path.relative_to(root)
        count = len(path.read_text(encoding="utf-8").splitlines())
        limit = LEGACY_LIMITS.get(relative, DEFAULT_LIMIT)
        if count > limit:
            found.append((relative, count, limit))
    return found


def input_ownership_violations(root: pathlib.Path) -> list[str]:
    backend_path = root / "src/ig_sdl_controller.c"
    input_paths = sorted((root / "src").glob("ig_*controller.[ch]"))
    found = []

    if backend_path.exists() and "SDL_PollEvent" in backend_path.read_text(encoding="utf-8"):
        found.append(
            "src/ig_sdl_controller.c polls SDL's global event queue; "
            "the application must forward events"
        )
    for path in input_paths:
        text = path.read_text(encoding="utf-8")
        if re.search(r"\b(?:x2|X2)_", text):
            found.append(
                f"{path.relative_to(root)} exposes title-specific X2 input vocabulary"
            )
    return found


def run_selftest() -> int:
    scratch = pathlib.Path(__file__).resolve().parents[1] / "scratch"
    scratch.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="structure_", dir=scratch) as temporary:
        root = pathlib.Path(temporary)
        source = root / "src"
        source.mkdir()
        (source / "at_limit.c").write_text(
            "line\n" * DEFAULT_LIMIT, encoding="utf-8"
        )
        if violations(root):
            print("structure self-test: rejected a file exactly at the limit")
            return 1
        (source / "too_large.c").write_text(
            "line\n" * (DEFAULT_LIMIT + 1), encoding="utf-8"
        )
        expected = [
            (pathlib.Path("src/too_large.c"), DEFAULT_LIMIT + 1, DEFAULT_LIMIT)
        ]
        observed = violations(root)
        if observed != expected:
            print(f"structure self-test: expected {expected}, observed {observed}")
            return 1
        (source / "ig_sdl_controller.c").write_text(
            "void poll(void) { SDL_PollEvent(0); }\n", encoding="utf-8"
        )
        (source / "ig_controller.h").write_text(
            "void x2_controller_init(void);\n", encoding="utf-8"
        )
        ownership = input_ownership_violations(root)
        if len(ownership) != 2:
            print(
                "structure self-test: expected event-pump and title-vocabulary "
                f"violations, observed {ownership}"
            )
            return 1
    print(
        "structure self-test: accepted boundaries and detected oversized source, "
        "backend event polling, and title-specific input vocabulary"
    )
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        return run_selftest()

    root = pathlib.Path(__file__).resolve().parents[1]
    found = violations(root)
    ownership = input_ownership_violations(root)
    if found or ownership:
        for path, count, limit in found:
            print(f"structure: {path} has {count} lines; limit is {limit}")
        for message in ownership:
            print(f"structure: {message}")
        return 1
    print(
        "structure: new first-party files are at most 500 lines; "
        "legacy limits did not grow; SDL input ownership is intact"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
