# alchemy

The **Alchemy** (Gap) engine layer, as a library: the IGB asset readers and the
engine's input abstraction, with the reverse-engineering write-ups that explain
the formats.

Alchemy is Raven/Vicarious Visions' engine. The games that matter here:

| Game | Platforms |
|---|---|
| X-Men Legends II: Rise of Apocalypse | PC, Xbox, PS2, GameCube, PSP |
| Marvel Ultimate Alliance | Xbox 360, PS2, Xbox, PS3, PSP, PC |

This repo exists because those two games span platforms with different CPUs and
GPU payload formats. An engine layer nested inside one platform's port gets
duplicated the moment the second game starts; this one is a peer, consumed by
each port.

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

Standalone it builds the viewers and tools. Consumed by a port
(`add_subdirectory`) it builds only the two libraries, `alchemy` and
`alchemy_input`.

**Two SDLs on purpose.** SDL3 backs `alchemy_input`, because that is what a live
port links. SDL2 backs the viewers, which are separate binaries — that split is
what stops two SDLs being linked into one process. Don't collapse them.

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
