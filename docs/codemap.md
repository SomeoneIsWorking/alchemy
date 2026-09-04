# Codemap

Alchemy is the intended platform-neutral engine peer below the X-Men Legends II
and Marvel Ultimate Alliance title adapters. The current repository contains
format, asset, input, viewer, and RE-tool owners; consumer-side guest ABI and
title policy stay above it. Capability status and remaining gaps belong in
`docs/project-state.md`.

## Subsystems

| Subsystem | Responsibility | Current/target location | Entry point | Deep doc |
|---|---|---|---|---|
| IGB container/schema | Parse container metadata and typed object fields | `src/igb.c`, `src/igb_internal.c` | `igb_open` | — |
| IGB images | Discover and decode platform image payloads | `src/igb.c`, `src/igb_image.c` | `igb_find_images`, `igb_image_to_rgba` | — |
| IGB meshes/scenes | Decode semantic geometry, materials, skinning, and transforms | `src/igb_mesh.c` | `igb_scene_load` | — |
| IGB CPU rasterizer | Render diagnostic viewer geometry without becoming a shipping renderer | `src/igb_raster.c` | raster API in `src/igb_raster.h` | — |
| Enbaya animation | Decode and sample compressed animation streams | `src/igb_anim.c` | `igb_enbaya_decode`, `igb_enbaya_pose_at` | `docs/enbaya_decode.md` |
| Platform-neutral controller state | Own stable controller slots, buttons, pressure, and axes | `src/ig_controller.c` | `ig_controller_manager_init` | `docs/input.md` |
| SDL controller transport | Own SDL gamepad handles, event translation, snapshots, and rumble | `src/ig_sdl_controller.c` | `ig_sdl_controller_initialize` | `docs/input.md` |
| Typed runtime options | Carry caller-validated options into shared library owners without process-environment reads | target: a focused public value type and owning source under `src/` | constructor/function option argument | `docs/migration.md` |
| Runtime diagnostics | Route shared-library diagnostics through one injected Lucent logger | target: a focused logging adapter under `src/` | explicit logger dependency | `docs/migration.md` |
| XMLB | Decode, edit, and serialize Raven binary XML | `tools/xmlb.py` | `parse`, `serialise` | — |
| FB packages | Decode ordered Raven package contents | `tools/raven-formats/src/raven_formats/fb.py`, `tools/alchemy_archives.py` | tool/library entry points | — |
| MUA corpus inspection | Traverse user-owned PS2/Xbox 360 archives through shared format owners | `tools/mua_corpus.py` | `inspect_archives` | `docs/mua-corpus.md` |
| Raven audio containers | Inspect ZSND containers offline | `tools/raven-formats/src/raven_formats/zsnd.py` | module API | — |
| Vendored Python IGB reader | Support Python-only inspection consumers with recorded provenance | `vendor/igblib/` | `igblib` package | `vendor/igblib/README.md` |
| ARK recovery | Recover class graphs and vtables from title binaries | `tools/ark_classes.py`, `tools/ark_vtables.py` | each tool's `main` | `docs/ark.md` |
| Asset viewers | Compose SDL2-only standalone inspection applications | `apps/` | each application's `main` | — |
| Verification | Exercise production parsers, input owners, structure rules, and measured corpora | `tests/`, `tools/check_structure.py`, `tools/mua_corpus.py` | CTest and tool selftests | — |
| X-Men 2 guest ABI adapter | Translate retained PC `igControllerManager` and later engine seams to shared contracts | consumer-owned target: `pc/xmen2` | title override/adapter registration | `docs/migration.md` |
| MUA guest ABI adapter | Translate retained PPC/ARK engine seams to shared contracts after deferral lifts | consumer-owned target: `x360/mua` | title override/adapter registration | `docs/migration.md` |
| Dependency resolution | Expose stable CMake targets; each consumer owns one immutable repo pin/resolver | `CMakeLists.txt`; consumer bootstrap/config | `alchemy`, `alchemy_input`, `alchemy_input_sdl` targets | `docs/migration.md` |

## Source tree

```text
src/  —  3,069 lines, 16 files
apps/  —  534 lines, 3 files
tools/  —  3,300 lines, 17 files
├─ raven-formats/  988 lines  7 files  [.py]
│  ├─ src/  952 lines  6 files
│  ├─ tests/  36 lines  1 file
tests/  —  576 lines, 4 files

TOTAL: 7,479 lines across 40 files in 4 roots
```

## Where is X?

- Open an IGB: `src/igb.c` → `igb_open`
- Discover/decode an image: `src/igb_image.c`, `src/igb.c` → `igb_find_images`, `igb_image_to_rgba`
- Walk a scene: `src/igb_mesh.c` → `igb_scene_load`
- Decode Enbaya: `src/igb_anim.c` → `igb_enbaya_decode`
- Read/update Alchemy controller state: `src/ig_controller.c`
- Translate centrally forwarded SDL input and hotplug: `src/ig_sdl_controller.c`
- Inspect XMLB: `tools/xmlb.py`
- Inspect FB/ZSND: `tools/raven-formats/src/raven_formats/`
- Verify the real MUA discs and Gold content: `tools/mua_corpus.py`
- Enforce source ownership limits: `tools/check_structure.py`
- Recover ARK metadata: `tools/ark_classes.py`, `tools/ark_vtables.py`
- Add a title-neutral runtime contract: the smallest cohesive owner under `src/`, with its guest ABI adapter kept in the consuming title
- Read configuration: consumer or standalone-app configuration owner; never a shared parser/input module
- Emit runtime diagnostics: injected Lucent logger boundary; never direct stderr/platform output from shipping library code
