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

The host owns the only event pump and forwards events through `SdlControllerBackend::handleEvent`.
Startup enumeration and `update` provide complete snapshots, including controllers attached after
initialization. Generic typed state and stable device slots live in the SDL-free `alchemy::input`
target; SDL handles and lifetime live in the separate RAII `alchemy::input_sdl` target. The
structure gate rejects backend `SDL_PollEvent`, title vocabulary, direct configuration/output, and
consumer dependencies.

## Evidence

The Clang RelWithDebInfo input tests pass platform-neutral external snapshot publication and SDL
startup enumeration, centrally forwarded late attach/removal, button/stick snapshots, RAII cleanup,
and stable identity with SDL virtual devices. `structure` and `structure_selftest` pass positive and
negative cases. See C003 and `docs/input.md`.
