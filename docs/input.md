# Native Alchemy input boundary

## What the shipped engine separates

The committed X-Men Legends II export in the consuming port is the current
ground truth for ownership. `libIGDisplay.ark.json` records abstract
`igController` (16 bytes) and `igControllerManager` (12 bytes), then maps them
to concrete `igWin32Controller` (148 bytes) and `igWin32ControllerManager`
through its implementation table. `libIGDisplay.json` independently names the
generic manager operations at `0x10002050`–`0x100020f0`, the generic event
collection at `0x10004800`, and the DirectInput implementation at
`igWin32Controller::getEvents` (`0x10004c80`) and
`igWin32ControllerManager::createControllers` (`0x100052a0`). The DLL imports
`DirectInputCreateEx` for that concrete Win32 layer.

That evidence establishes a native replacement boundary: the shared engine
owns controller devices and state; SDL replaces the Win32/DirectInput concrete
implementation. A game executable still owns action meanings, joining and
leaving players, assignment policy, and prompt choice.

## Current native slice

- `ig_controller` is the platform-neutral current snapshot: connection,
  Alchemy button bits and pressure, two sticks, controller type, and an opaque
  backend identity.
- `ig_controller_manager` keeps four stable slots. Disconnecting one device
  does not move the remaining devices, so a title adapter can retain a device
  identity across hotplug without confusing it with player order.
- `ig_sdl_controller_initialize` admits devices present at startup.
- The application forwards its centrally-polled SDL events to
  `ig_sdl_controller_handle_event`; a late `SDL_EVENT_GAMEPAD_ADDED` goes
  through the same admission path as startup enumeration.
- `ig_sdl_controller_update` reads a complete SDL state snapshot. Event
  delivery therefore owns lifecycle and immediate changes, while the snapshot
  prevents state from depending on every motion event surviving unrelated UI
  consumers.
- `ig_sdl_controller_shutdown` and `ig_sdl_controller_set_rumble` own SDL
  handles and output. The generic model never includes an SDL header.

The controller test uses two virtual devices to exercise both startup
enumeration and late attachment through the production path. Its event pump is
deliberately in the test host, not in the backend. The structure gate refuses
an `SDL_PollEvent` call in the backend and refuses reintroduction of `x2_`/`X2_`
vocabulary into the shared input sources.

## Compatibility and remaining work

This state model is not a claim that the guest object ABI has been reproduced.
The ARK class graph and concrete substitution for MUA remain unverified, and
X-Men Legends II still needs a narrow adapter between these native snapshots
and the guest `igControllerManager` surface. Keep its existing DirectInput path
as the oracle until an A/B run proves button bits, pressure, axes, lifecycle,
and callbacks agree. Then retire the Win32 concrete adapter; do not move title
action or player policy into this repository as part of that retirement.
