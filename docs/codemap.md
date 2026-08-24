# Codemap

Alchemy is the platform-neutral engine layer consumed by the X-Men Legends II
and Marvel Ultimate Alliance ports. Game executable code and console services
sit above it; raw Raven/Alchemy containers sit below it. MUA's supplied PS2 and
Xbox 360 archive/container corpora are now traversed and its two PS2 image
formats are decoded. Mesh, animation, audio, and executable-side ARK/input
semantics remain separate compatibility frontiers.

Status vocabulary:

| Status | Meaning |
|---|---|
| **real** | Implemented and verified on real game data. |
| **partial** | Implemented with the named coverage gap. |
| **tool-only** | Available for offline inspection, not a linked runtime owner. |
| **absent** | Not implemented. |

## Subsystems

| Subsystem | Status | Where | Gap / next |
|---|---|---|---|
| IGB container/schema | **real** | `src/igb.c`, `igb_open` | Opens the supplied MUA PS2 v6 and Xbox 360 v8 corpora. General big-endian semantic payloads remain unverified. |
| IGB images | **real** | `src/igb.c`, `src/igb_image.c`, `igb_image_to_rgba` | Xbox 360 DXT and MUA PS2 RGBA5551/CLUT8 decode through one API. Additional platform formats need their own evidence. |
| IGB meshes/scenes | **partial** | `src/igb_mesh.c`, `igb_scene_load` | Verified for XML2 assets only; MUA topology, transforms, palettes, and platform payload assumptions need corpus checks. |
| IGB CPU rasterizer | **tool-only** | `src/igb_raster.c` | Debug viewer rasterizer, not a shipping renderer. |
| Enbaya animation | **partial** | `src/igb_anim.c`, `igb_enbaya_decode` | Algorithm is implemented; the normal test currently lacks a bundled real blob. Add explicit MUA corpus verification. |
| Input abstraction | **partial** | `src/ig_controller.c`, `src/ig_sdl_controller.c` | Host data model works. MUA ARK class graph, vtable ABI, and concrete substitution are unverified. |
| XMLB | **real** | `tools/xmlb.py`, `tools/mua_corpus.py` | 14,348/14,348 MUA payload occurrences round-trip byte-identically; this remains an offline asset owner, not a linked runtime service. |
| FB packages | **real** | `tools/raven-formats/src/raven_formats/fb.py`, `tools/alchemy_archives.py` | 1,087/1,087 MUA packages parse; ordered iteration retains all 129 duplicate-path occurrences. |
| MUA real-disc corpus | **real** | `tools/mua_corpus.py`, `inspect_archives` | PS2, base Xbox 360, Gold DLC, and title-update archives are fully traversed through FB/XMLB/IGB owners; semantic mesh/animation/raster correctness remains separate. |
| Raven audio containers | **tool-only** | `tools/raven-formats/src/raven_formats/zsnd.py` | Recognizes PS2/Xenon layouts; no native runtime decoder or MUA corpus gate. |
| igblib (vendored Python IGB reader) | **vendored** | `vendor/igblib/` | KaikoClanworth1's reader, tracked so consumers resolve it from this repo instead of a per-machine gitignored scratch copy (`tools/extract_font_igb.py`; the XML2 port's `make_pad_font.py`). No LICENSE accompanied the snapshot — origin recorded in `vendor/igblib/README.md`. |
| ARK recovery | **partial** | `tools/ark_classes.py`, `tools/ark_vtables.py`, `docs/ark.md` | Current analyzers understand x86 PE patterns only. Add chosen-platform MUA PPC evidence in the port, then lift shared semantics here. |
| Asset viewers | **partial** | `apps/` | Functional XML2 viewers with title-specific names and transform policy; generalize against MUA data. |
| Verification | **partial** | `tests/`, `tools/mua_corpus.py` | Deterministic tests cover parser refusals and image decoding; the full measured disc gate is explicit because copyrighted corpora are never tracked. Mesh, animation, audio, and runtime semantics need their own gates. |

## Source tree

```text
src/  —  2,963 lines, 16 files
apps/  —  534 lines, 3 files
tools/  —  3,258 lines, 17 files
├─ raven-formats/  988 lines  7 files  [.py]
│  ├─ src/  952 lines  6 files
│  ├─ tests/  36 lines  1 file
tests/  —  571 lines, 4 files

TOTAL: 7,326 lines across 40 files in 4 roots
```

## Where is X?

- Open an IGB: `src/igb.c` → `igb_open`
- Discover/decode an image: `src/igb_image.c`, `src/igb.c` → `igb_find_images`, `igb_image_to_rgba`
- Walk a scene: `src/igb_mesh.c` → `igb_scene_load`
- Decode Enbaya: `src/igb_anim.c` → `igb_enbaya_decode`
- Translate SDL input: `src/ig_sdl_controller.c`
- Inspect XMLB: `tools/xmlb.py`
- Inspect FB/ZSND: `tools/raven-formats/src/raven_formats/`
- Verify the real MUA discs and Gold content: `tools/mua_corpus.py`
- Enforce source ownership limits: `tools/check_structure.py`
- Recover ARK metadata: `tools/ark_classes.py`, `tools/ark_vtables.py`
