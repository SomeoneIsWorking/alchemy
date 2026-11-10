# Shared Alchemy

## Current and intended boundary

This repository is a partial native Alchemy/Gap foundation, not a completed
shared engine. It owns measured IGB readers, typed payload decoders, offline
XMLB/FB/ARK tooling, an abstract controller state model, and an SDL3
controller backend. Neither X-Men Legends II nor Marvel: Ultimate Alliance yet
links and exercises them as part of its gameplay product.

The goal is that both ports consume title-neutral native engine contracts
here. X-Men 2 is the active conformance title and establishes the first
gameplay build/link/call-path integration. MUA stays deferred until every
X-Men 2 project goal is verified, then consumes the proven contracts through
its own PPC/ARK ABI adapter rather than a parallel engine.

It does not own a game's executable, guest addresses, a console kernel or CPU
execution engine, title-specific scripts, or a shipping renderer. The neutral
core has zero dependency on x86port or an Xbox 360 host. Optional, separately
selectable `alchemy/x86` and `alchemy/x360` adapters may consume those hosts'
public execution/context interfaces when a proven Alchemy contract needs a
guest bridge. Exact executable hashes, guest addresses, object layout adapters,
override registration, and title policy stay in each title binding. A behavior
is shared only after its engine ownership is evidenced, not because two games
call it. Shared interfaces are extracted or extended only from a narrow
contract the active title demonstrates; no generic scene graph, renderer,
resource system, or title framework ahead of evidence.

## Consumers

- A checkout or a tool import is not gameplay consumption. A consumer resolves
  one pinned revision, links a shared target, executes its API on the shipping
  path, and passes title-specific conformance evidence.
- Each consumer has one dependency resolver and one immutable revision: no
  sibling-checkout fallback, second environment-variable path, or independent
  tool/runtime pin.

## Native input

`include/alchemy/input/controller.hpp` and `src/input/controller.cpp` own
typed controller snapshots, stable device/slot identities, and connection
events. `SdlControllerBackend` owns SDL gamepad discovery, handles,
translation, rumble, and snapshot refresh behind a pImpl. The host application
owns SDL's process-wide event pump and forwards every event through
`handleEvent`; the backend never calls `SDL_PollEvent`. A title owns action
bindings, player participation, device-to-player policy, and prompt
presentation. Binary evidence and the guest-substitution frontier:
`docs/input.md`.

This is the first candidate product contract, not a completed integration.
X-Men 2 must link the shared target, adapt its snapshots to the retained guest
`igControllerManager` surface, and A/B-verify button bits, pressure, axes,
lifecycle, callbacks, and stable identity through the shipping path before its
DirectInput owner can be retired. MUA integration begins only after its
deferral lifts and recovers its own guest ABI first.

## Compatibility evidence

- CPU byte order, container byte order, and encoded GPU/audio payload order are
  separate facts; never infer one from another.
- A parser opening a container proves structure only. Texture decode,
  animation semantics, mesh topology, and rendering each need their own
  positive and negative checks.
- Real-corpus tests take explicit paths or environment variables.
- `x2_*`, `X2_*`, and `X2VIEW_*` names are extraction residue, not an ownership
  boundary. New public APIs use `alchemy_` or the engine's `ig*` vocabulary;
  consumers migrate atomically when an old name retires.

## Structure

- Owners: container parsing, typed mesh/image interpretation, animation,
  input, archive tooling, and viewers are separate.
- The proven C parsers and pure transformations stay C; do not rewrite them for
  style. New stateful owners and guest-facing adapters are C++20.
- Library code never reads the environment or writes to stderr/platform debug
  output. Libraries emit typed diagnostic events through an injected observer;
  the viewer/tool application translates them into its Lucent logger and owns
  CLI/environment configuration.
- New first-party source files are capped at 500 lines. Larger existing files
  are frozen until a cohesive owner is extracted, then their ceiling is lowered.
- `tools/check_structure.py` rejects `getenv`, direct stderr/debug output,
  title vocabulary, forbidden consumer dependencies, and growth beyond the
  recorded limits.

## Build and test

- Full verifier: `uv run --frozen python tools/verify.py` (builds the real
  owners in `build/verify` and runs CTest).
- Focused tests: `ctest --test-dir build/verify -R <name>` (`enbaya`,
  `file_io`, `igb_image`, `scene_options`, `viewer_config`, `input`,
  `sdl_input`, `xmlb_roundtrip`, ...; `*_real` need the real corpus).
- Do not add tests of the verifier, the structure check, or other tooling.
