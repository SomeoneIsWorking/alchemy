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
lifetime. Each game owns its executable identity, guest addresses and object
layout, native override registration, player/game policy, and conformance
adapter. CPU execution and console/OS services remain in `x86port`,
`xenonport`, or the host layer; they do not move here.

Do not design a universal engine before a real seam requires it. Begin with one
narrow interface already supported by binary evidence, retain the ordinary
guest path as the differential oracle, and move only the invariant contract.

## Migration order

1. Remove title/viewer policy from the shipping library boundary.
   `src/igb_mesh.c` must receive typed immutable options instead of reading
   `X2VIEW_*` environment variables, and runtime diagnostics must use one
   injected configurable Lucent logger instead of stderr. Extend the structure
   gate to reject both regressions and shared-library title vocabulary. Keep
   the proven pure C parsers in C; use focused C++20 RAII owners for new
   stateful runtime services and adapters.
2. Make X-Men 2 the first gameplay consumer. Its one existing Alchemy pin must
   feed the runtime CMake dependency as well as tools. Link the required shared
   input target and add a title-local adapter between shared snapshots and the
   retained PC `igControllerManager` ABI.
3. A/B-verify the shipping X-Men 2 path against its retained DirectInput
   implementation: button bits, pressure, axes, callbacks, hotplug, stable
   identity, and resource lifetime must agree. Only then may the replaced
   title-local concrete implementation be removed.
4. Extend the shared surface one proven title-neutral service at a time as
   X-Men 2 completes its other project goals. A checkout, linked-but-unused
   target, unit-only call, or offline asset tool does not establish runtime
   ownership.
5. Keep MUA deferred until every X-Men 2 project goal is verified. After that,
   add one MUA resolver/pin and its separately recovered PPC/ARK ABI adapter,
   then prove it calls the same shared contracts without moving MUA addresses
   or policy into this repository.

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
