# Codemap

Alchemy is the intended engine peer below the X-Men Legends II and Marvel
Ultimate Alliance title bindings. Its neutral core contains format, asset,
input, viewer, and RE-tool owners and depends on no CPU/console host. Optional
`alchemy/x86` and `alchemy/x360` layers may translate proven contracts through
the corresponding host's public execution/context interfaces. Exact guest ABI
addresses, hashes, layouts, override registration, and title policy stay above
them. Capability status and remaining gaps belong in `docs/project-state.md`.

## Subsystems

| Subsystem | Responsibility | Current/target location | Entry point | Deep doc |
|---|---|---|---|---|
| IGB container/schema | Parse container metadata and typed object fields; one file-open boundary adapts POSIX and Windows CRT contracts | `src/igb.c`, `src/igb_internal.c` | `igb_open`, `igb_file_open` | — |
| IGB images | Discover and decode platform image payloads | `src/igb.c`, `src/igb_image.c` | `igb_find_images`, `igb_image_to_rgba` | — |
| IGB object access | Own object-type and typed field/slot lookup used by semantic decoders | `src/igb_object.c` | `igb_obj_is`, `igb_obj_slot_i32`, `igb_obj_slot_blob` | — |
| IGB meshes/scenes | Decode semantic geometry, materials, skinning, caller-selected transforms, and typed diagnostic events | `src/igb_mesh.c`, `src/igb_mesh.h` | `igb_scene_load` | — |
| IGB CPU rasterizer | Render diagnostic viewer geometry without becoming a shipping renderer | `src/igb_raster.c` | raster API in `src/igb_raster.h` | — |
| Enbaya animation | Decode and sample compressed animation streams | `src/igb_anim.c` | `igb_enbaya_decode`, `igb_enbaya_pose_at` | `docs/enbaya_decode.md` |
| Platform-neutral controller state | Own typed devices, stable slots, button/pressure/axis snapshots, and lifecycle events without an SDL dependency | `include/alchemy/input/controller.hpp`, `src/input/controller.cpp` | `alchemy::input::ControllerManager` | `docs/input.md` |
| SDL controller transport | Own SDL gamepad handles, event translation, snapshots, rumble, immutable thresholds, and typed failures through RAII | `include/alchemy/input/sdl_controller.hpp`, `src/input/sdl_controller.cpp` | `alchemy::input::SdlControllerBackend` | `docs/input.md` |
| Typed runtime options | Carry caller-validated immutable options into shared library owners without process-environment reads | `src/igb_mesh.h`, `include/alchemy/input/sdl_controller.hpp` | `igb_scene_options`, `SdlControllerSettings` | `docs/migration.md` |
| Runtime diagnostics | Emit typed library events to an injected observer for one application-owned Lucent adapter | `src/igb_mesh.h`, `include/alchemy/input/sdl_controller.hpp` | `igb_scene_diagnostic_fn`, `SdlDiagnosticObserver` | `docs/migration.md` |
| XMLB | Decode, edit, and serialize Raven binary XML | `tools/xmlb.py` | `parse`, `serialise` | — |
| FB packages | Decode ordered Raven package contents | `tools/raven-formats/src/raven_formats/fb.py`, `tools/alchemy_archives.py` | tool/library entry points | — |
| MUA corpus inspection | Traverse user-owned PS2/Xbox 360 archives through shared format owners | `tools/mua_corpus.py` | `inspect_archives` | `docs/mua-corpus.md` |
| Raven audio containers | Inspect ZSND containers offline | `tools/raven-formats/src/raven_formats/zsnd.py` | module API | — |
| Vendored Python IGB reader | Support Python-only inspection consumers with recorded provenance | `vendor/igblib/` | `igblib` package | `vendor/igblib/README.md` |
| ARK recovery | Recover class graphs and vtables from title binaries | `tools/ark_classes.py`, `tools/ark_vtables.py` | each tool's `main` | `docs/ark.md` |
| Asset viewers | Compose SDL2-only standalone inspection applications | `apps/` | each application's `main` | — |
| Viewer configuration | Parse the shared screenshot and transform CLI options once for standalone viewers | `apps/viewer_config.c`, `apps/viewer_config.h` | `alchemy_viewer_config_parse` | — |
| Verification | Select native toolchains and build actual library/viewer owners; require executed synthetic/quality checks and separate real-corpus evidence | `tests/`, `tools/verify.py`, `tools/verification.py`, `tools/check_structure.py`, `tools/cpp_quality.py`, `tools/mua_corpus.py` | `uv run --frozen python tools/verify.py` and CTest | — |
| Standalone SDL provisioning | Fetch immutable checksum-pinned SDL2/SDL3 sources when requested, keeping the two library processes separate | `cmake/SdlDependencies.cmake` | `ALCHEMY_FETCH_SDL` | — |
| X-Men 2 guest ABI adapter | Translate retained PC `igControllerManager` and later engine seams to shared contracts | consumer-owned target: `pc/xmen2` | title override/adapter registration | `docs/migration.md` |
| MUA guest ABI adapter | Translate retained PPC/ARK engine seams to shared contracts after deferral lifts | consumer-owned target: `x360/mua` | title override/adapter registration | `docs/migration.md` |
| x86 platform adapter | Translate proven Alchemy contracts through x86port public execution/context interfaces without title identity or addresses | target: optional `alchemy/x86` target and source subtree | narrow adapter API selected by an x86 consumer | `docs/migration.md` |
| Xbox 360 platform adapter | Translate proven Alchemy contracts through Xbox 360 host public execution/context interfaces without title identity or addresses, after X-Men 2 proves the contract | target: optional `alchemy/x360` target and source subtree | narrow adapter API selected by an Xbox 360 consumer | `docs/migration.md` |
| Dependency resolution | Expose stable CMake targets; each consumer owns one immutable repo pin/resolver | `CMakeLists.txt`; consumer bootstrap/config | `alchemy`, `alchemy::input`, `alchemy::input_sdl` targets | `docs/migration.md` |

