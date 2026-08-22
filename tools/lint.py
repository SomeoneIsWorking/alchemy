#!/usr/bin/env python3
"""Run the repository's defect-oriented Ruff policy, or report a real skip."""

from __future__ import annotations

import subprocess
import shutil
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    executable = shutil.which("ruff")
    if executable is None:
        print("lint: SKIP -- Ruff is not installed; no Python was linted")
        return 77
    version = subprocess.run(
        [executable, "--version"],
        cwd=root,
        check=False,
        capture_output=True,
        text=True,
    )
    result = subprocess.run(
        [executable, "check", "tools", "tests"],
        cwd=root,
        check=False,
    )
    files = sum(1 for top in ("tools", "tests") for _ in (root / top).rglob("*.py"))
    print(f"lint: {version.stdout.strip()} checked {files} Python files")
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
