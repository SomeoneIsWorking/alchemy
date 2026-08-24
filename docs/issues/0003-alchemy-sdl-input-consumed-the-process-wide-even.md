---
id: 3
title: Alchemy SDL input consumed the process-wide event queue
status: resolved
symptom: Controller hotplug or UI/window events can disappear when the shared Alchemy input backend polls SDL independently; the shared API also exposes X-Men-specific x2 names.
tags: input,sdl,hotplug,architecture
created: 2026-08-24
updated: 2026-08-24
---

## Root cause

`src/ig_sdl_controller.c` called `SDL_PollEvent` itself. SDL has one process-wide queue, so a reusable input backend could consume unrelated window/UI events or gamepad events before the host composed other consumers. The extracted shared API also retained `x2_`/`X2_` names even though the binary separates generic `igController`/`igControllerManager` from the Win32 implementation.

## Resolution

The host now owns the only event pump and forwards events through `ig_sdl_controller_handle_event`. Startup enumeration and `ig_sdl_controller_update` provide complete snapshots, including controllers attached after initialization. Generic state uses `ig_*` names and stable device slots; SDL code is a separate `alchemy_input_sdl` target. The structure gate rejects both `SDL_PollEvent` in the backend and title-specific input vocabulary.

## Evidence

The Clang RelWithDebInfo controller test passed startup enumeration, centrally forwarded late attach/removal, button/stick snapshot publication, and stable identity with SDL virtual devices. `structure` and `structure_selftest` passed positive and negative cases. See C003 and `docs/input.md`.
