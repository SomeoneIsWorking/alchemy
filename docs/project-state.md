# Project state

Baseline: every Alchemy-engine game port carrying its own container decoders, archive tools,
asset viewers, and controller glue. This repository supplies the shared libraries and tools; no
gameplay product links them yet.

Current focus: S011 — X-Men 2 links and executes a conformed shared Alchemy contract.

| ID | Capability | State | Evidence or gap |
| --- | --- | --- | --- |
| S001 | Open and inspect IGB containers | verified | `src/igb.c` parses every supplied MUA PS2 (v6) and Xbox 360 (v8) file; little-endian, no byte swapping |
| S002 | Decode mesh, texture, raster, and animation payloads into native data | partial | Measured DXT, PS2 RGBA5551 and CLUT_INDEX8, and Enbaya decode; big-endian structures and cross-title render validation unproven |
| S003 | Reusable XMLB, FB/WAD, ARK, font, and conversation tooling | partial | `tools/xmlb.py`, `tools/alchemy_archives.py`, `tools/ark_classes.py`, `tools/ark_vtables.py`, `tools/extract_font_igb.py` |
| S004 | Platform-neutral controller snapshots and SDL3 backend | partial | `ControllerManager` plus `SdlControllerBackend`; X-Men 2 guest substitution and MUA ARK ABI unverified |
| S005 | Standalone viewers and dump tool inspect assets without a game port | verified | `apps/x2view`, `meshview`, `flyview`, `tools/igb_dump.c` build on the shared libraries |
| S006 | IGB meshes decode into vertices, indices, materials, skinning | partial | `igb_scene_load`; complete cross-title validation missing |
| S007 | Xbox 360 and PS2 texture/raster payloads decode into native images | partial | `src/igb_image.c`; `igb_image_real` compares matching PS2/360 assets when a corpus is present |
| S008 | Enbaya-compressed animation decodes into native animation | partial | `igb_enbaya_decode`, `igb_enbaya_pose_at`; cross-title semantics unverified |
| S009 | XMLB assets decode, edit, and round-trip through shared tooling | partial | `tools/xmlb.py --selftest` round-trips byte-identically; unmeasured variants remain |
| S010 | FB/WAD, ARK class/vtable, font, and conversation formats have reusable tools | partial | Each tool's coverage is evidence-driven and incomplete |
| S011 | X-Men 2 gameplay links and executes a conformed shared contract | missing | Its CMake authority links none of `alchemy`, `alchemy_input`, `alchemy_input_sdl`; `alchemy::input` is the first candidate, DirectInput stays the oracle |
| S012 | MUA gameplay links and executes proven shared contracts | missing | No resolver, pin, build edge, or call path; deferred until every X-Men 2 goal is verified |
| S013 | Configuration, diagnostics, language, and dependency boundaries are mechanically enforced | verified | `structure`, `cpp_format`, `cpp_tidy`, `python_lint` reject environment reads, direct output, title vocabulary, consumer edges, and source growth |
| S014 | Each consumer resolves one immutable revision for tools and runtime targets | partial | X-Men 2 pins the repository for offline tooling only; MUA has no resolver or pin |