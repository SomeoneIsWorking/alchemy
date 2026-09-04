# alchemy

This repository is the partial native foundation for a shared **Alchemy** (Gap)
engine. Today it provides measured IGB readers and payload decoders, offline
format tools, an abstract controller state model, and an SDL3 controller
backend. It is not yet a complete engine and neither gameplay product currently
links it.

Alchemy is Raven/Vicarious Visions' engine. The games that matter here:

| Game | Platforms |
|---|---|
| X-Men Legends II: Rise of Apocalypse | PC, Xbox, PS2, GameCube, PSP |
| Marvel Ultimate Alliance | Xbox 360, PS2, Xbox, PS3, PSP, PC |

This repo exists because those two games span platforms with different CPUs and
GPU payload formats. Title-neutral engine behavior belongs in one peer rather
than being copied between ports. The intended consumers are X-Men Legends II
and Marvel: Ultimate Alliance; actual runtime consumption is still missing.

X-Men 2 currently pins this repository and uses its XMLB/ARK tooling during
asset preparation and reverse engineering, but `x2native` explicitly links
none of the libraries. MUA has no build, source, launcher, or tool dependency on
this repository yet. X-Men 2 must establish the first conformed runtime
contract; MUA is deferred until all X-Men 2 goals are verified and then adopts
the proven shared surface.

## What's here

```
src/     igb.c/.h            IGB container reader
         igb_image.c/.h      image discovery and PS2 image formats
         igb_mesh.c/.h       meshes
         igb_raster.c/.h     textures
         igb_anim.c/.h       animation, incl. the Enbaya-compressed codec
         ig_controller.c/.h  the engine's controller abstraction
         ig_sdl_controller.c SDL3 backend for it
apps/    x2view meshview flyview   asset viewers (SDL2, separate binaries)
tools/   igb_dump.c          dump an IGB's structure
         ark_classes.py      recover a module's ARK class graph
         ark_vtables.py      ARK vtable addresses, slots, overrides
         xmlb.py             Raven's binary XML container
         extract_wad.py      WAD archives
         extract_font_igb.py fonts out of an IGB
         raven-formats/      vendored Raven format tooling
docs/    ark.md              the ARK object/reflection system
         enbaya_decode.md    the animation codec, decoded
         conversations.md    the conversation system
```

`src/` includes only its own headers — no recompiler, no platform loader, no
game. That self-containment is what let it be lifted out of the X-Men Legends II
port unchanged.

## Build

```sh
cmake -S . -B build && cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure     # enbaya, controller, xmlb_roundtrip
```

Standalone it builds the viewers and tools. Added as a subdirectory, it exposes
the `alchemy` and `alchemy_input` libraries plus `alchemy_input_sdl` when SDL3
is available. No current gameplay product links those targets.

**Two SDLs on purpose.** SDL3 backs `alchemy_input`, because that is what a live
port links. SDL2 backs the viewers, which are separate binaries — that split is
what stops two SDLs being linked into one process. Don't collapse them.

## Integration target

The first candidate shipping contract is the existing controller layer. X-Men
2 needs a title-local adapter from shared controller snapshots to the retained
guest `igControllerManager` ABI, with its current DirectInput path retained as
an A/B oracle until buttons, pressure, axes, callbacks, hotplug, and stable
identity agree. Merely provisioning this repository or importing `xmlb.py` is
not that proof.

After X-Men 2 completes all of its project goals, MUA adds its separately
recovered PPC/ARK ABI adapter to the same title-neutral contracts. Guest object
layouts, addresses, and player policy remain in each title repository.

The migration is specified in [`docs/migration.md`](docs/migration.md), and
current capability coverage is recorded in
[`docs/project-state.md`](docs/project-state.md).

## Platform coverage

Container byte order is a property of the asset, not the CPU that consumes it.
All supplied MUA assets are little-endian IGBs: 311 PS2 files use version 6 and
the 324 Xbox 360 base files plus 261 Gold/title-update files use version 8. The
current reader opens that complete corpus.

The Xbox 360 assets use the existing DXT image path. MUA's PS2 assets add
RGBA5551 and indexed CLUT8 payloads; both are decoded by `igb_image.c` and
checked against matching Xbox 360 images. General big-endian IGB structure and
semantic payloads remain unsupported until a real corpus proves their layout.

## History

Extracted from the X-Men Legends II port (`pc/xmen2`) on 2026-08-18. Commit
history for these files before that date lives in that repository.
