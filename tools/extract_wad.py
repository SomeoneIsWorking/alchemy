#!/usr/bin/env python3
"""Extract Raven's ZIP-derived WAD archives through the shared archive reader."""

import sys
from pathlib import Path

if __package__:
    from .alchemy_archives import AlchemyZip
else:
    from alchemy_archives import AlchemyZip


def extract_wad(wad_path, outdir, want_substr=None):
    with AlchemyZip(Path(wad_path)) as archive:
        return archive.extract(Path(outdir), want_substr)


if __name__ == "__main__":
    want = sys.argv[3].split(",") if len(sys.argv) > 3 else None
    n = extract_wad(sys.argv[1], sys.argv[2], want)
    print("extracted", n)
