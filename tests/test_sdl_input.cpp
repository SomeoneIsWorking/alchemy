#include <alchemy/input/sdl_controller.hpp>

#include <SDL3/SDL.h>

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace alchemy::input;

class EventRecorder final : public ConnectionObserver {
public:
  void onConnectionEvent(const ConnectionEvent &event) noexcept override {
    events.push_back(event);
  }

  std::vector<ConnectionEvent> events;
};

class DiagnosticRecorder final : public SdlDiagnosticObserver {
public:
  void onSdlDiagnostic(const SdlDiagnostic &diagnostic) noexcept override {
    diagnostics.push_back(diagnostic);
  }

  std::vector<SdlDiagnostic> diagnostics;
};

SDL_JoystickID attachVirtualGamepad() {
  SDL_VirtualJoystickDesc descriptor;
  SDL_INIT_INTERFACE(&descriptor);
  descriptor.type = SDL_JOYSTICK_TYPE_GAMEPAD;
  descriptor.naxes = SDL_GAMEPAD_AXIS_COUNT;
  descriptor.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
  descriptor.nhats = 1;
  return SDL_AttachVirtualJoystick(&descriptor);
}

void addVirtualMapping(SDL_JoystickID id, const char *name) {
  const SDL_GUID guid = SDL_GetJoystickGUIDForID(id);
  char guidText[64];
  char mapping[512];
  SDL_GUIDToString(guid, guidText, sizeof(guidText));
  SDL_snprintf(mapping, sizeof(mapping),
               "%s,%s,a:b0,b:b1,x:b2,y:b3,back:b4,start:b6,"
               "leftshoulder:b9,rightshoulder:b10,leftstick:b7,rightstick:b8,"
               "dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,"
               "leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4,righttrigger:a5,",
               guidText, name);
  assert(SDL_AddGamepadMapping(mapping) >= 0);
}

int forwardPendingEvents(SdlControllerBackend &backend) {
  SDL_Event event;
  int forwarded = 0;
  while (SDL_PollEvent(&event)) {
    backend.handleEvent(event);
    ++forwarded;
  }
  return forwarded;
}

void testMappingAndSettings() {
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_SOUTH) == Button::faceDown);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_EAST) == Button::faceRight);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_WEST) == Button::faceLeft);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_NORTH) == Button::faceUp);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_BACK) == Button::select);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_START) == Button::start);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_LEFT_STICK) == Button::leftStick);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_RIGHT_STICK) == Button::rightStick);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) == Button::leftShoulder);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) ==
         Button::rightShoulder);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_DPAD_UP) == Button::dpadUp);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_DPAD_DOWN) == Button::dpadDown);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_DPAD_LEFT) == Button::dpadLeft);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_DPAD_RIGHT) == Button::dpadRight);
  assert(SdlControllerBackend::mapButton(SDL_GAMEPAD_BUTTON_MISC1) == Button::unmapped);

  ControllerManager controllers;
  DiagnosticRecorder diagnostics;
  bool rejected = false;
  try {
    SdlControllerBackend invalid(controllers, diagnostics, {.stickDeadzone = 1.1F});
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  assert(rejected);
  std::cout << "SDL mapping: 15 mapped buttons, unmapped input, and config "
               "refusal passed\n";
}

void testSdlLifecycleAndSnapshot() {
  EventRecorder events;
  DiagnosticRecorder diagnostics;
  ControllerManager controllers(&events);

  assert(SDL_InitSubSystem(SDL_INIT_GAMEPAD));
  const SDL_JoystickID startupId = attachVirtualGamepad();
  assert(startupId != 0);
  addVirtualMapping(startupId, "Startup Virtual Controller");

  {
    SdlControllerBackend backend(controllers, diagnostics);
    assert(backend.initialize());
    assert(backend.initialize());
    assert(controllers.size() == 1);
    assert(events.events.size() == 1);
    const DeviceId startupDevice{static_cast<std::uint32_t>(startupId)};
    const Controller *startup = controllers.find(startupDevice);
    assert(startup != nullptr);

    SDL_Event unrelated{};
    unrelated.type = SDL_EVENT_USER;
    backend.handleEvent(unrelated);
    assert(controllers.size() == 1);

    SDL_Joystick *joystick = SDL_OpenJoystick(startupId);
    assert(joystick != nullptr);
    assert(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true));
    assert(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHTX, 32767));
    forwardPendingEvents(backend);
    backend.update();
    startup = controllers.find(startupDevice);
    assert(startup != nullptr && startup->state.pressed(Button::faceDown));
    assert(startup->state.stick(1).x > 0.99F);
    assert(startup->state.stick(1).y == 0.0F);

    const SDL_JoystickID lateId = attachVirtualGamepad();
    assert(lateId != 0);
    addVirtualMapping(lateId, "Late Virtual Controller");
    assert(forwardPendingEvents(backend) > 0);
    assert(controllers.size() == 2);
    const DeviceId lateDevice{static_cast<std::uint32_t>(lateId)};
    const Controller *late = controllers.find(lateDevice);
    assert(late != nullptr);
    const SlotId lateSlot = late->slot;

    SDL_CloseJoystick(joystick);
    assert(SDL_DetachVirtualJoystick(startupId));
    assert(forwardPendingEvents(backend) > 0);
    assert(controllers.size() == 1);
    assert(controllers.find(lateDevice)->slot == lateSlot);

    assert(SDL_DetachVirtualJoystick(lateId));
    forwardPendingEvents(backend);
  }

  assert(controllers.size() == 0);
  assert(events.events.size() == 4);
  assert(diagnostics.diagnostics.empty());
  assert((SDL_WasInit(SDL_INIT_GAMEPAD) & SDL_INIT_GAMEPAD) != 0);
  SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
  std::cout << "SDL path: startup, central events, snapshots, hotplug, and "
               "RAII passed\n";
}

} // namespace

int main() {
  testMappingAndSettings();
  testSdlLifecycleAndSnapshot();
  std::cout << "all SDL input tests passed\n";
  return 0;
}
