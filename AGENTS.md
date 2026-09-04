# Shared Alchemy guidance

The repository-wide rules in `../../AGENTS.md` apply here. Consult
`docs/codemap.md` before changing a subsystem and update it in the same commit.

## Current and intended boundary

This repository is a partial native Alchemy/Gap foundation, not a completed
shared engine. It currently owns measured IGB readers, typed payload decoders,
offline XMLB/FB/ARK tooling, an abstract controller state model, and an SDL3
controller backend. These are useful shared components, but neither X-Men
Legends II nor Marvel: Ultimate Alliance currently links and exercises them as
part of its gameplay product.

The durable product goal is that both ports consume title-neutral native engine
contracts here. X-Men 2 is the active conformance title and must establish the
first gameplay build/link/call-path integration. MUA remains deferred until
every X-Men 2 project goal is verified, then consumes the proven contracts
through its own PPC/ARK ABI adapter rather than creating a parallel engine.

It does not own a game's executable, guest addresses, a console kernel or CPU
execution engine, title-specific scripts, or a shipping renderer. The neutral
core has zero dependency on x86port or an Xbox 360 host. Optional, separately
selectable `alchemy/x86` and `alchemy/x360` adapters may consume those hosts'
public execution/context interfaces when a proven Alchemy contract needs a
guest bridge. Exact executable hashes, guest addresses/layouts, override
registration, and title policy remain in each title binding. A behavior is
shared only after its engine ownership is evidenced; do not move title policy
here because two games happen to call it.

## Native input ownership

`include/alchemy/input/controller.hpp` and `src/input/controller.cpp` own
Alchemy's typed controller snapshots, stable device/slot identities, and
connection events. `SdlControllerBackend` owns SDL gamepad discovery, handles,
translation, rumble, and snapshot refresh behind a pImpl. The host application
owns SDL's one process-wide event pump and forwards every event through
`handleEvent`; the backend must never call `SDL_PollEvent` itself. A title owns
action bindings, player participation, device-to-player policy, and prompt
presentation. See `docs/input.md` for the binary evidence and the remaining
guest-substitution frontier.

This is the first candidate product contract, not a completed integration.
X-Men 2 must link the shared target, adapt its snapshots to the retained guest
`igControllerManager` surface, and A/B-verify button bits, pressure, axes,
lifecycle, callbacks, and stable identity through the shipping path before its
current DirectInput owner can be retired. MUA integration begins only after its
deferral lifts and must recover its own guest ABI before reusing the same
title-neutral model.

## Consumer and dependency discipline

- A checkout, tool import, or offline corpus run is not gameplay consumption.
  A consumer must resolve one pinned revision, link a shared target, execute its
  API on the shipping path, and pass title-specific conformance evidence.
- Each consumer has one dependency resolver and one immutable revision. Do not
  add a sibling-checkout fallback, vendored copy, second environment-variable
  path, or independent tool/runtime pin.
- Shared interfaces are extracted or extended only from a narrow contract
  demonstrated by the active title. Do not design a generic scene graph,
  renderer, resource system, or title framework ahead of evidence.
- Guest addresses, object layout adapters, native override registration, and
  title policy stay in the consuming port. This repository owns only the
  title-neutral semantic contract.

## Compatibility evidence

- Treat CPU byte order, container byte order, and encoded GPU/audio payload
  order as separate facts. Never infer one from another.
- A parser opening a container proves structure only. Texture decode,
  animation semantics, mesh topology, and rendering each need their own
  positive and negative checks.
- Real-disc corpus checks accept explicit paths or environment variables and
  keep all extracted data under ignored `scratch/`. Tests never contain or
  download copyrighted assets.
- Every corpus report prints how many archives, embedded entries, objects, and
  payloads it examined, plus every unsupported case. Zero matches is a refusal,
  not success.
- `x2_*`, `X2_*`, and `X2VIEW_*` names are extraction residue, not an ownership
  boundary. New public APIs use `alchemy_` or the engine's factual `ig*`
  vocabulary; migrate consumers atomically when retiring an old name.

## Structure

Keep container parsing, typed mesh/image interpretation, animation, input,
archive tooling, and viewers as separate owners. Tests call production seams;
they do not reimplement byte-order or layout rules. New first-party source
files are capped at 500 lines. Existing larger files are frozen until a
cohesive owner is extracted, then their ceiling is lowered.

The proven C parsers and pure transformations stay C; do not rewrite them for
style. New stateful engine owners and guest-facing adapters use focused C++20
RAII classes, explicit constructor dependencies, narrow APIs, and composition.
No shared owner may become a service locator or title-policy container.

Shipping library code never reads process environment variables or writes
directly to stderr/platform debug output. Viewer/tool entry points may ingest
CLI or environment configuration through one typed configuration owner, then
pass immutable options into the library. Libraries emit typed diagnostic
events through an injected observer; the application translates those events
at one boundary into its configurable Lucent logger. The normal structure gate
rejects `getenv`, direct stderr/debug output, title vocabulary, forbidden
consumer dependencies, and source growth beyond the recorded limits.

Agent verification uses Clang in a top-level `build/` child. `scratch/` is for
disposable run evidence, never compiler output.
