---
id: C003
kind: claim
status: holds
created: 2026-08-24
tags: input,sdl,controller
depends: src/input/sdl_controller.cpp#SdlControllerBackend::handleEvent, src/input/sdl_controller.cpp#SdlControllerBackend::update, src/input/controller.cpp#ControllerManager::disconnect, tests/test_sdl_input.cpp#testSdlLifecycleAndSnapshot
---

## Claim

The native Alchemy SDL controller backend enumerates devices present at initialization, admits a later device through the centrally forwarded SDL event path without polling the global event queue, publishes button/stick snapshots, and keeps an unaffected device in its stable slot when another disconnects.

## Evidence

2026-09-04 Clang RelWithDebInfo `sdl_input` passed its real SDL3 virtual-device path: one device was
enumerated before event forwarding, a second was attached and admitted after initialization,
button/stick state was published through `SdlControllerBackend::update`, removing the first left the
second in its stable slot, and RAII teardown emitted matching disconnects without shutting down a
host-owned SDL subsystem. The platform-neutral `input` test independently passed externally supplied
snapshots, lifecycle events, capacity refusal, and slot reuse. The structure self-test accepted the
shipping sources and rejected synthetic SDL event polling, title/consumer coupling, process
configuration, and direct diagnostics.

## What would falsify it

A real SDL device or supported host fails startup/late admission, event forwarding, snapshot publication, or stable identity; SDL input source/test/gate changes without rerunning the controller and structure tests; or the production backend regains its own SDL event pump.
