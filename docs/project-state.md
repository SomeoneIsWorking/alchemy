# Project state

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