## Source tree

```text
./  —  8,189 lines, 51 files
├─ apps/  611 lines  5 files  [.c .h]
├─ include/  231 lines  2 files  [.hpp]
│  ├─ alchemy/  231 lines  2 files
│  │  ├─ input/  231 lines  2 files
├─ src/  3,000 lines  15 files  [.c .h .cpp]
│  ├─ input/  500 lines  2 files
├─ tests/  794 lines  9 files  [.c .cpp .py]
├─ tools/  3,553 lines  20 files  [.py .c]
│  ├─ raven-formats/  988 lines  7 files
│  │  ├─ src/  952 lines  6 files
│  │  ├─ tests/  36 lines  1 file

TOTAL: 8,189 lines across 51 files in 1 root
```

## Where is X?

- Open an IGB: `src/igb.c` → `igb_open`
- Discover/decode an image: `src/igb_image.c`, `src/igb.c` → `igb_find_images`, `igb_image_to_rgba`
- Walk a scene: `src/igb_mesh.c` → `igb_scene_load`
- Decode Enbaya: `src/igb_anim.c` → `igb_enbaya_decode`
- Read/update Alchemy controller state: `ControllerManager` and `ControllerState` in `include/alchemy/input/controller.hpp`
- Publish an external platform snapshot without SDL: link `alchemy::input`, then call `connect`, `publish`, and `disconnect`
- Translate centrally forwarded SDL input and hotplug: `SdlControllerBackend` in `include/alchemy/input/sdl_controller.hpp`
- Inspect XMLB: `tools/xmlb.py`
- Inspect FB/ZSND: `tools/raven-formats/src/raven_formats/`
- Verify the real MUA discs and Gold content: `tools/mua_corpus.py`
- Enforce source ownership limits: `tools/check_structure.py`
- Recover ARK metadata: `tools/ark_classes.py`, `tools/ark_vtables.py`
- Configure an asset viewer: `alchemy_viewer_config_parse` in `apps/viewer_config.c`
- Add a title-neutral runtime contract: the smallest cohesive owner under `src/`, with its guest ABI adapter kept in the consuming title
- Add reusable host translation: an optional `alchemy/x86` or `alchemy/x360` adapter; never add a host dependency to the neutral targets
- Bind an exact executable/address/layout: the consuming title repository, never a shared platform adapter
- Read configuration: consumer or standalone-app configuration owner; never a shared parser/input module
- Emit runtime diagnostics: typed injected observer at the library edge, translated once to Lucent by the application; never direct stderr/platform output from shipping library code
