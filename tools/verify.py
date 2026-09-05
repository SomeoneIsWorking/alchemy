#!/usr/bin/env python3
"""Run the locked local/CI Alchemy verifier (never a product launcher)."""

from __future__ import annotations

import argparse
from pathlib import Path

from verification import verify


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=Path("build/verify"))
    parser.add_argument("--fetch-sdl", action="store_true")
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    root = Path(__file__).resolve().parents[1]
    verify(root, (root / args.build_dir).resolve(), args.fetch_sdl, args.jobs)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
