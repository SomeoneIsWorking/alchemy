---
id: C002
kind: claim
status: holds
created: 2026-08-22
tags: mua,ps2,texture
depends: src/igb_image.c, src/igb.c#igb_image_to_rgba, tests/test_igb_image_real.c
---

## Claim

MUA PS2 image formats 8 and 65536 are respectively little-endian RGBA5551 and row-major CLUT_INDEX8 with an igClut RGBA8888 palette, and decode through igb_image_to_rgba.

## Evidence

Matching real assets: CLUT8 direct layout 40,656/65,536 exact pixels and RGBA SAD 165,402 versus GS unswizzle 27,703 exact and SAD 16,147,490; RGBA5551 comparison SAD 5,760/1,024 RGBA bytes; 65,792/65,792 pixels compared in the production-path test

## What would falsify it

A matching platform source disproves the bit/palette layout, a different layout yields a lower evidenced error, or the image parser/decoder/test changes without rerunning the real differential
