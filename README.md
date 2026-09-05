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
         igb_object.c        object/field lookup used by scene decoding
         igb_raster.c/.h     textures
         igb_anim.c/.h       animation, incl. the Enbaya-compressed codec
         input/              C++20 controller registry and SDL3 transport
include/ alchemy/input/      typed public controller and SDL APIs
apps/    x2view meshview flyview   asset viewers (SDL2, separate binaries)
         viewer_config.c/.h shared typed CLI configuration for those viewers
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
uv run --frozen python tools/verify.py
```

The verifier requires CMake, Ninja, Clang/clang-cl (AppleClang on macOS),
clang-format, clang-tidy, and both SDL development packages. Python/Ruff are
locked by `pyproject.toml` and `uv.lock`. Use `--fetch-sdl` to build the
checksum-pinned SDL2/SDL3 sources instead of using system packages. Windows
uses a Visual Studio x64 developer environment with clang-cl and `--fetch-sdl`;
the workflow prepares that environment and executes the same verifier. macOS
selects `/usr/bin/clang` and `/usr/bin/clang++` even when Homebrew LLVM provides
the formatting/lint tools.

All three desktop jobs compile the actual libraries, viewers and dump tool and
execute synthetic parser/animation/input/SDL tests. Missing required tests or
skipped quality checks fail verification. Only explicitly named real-corpus
tests may skip without game files. Android runtime/package evidence is still
missing; see the host matrix in `docs/project-state.md`.

Standalone it builds the viewers and tools. Added as a subdirectory, it exposes
the `alchemy` and `alchemy::input` libraries plus `alchemy::input_sdl` when SDL3
is available. The un-namespaced input target names remain the real CMake
targets; the namespaced aliases make consumer intent explicit. No current
gameplay product links those targets.

**Two SDLs on purpose.** SDL3 backs the separate `alchemy::input_sdl` transport,
while the platform-neutral `alchemy::input` target has no SDL dependency. SDL2
backs the viewers, which are separate binaries — that split prevents two SDLs
from entering one process. Don't collapse them.

Viewer configuration is explicit CLI input: `--screenshot PATH` is shared by
all three viewers, while `meshview` and `flyview` also accept
`--compose-transforms`. The viewers no longer read title-specific environment
switches, and `igb_scene_load` receives the resulting immutable options.

## Integration target

The first candidate shipping contract is the typed controller layer. X-Men 2
can feed externally acquired DirectInput state into `ControllerManager` through
`connect`, `publish`, and `disconnect` while linking only `alchemy::input`; SDL
ownership stays in the optional peer transport. Its title-local adapter maps
the read-only shared snapshots to the retained guest `igControllerManager` ABI,
with DirectInput retained as an A/B oracle until buttons, pressure, axes,
callbacks, hotplug, and stable identity agree. Merely provisioning this
repository or importing `xmlb.py` is not that proof.

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
