# Project state

## Comparison baseline

The baseline is every Alchemy-engine game port carrying its own container decoders, archive tools,
asset viewers, and controller glue. This library provides one measured, reusable native owner for the
shared IGB/XMLB/ARK formats, semantic assets, inspection tools, and platform-neutral input.

## Current focus

S004 is the current focus.

## Capability inventory

| ID | Capability or outcome | State | Factual dependency | Goals |
| --- | --- | --- | --- | --- |
| S001 | IGB containers and the measured Xbox 360 and PS2 corpora can be opened and inspected | verified | — | G001 |
| S002 | Mesh, texture, raster, and animation payloads decode into native semantic data | partial | S001 | G001 |
| S003 | XMLB, FB/WAD, ARK class, font, and conversation tooling exposes reusable engine formats | partial | — | G001 |
| S004 | Platform-neutral controller snapshots and an SDL3 backend support stable native devices | partial | — | G001 |
| S005 | Existing standalone viewers and dump tools inspect measured assets without depending on a game port | verified | S001 | G001 |
| S006 | IGB meshes decode into native vertex, index, material, and skinning data | partial | S001 | G001 |
| S007 | Xbox 360 and PS2 texture/raster payloads decode into native images | partial | S001 | G001 |
| S008 | Enbaya-compressed animation payloads decode into native animation data | partial | S001 | G001 |
| S009 | XMLB assets can be decoded, edited, and round-tripped by shared tooling | partial | — | G001 |
| S010 | FB/WAD, ARK class/vtable, font, and conversation formats have reusable inspection tools | partial | — | G001 |

## Capability details

### S001 — Container coverage

Evidence: corpus checks open the measured MUA PS2, Xbox 360 base, Gold, and title-update IGB sets and
report exact archive, object, and unsupported-case counts.

### S002 — Typed asset semantics

IGB owners decode meshes, DXT and PS2 image formats, raster data, and Enbaya-compressed animation.

Gap: general big-endian structure and semantic payloads, plus complete cross-title render validation,
remain unsupported until grounded by a real corpus.

### S003 — Engine format tooling

The repository contains production XMLB, archive, ARK graph/vtable, font, WAD, and conversation tools.

Gap: format and title coverage remains evidence-driven and incomplete beyond the measured consumers.

### S004 — Native input

The generic controller owner uses stable slots and the SDL backend handles startup devices, forwarded
hot-plug events, complete snapshots, handles, and rumble.

Gap: X-Men 2 guest substitution/A-B parity and Marvel Ultimate Alliance ARK/vtable ABI integration
remain unverified.

### S005 — Asset viewers

Evidence: the existing XML2-focused `x2view`, `meshview`, `flyview`, and `igb_dump` build as standalone
consumers of the shared libraries and keep SDL2 viewer policy separate from SDL3 shipping input. This
does not claim generalized MUA or cross-title viewer coverage.

### S006 — Mesh decoding

Mesh owners expose measured geometry, material, and skinning semantics.

Gap: general big-endian structures and complete cross-title render validation remain incomplete.

### S007 — Texture and raster decoding

Texture owners decode measured DXT and PS2 image/raster formats.

Gap: format and cross-title coverage remains limited to grounded corpora.

### S008 — Animation decoding

The shared Enbaya owner decodes measured compressed animation payloads.

Gap: complete animation semantics across Alchemy titles remain unverified.

### S009 — XMLB round-trip

Shared XMLB tooling and round-trip tests cover the measured format surface.

Gap: unmeasured format variants and title coverage remain incomplete.

### S010 — Archive and auxiliary formats

The repository contains production FB/WAD, ARK graph/vtable, font, and conversation tools.

Gap: each tool's format and title coverage remains evidence-driven and incomplete.
