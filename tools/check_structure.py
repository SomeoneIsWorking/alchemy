#!/usr/bin/env python3
"""Enforce Alchemy subsystem source-file ownership boundaries."""

from __future__ import annotations

import pathlib
import re

SOURCE_SUFFIXES = {".c", ".cpp", ".h", ".hpp", ".py"}
DEFAULT_LIMIT = 500
LEGACY_LIMITS = {
    pathlib.Path("src/igb.c"): 775,
    pathlib.Path("src/igb_mesh.c"): 538,
}


def source_paths(root: pathlib.Path):
    for top in ("apps", "include", "src", "tests", "tools"):
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
    backend_path = root / "src/input/sdl_controller.cpp"
    found = []

    if backend_path.exists() and "SDL_PollEvent" in backend_path.read_text(encoding="utf-8"):
        found.append(
            "src/input/sdl_controller.cpp polls SDL's global event queue; "
            "the application must forward events"
        )
    return found


def shipping_policy_violations(root: pathlib.Path) -> list[str]:
    paths = [
        *sorted((root / "src").glob("*.[ch]")),
        *sorted((root / "src").glob("*.cpp")),
        *sorted((root / "src/input").glob("*.cpp")),
        *sorted((root / "include/alchemy").rglob("*.hpp")),
    ]
    found = []
    for path in paths:
        text = path.read_text(encoding="utf-8")
        if re.search(r"\bgetenv\s*\(", text):
            found.append(f"{path.relative_to(root)} reads process configuration")
        if re.search(r"\b(?:fprintf|printf|puts|fputs)\s*\(|\bstderr\b|\bstd::cerr\b", text):
            found.append(f"{path.relative_to(root)} writes diagnostics directly")
        if re.search(r"\b(?:x2|X2|mua|MUA)[_A-Za-z0-9]*", text):
            found.append(f"{path.relative_to(root)} contains title-specific vocabulary")
        if re.search(r"(?:pc[/\\]xmen2|x360[/\\]mua)", text, re.IGNORECASE):
            found.append(f"{path.relative_to(root)} depends on a consuming title")
        is_platform_adapter = any(part in {"x86", "x360"} for part in path.parts)
        if not is_platform_adapter and re.search(
            r"\b(?:x86port|xenonport|x360port)\b", text, re.IGNORECASE
        ):
            found.append(f"{path.relative_to(root)} couples the neutral core to a CPU host")
    return found


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[1]
    found = violations(root)
    ownership = input_ownership_violations(root)
    shipping = shipping_policy_violations(root)
    if found or ownership or shipping:
        for path, count, limit in found:
            print(f"structure: {path} has {count} lines; limit is {limit}")
        for message in ownership:
            print(f"structure: {message}")
        for message in shipping:
            print(f"structure: {message}")
        return 1
    print(
        "structure: new first-party files are at most 500 lines; "
        "legacy limits did not grow; typed input and shipping policy are intact"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
