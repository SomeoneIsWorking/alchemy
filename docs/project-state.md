# Project state

## Comparison baseline

The baseline is every Alchemy-engine game port carrying its own container decoders, archive tools,
asset viewers, controller glue, and eventually title-local native engine replacements. This
repository already provides measured shared libraries and tools, but no gameplay product currently
links and exercises them as its engine boundary.

## Current focus

S011 is the current focus.

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
| S011 | X-Men 2 gameplay links and executes a conformed shared Alchemy contract | missing | S004 | G001 |
| S012 | MUA gameplay links and executes proven shared Alchemy contracts | missing | S011 | G001 |
| S013 | Shipping library configuration, diagnostics, language, and dependency boundaries are mechanically enforced | verified | — | G002 |
| S014 | Each consumer resolves one immutable Alchemy revision for tools and runtime targets | partial | — | G001, G002 |

## Host CI support

The workflow uses complete-history checkouts and synthetic/asset-free tests;
the real-disc corpus test remains explicitly skipped when no user corpus is
provided:

| Host | State | Evidence or exact gap |
| --- | --- | --- |
| Linux x86-64 | supported | `.github/workflows/ci.yml` installs SDL2/SDL3, builds viewers and both input paths with Clang, and runs the complete CTest graph. |
| macOS arm64 | supported | `.github/workflows/ci.yml` installs SDL2/SDL3, builds viewers and both input paths with Apple Clang, and runs the complete CTest graph. |
| Windows x86-64 | unsupported | The current top-level CMake graph unconditionally links the POSIX `m` library into the shared C and viewer targets and has no Windows replacement; a Windows job would fail at link time before exercising Alchemy. Portability must be fixed at that owner before adding a Windows job. |
| Android arm64-v8a | missing | Alchemy is intended for MUA's Android-capable shared engine path, but no NDK library build or consuming title package/device discriminator exists yet. Android package/device mechanics belong to `shared/android-port`; configure-only evidence is intentionally absent. |

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

The C++20 `ControllerManager` owns typed state, stable slots and value-owning lifecycle events. The
separate RAII SDL backend handles startup devices, forwarded hot-plug events, complete snapshots,
handles, rumble, immutable thresholds, and typed diagnostic delivery. A non-SDL producer can publish
external snapshots through the same manager target.

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

### S011 — X-Men 2 gameplay consumption

Missing capability: X-Men 2 pins this repository and uses shared XMLB/ARK tooling, but its CMake
authority explicitly links none of the `alchemy`, `alchemy_input`, or `alchemy_input_sdl` targets and
its product source calls no shared runtime API.

Gap: link the first narrow contract through the shipping build and a title-local guest ABI adapter.
The `alchemy::input` target is the first candidate; retain DirectInput as the oracle until button
bits, pressure, axes, callbacks, lifecycle, and stable identity pass A/B conformance.
The neutral target must remain host-independent; any reusable x86port context translation belongs
in a separately selected `alchemy/x86` adapter while exact addresses and registration stay in X-Men 2.

### S012 — MUA gameplay consumption

Missing capability: MUA has no Alchemy dependency resolver, build edge, source include, tool import,
or runtime call path. Its current documentation describes only the intended ownership direction.

Gap: MUA remains deferred until every X-Men 2 project goal is verified. After that deferral lifts,
recover the MUA PPC/ARK guest ABI and adapt it to the contracts X-Men 2 has already proven; do not
create an MUA-local engine implementation. Reusable host translation may live in a separately
selected `alchemy/x360` adapter; exact MUA identity and bindings remain consumer-owned.

### S013 — Runtime API and quality enforcement

Evidence: `igb_scene_load` accepts immutable typed options and emits typed diagnostic events through
an injected observer; no shipping library source reads process configuration or writes diagnostics.
The standalone viewers parse their shared screenshot/transform CLI through one
`alchemy_viewer_config_parse` owner and contain no environment reads.
The C++20 input owner uses narrow typed value objects and RAII SDL lifetime. The structure self-test
proves the normal gate rejects environment reads, direct output, title vocabulary, consumer edges,
backend event polling, and source growth. `cpp_format` and `cpp_tidy` cover the C++ runtime and tests.
The pure C parsers remain C, and object lookup was extracted into `src/igb_object.c` instead of
growing the legacy mesh unit.

### S014 — Consumer dependency authority

X-Men 2's bootstrap pins one immutable revision of this repository and uses it for offline tooling.

Gap: that pin does not feed any runtime CMake target, and MUA has no resolver or pin. Each consumer
needs one authoritative immutable revision used by both tooling and runtime integration, with no
vendored or sibling-checkout fallback.
