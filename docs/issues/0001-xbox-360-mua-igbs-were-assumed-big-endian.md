---
id: 1
title: Xbox 360 MUA IGBs were assumed big-endian
status: resolved
symptom: Alchemy README says Xbox 360 IGB assets require global byte swapping, but compatibility work must determine the actual supplied MUA format
tags: mua,igb,endianness,assets
created: 2026-08-22
updated: 2026-08-22
---

## Root cause


## What was tried / dead ends


## Resolution

### Resolution (2026-08-22)
Root cause was an invalid inference from the Xbox 360 CPU byte order to the asset container byte order. The supplied MUA corpus contains little-endian version-8 IGBs on Xbox 360; all 14,187 IGB occurrences opened in the explicit four-archive gate. The real platform gap was PS2 payload formats RGBA5551 and CLUT_INDEX8/igClut, now implemented and verified by the cross-platform differential. README and codemap now separate container order from CPU/GPU payload order.
