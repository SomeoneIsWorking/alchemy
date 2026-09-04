# Shared Alchemy runtime adoption

## Starting point

`shared/alchemy` is a partial library and tooling foundation. It already owns
measured IGB/image/mesh/animation code, XMLB/FB/ARK tools, a platform-neutral
controller model, and an SDL3 backend. X-Men 2 pins the repository and imports
some offline tools, but its gameplay target links no shared Alchemy library.
MUA has no dependency edge at all. Repository presence is therefore not engine
adoption.

## Ownership boundary

The shared repository owns title-neutral Alchemy semantics and their runtime
lifetime. The neutral targets depend on no CPU/console host. Optional,
separately selectable `alchemy/x86` and `alchemy/x360` adapters may consume the
corresponding host's public execution/context interfaces when a proven engine
contract needs that translation. CPU execution and console/OS services remain
in `x86port` or the Xbox 360 host; they do not move here. Each game still owns
its executable identity, hashes, guest addresses/layouts, native override
registration, player/game policy, and thin binding into the relevant adapter.

Do not design a universal engine before a real seam requires it. Begin with one
narrow interface already supported by binary evidence, retain the ordinary
guest path as the differential oracle, and move only the invariant contract.

## Migration order

1. The shipping library boundary is now free of title/viewer policy.
   `igb_scene_load` receives immutable `igb_scene_options` and emits typed
   `igb_scene_diagnostic` events to an injected observer; it no longer reads
   `X2VIEW_*` or writes stderr. The application maps diagnostics into Lucent.
   The structure gate rejects process configuration, direct diagnostics,
   forbidden consumer dependencies, and shared-library title vocabulary. The
   proven pure C parsers remain C, while the stateful input owner is a focused
   C++20 RAII API.
2. Make X-Men 2 the first gameplay consumer. Its one existing Alchemy pin must
   feed the runtime CMake dependency as well as tools. Link the required shared
   `alchemy::input` target and add a title-local adapter between
   `ControllerManager` snapshots and the retained PC `igControllerManager` ABI.
   If reusable x86port context translation is required, place only that part in
   the optional `alchemy/x86` layer; keep addresses and registration in X-Men 2.
3. A/B-verify the shipping X-Men 2 path against its retained DirectInput
   implementation: button bits, pressure, axes, callbacks, hotplug, stable
   identity, and resource lifetime must agree. Only then may the replaced
   title-local concrete implementation be removed.
4. Extend the shared surface one proven title-neutral service at a time as
   X-Men 2 completes its other project goals. A checkout, linked-but-unused
   target, unit-only call, or offline asset tool does not establish runtime
   ownership.
5. Keep MUA and `alchemy/x360` work deferred until every X-Men 2 project goal is
   verified. After that, add one MUA resolver/pin, recover its PPC/ARK binding,
   and introduce only reusable Alchemy-to-host translation in the optional
   `alchemy/x360` layer. Prove it calls the same shared contracts without moving
   MUA addresses, hashes, layouts, registration, or policy into this repository.

## Acceptance

- Consumer build graphs show the pinned shared target in the gameplay link
  closure, and product inspection identifies no duplicate title-local
  implementation of the adopted contract.
- A shipping-path integration check reaches the shared implementation; title
  A/B evidence proves semantics rather than merely counting calls.
- Wrong or unsupported ABI/layout cases refuse by complete identity.
- The shared normal verifier rejects direct environment reads and stderr/debug
  output in shipping library code, forbidden title dependencies, title
  vocabulary, and source growth beyond the project limits.
- X-Men 2 and, after its deferral lifts, MUA each retain one immutable Alchemy
  revision used consistently by tooling and runtime targets.
