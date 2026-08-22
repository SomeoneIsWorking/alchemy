---
id: C001
kind: claim
status: holds
created: 2026-08-22
tags: mua,corpus,archive
depends: tools/mua_corpus.py, tools/alchemy_archives.py, tools/xmlb.py, src/igb.c
---

## Claim

The supplied MUA PS2 and Xbox 360 Gold archive corpus traverses 4/4 required archives, parses 1,087/1,087 FB packages, round-trips 14,348/14,348 XMLB payloads byte-identically, and opens 14,187/14,187 IGB occurrences with zero parser failures.

## Evidence

2026-08-22 explicit tools/mua_corpus.py run from both supplied ISOs: 2,949 archive entries, 40,158 FB entries, 283,057 XMLB nodes, 7,152,367 IGB objects, PASS

## What would falsify it

A supplied disc image changes, any required denominator changes, a parser failure appears, or the archive/FB/XMLB/IGB code named below changes without rerunning the explicit-disc gate
