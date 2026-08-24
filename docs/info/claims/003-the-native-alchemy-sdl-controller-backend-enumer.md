---
id: C003
kind: claim
status: holds
created: 2026-08-24
tags: input,sdl,controller
depends: src/ig_sdl_controller.c#ig_sdl_controller_handle_event, src/ig_sdl_controller.c#ig_sdl_controller_update, src/ig_controller.c#ig_controller_manager_disconnect, tests/test_controller.c#test_sdl_event_and_snapshot_path
---

## Claim

The native Alchemy SDL controller backend enumerates devices present at initialization, admits a later device through the centrally forwarded SDL event path without polling the global event queue, publishes button/stick snapshots, and keeps an unaffected device in its stable slot when another disconnects.

## Evidence

2026-08-24 Clang RelWithDebInfo test_controller passed its real SDL3 virtual-device path: one device was enumerated before event forwarding, a second was attached and admitted after initialization, button/stick state was read through ig_sdl_controller_update, and removing the first left the second pointer/slot unchanged; the structure and structure_selftest gates independently accepted the shipping backend and rejected synthetic SDL_PollEvent/title-vocabulary violations.

## What would falsify it

A real SDL device or supported host fails startup/late admission, event forwarding, snapshot publication, or stable identity; SDL input source/test/gate changes without rerunning the controller and structure tests; or the production backend regains its own SDL event pump.
