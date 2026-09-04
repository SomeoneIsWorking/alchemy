#include <alchemy/input/sdl_controller.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace alchemy::input {
namespace {

struct GamepadEntry {
  DeviceId device;
  SDL_Gamepad *handle = nullptr;
};

float normalizeStick(Sint16 value, float deadzone) noexcept {
  const float normalized =
      value >= 0 ? static_cast<float>(value) / 32767.0F : static_cast<float>(value) / 32768.0F;
  return normalized > -deadzone && normalized < deadzone ? 0.0F : normalized;
}

float normalizeTrigger(Sint16 value) noexcept {
  return value <= 0 ? 0.0F : static_cast<float>(value) / 32767.0F;
}

ControllerType detectType(SDL_Gamepad *gamepad) noexcept {
  switch (SDL_GetGamepadType(gamepad)) {
  case SDL_GAMEPAD_TYPE_XBOX360:
  case SDL_GAMEPAD_TYPE_XBOXONE:
    return ControllerType::xbox360Microsoft10ButtonsPov;
  default:
    return ControllerType::unknown;
  }
}

bool validUnitInterval(float value) noexcept { return value >= 0.0F && value <= 1.0F; }

} // namespace

struct SdlControllerBackend::Impl {
  ControllerManager &controllers;
  const SdlControllerSettings settings;
  SdlDiagnosticObserver &diagnostics;
  std::array<GamepadEntry, kMaxControllerCount> gamepads{};
  bool initialized = false;
  bool ownsSubsystem = false;

  void report(SdlDiagnosticSeverity severity, SdlOperation operation, DeviceId device,
              std::string detail) const noexcept {
    diagnostics.onSdlDiagnostic(SdlDiagnostic{severity, operation, device, std::move(detail)});
  }

  [[nodiscard]] GamepadEntry *findEntry(DeviceId device) noexcept {
    for (auto &entry : gamepads) {
      if (entry.handle != nullptr && entry.device == device) {
        return &entry;
      }
    }
    return nullptr;
  }

  [[nodiscard]] GamepadEntry *emptyEntry() noexcept {
    for (auto &entry : gamepads) {
      if (entry.handle == nullptr) {
        return &entry;
      }
    }
    return nullptr;
  }

  void addController(SDL_JoystickID joystickId) {
    const DeviceId eventDevice{static_cast<std::uint32_t>(joystickId)};
    if (controllers.find(eventDevice) != nullptr) {
      return;
    }

    SDL_Gamepad *gamepad = SDL_OpenGamepad(joystickId);
    if (gamepad == nullptr) {
      report(SdlDiagnosticSeverity::warning, SdlOperation::openDevice, eventDevice, SDL_GetError());
      return;
    }

    const DeviceId device{static_cast<std::uint32_t>(SDL_GetGamepadID(gamepad))};
    GamepadEntry *entry = emptyEntry();
    if (!device || controllers.find(device) != nullptr || entry == nullptr) {
      SDL_CloseGamepad(gamepad);
      report(SdlDiagnosticSeverity::warning, SdlOperation::openDevice, device,
             entry == nullptr ? "controller capacity is full"
                              : "controller identity is invalid or already present");
      return;
    }
    const ConnectResult result =
        controllers.connect(DeviceDescriptor{device, detectType(gamepad), true});
    if (result.status != ConnectStatus::connected) {
      SDL_CloseGamepad(gamepad);
      report(SdlDiagnosticSeverity::warning, SdlOperation::openDevice, device,
             "controller registry rejected the device");
      return;
    }
    *entry = GamepadEntry{device, gamepad};
  }

  void removeController(DeviceId device) noexcept {
    GamepadEntry *entry = findEntry(device);
    if (entry == nullptr) {
      return;
    }
    static_cast<void>(controllers.disconnect(device));
    SDL_CloseGamepad(entry->handle);
    *entry = {};
  }

  void setButton(DeviceId device, SDL_GamepadButton button, bool pressed) noexcept {
    const Controller *controller = controllers.find(device);
    const Button mapped = SdlControllerBackend::mapButton(button);
    if (controller == nullptr || mapped == Button::unmapped) {
      return;
    }
    ControllerState state = controller->state;
    state.setPressed(mapped, pressed);
    state.setPressure(mapped, pressed ? 1.0F : 0.0F);
    static_cast<void>(controllers.publish(device, state));
  }

  void setAxis(DeviceId device, SDL_GamepadAxis axis, Sint16 value) noexcept {
    const Controller *controller = controllers.find(device);
    if (controller == nullptr) {
      return;
    }
    ControllerState state = controller->state;
    switch (axis) {
    case SDL_GAMEPAD_AXIS_LEFTX:
      state.setStick(0, {normalizeStick(value, settings.stickDeadzone), state.stick(0).y});
      break;
    case SDL_GAMEPAD_AXIS_LEFTY:
      state.setStick(0, {state.stick(0).x, normalizeStick(value, settings.stickDeadzone)});
      break;
    case SDL_GAMEPAD_AXIS_RIGHTX:
      state.setStick(1, {normalizeStick(value, settings.stickDeadzone), state.stick(1).y});
      break;
    case SDL_GAMEPAD_AXIS_RIGHTY:
      state.setStick(1, {state.stick(1).x, normalizeStick(value, settings.stickDeadzone)});
      break;
    case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
      setTrigger(state, Button::leftTrigger, value);
      break;
    case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
      setTrigger(state, Button::rightTrigger, value);
      break;
    default:
      return;
    }
    static_cast<void>(controllers.publish(device, state));
  }

  void setTrigger(ControllerState &state, Button button, Sint16 value) const noexcept {
    const float pressure = normalizeTrigger(value);
    state.setPressure(button, pressure);
    state.setPressed(button, pressure > settings.triggerButtonThreshold);
  }
};

SdlControllerBackend::SdlControllerBackend(ControllerManager &controllers,
                                           SdlDiagnosticObserver &diagnostics,
                                           SdlControllerSettings settings)
    : impl_(std::make_unique<Impl>(Impl{controllers, settings, diagnostics})) {
  if (!validUnitInterval(settings.stickDeadzone) ||
      !validUnitInterval(settings.triggerButtonThreshold)) {
    throw std::invalid_argument("SDL controller thresholds must be within [0, 1]");
  }
}

SdlControllerBackend::~SdlControllerBackend() { shutdown(); }

bool SdlControllerBackend::initialize() {
  if (impl_->initialized) {
    return true;
  }
  if ((SDL_WasInit(SDL_INIT_GAMEPAD) & SDL_INIT_GAMEPAD) == 0) {
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
      impl_->report(SdlDiagnosticSeverity::error, SdlOperation::initializeSubsystem, {},
                    SDL_GetError());
      return false;
    }
    impl_->ownsSubsystem = true;
  }

  int count = 0;
  SDL_JoystickID *gamepads = SDL_GetGamepads(&count);
  if (gamepads == nullptr && count != 0) {
    impl_->report(SdlDiagnosticSeverity::error, SdlOperation::enumerateDevices, {}, SDL_GetError());
    if (impl_->ownsSubsystem) {
      SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
      impl_->ownsSubsystem = false;
    }
    return false;
  }
  impl_->initialized = true;
  for (int index = 0; index < count; ++index) {
    impl_->addController(gamepads[index]);
  }
  SDL_free(gamepads);
  return true;
}

void SdlControllerBackend::handleEvent(const SDL_Event &event) {
  switch (event.type) {
  case SDL_EVENT_GAMEPAD_ADDED:
    impl_->addController(event.gdevice.which);
    break;
  case SDL_EVENT_GAMEPAD_REMOVED:
    impl_->removeController(DeviceId{static_cast<std::uint32_t>(event.gdevice.which)});
    break;
  case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
  case SDL_EVENT_GAMEPAD_BUTTON_UP:
    impl_->setButton(DeviceId{static_cast<std::uint32_t>(event.gbutton.which)},
                     static_cast<SDL_GamepadButton>(event.gbutton.button),
                     event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
    break;
  case SDL_EVENT_GAMEPAD_AXIS_MOTION:
    impl_->setAxis(DeviceId{static_cast<std::uint32_t>(event.gaxis.which)},
                   static_cast<SDL_GamepadAxis>(event.gaxis.axis), event.gaxis.value);
    break;
  case SDL_EVENT_GAMEPAD_REMAPPED: {
    const DeviceId device{static_cast<std::uint32_t>(event.gdevice.which)};
    const auto *entry = impl_->findEntry(device);
    const auto *controller = impl_->controllers.find(device);
    if (entry != nullptr && controller != nullptr) {
      static_cast<void>(impl_->controllers.updateDescriptor(
          DeviceDescriptor{device, detectType(entry->handle), controller->device.isConsole}));
    }
    break;
  }
  default:
    break;
  }
}

void SdlControllerBackend::update() {
  for (const auto &entry : impl_->gamepads) {
    if (entry.handle == nullptr || impl_->controllers.find(entry.device) == nullptr) {
      continue;
    }
    for (int button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; ++button) {
      impl_->setButton(entry.device, static_cast<SDL_GamepadButton>(button),
                       SDL_GetGamepadButton(entry.handle, static_cast<SDL_GamepadButton>(button)));
    }
    for (int axis = 0; axis < SDL_GAMEPAD_AXIS_COUNT; ++axis) {
      impl_->setAxis(entry.device, static_cast<SDL_GamepadAxis>(axis),
                     SDL_GetGamepadAxis(entry.handle, static_cast<SDL_GamepadAxis>(axis)));
    }
  }
}

void SdlControllerBackend::shutdown() noexcept {
  if (!impl_) {
    return;
  }
  for (auto &entry : impl_->gamepads) {
    if (entry.handle != nullptr) {
      impl_->removeController(entry.device);
    }
  }
  if (impl_->ownsSubsystem) {
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
  }
  impl_->ownsSubsystem = false;
  impl_->initialized = false;
}

bool SdlControllerBackend::setRumble(DeviceId device, RumbleMotor motor, float speed) {
  GamepadEntry *entry = impl_->findEntry(device);
  if (entry == nullptr) {
    return false;
  }
  const auto intensity = static_cast<std::uint16_t>(std::clamp(speed, 0.0F, 1.0F) * 65535.0F);
  const std::uint16_t low = motor == RumbleMotor::lowFrequency ? intensity : 0;
  const std::uint16_t high = motor == RumbleMotor::highFrequency ? intensity : 0;
  if (!SDL_RumbleGamepad(entry->handle, low, high, 0)) {
    impl_->report(SdlDiagnosticSeverity::warning, SdlOperation::rumble, device, SDL_GetError());
    return false;
  }
  return true;
}

Button SdlControllerBackend::mapButton(SDL_GamepadButton button) noexcept {
  switch (button) {
  case SDL_GAMEPAD_BUTTON_SOUTH:
    return Button::faceDown;
  case SDL_GAMEPAD_BUTTON_EAST:
    return Button::faceRight;
  case SDL_GAMEPAD_BUTTON_WEST:
    return Button::faceLeft;
  case SDL_GAMEPAD_BUTTON_NORTH:
    return Button::faceUp;
  case SDL_GAMEPAD_BUTTON_BACK:
    return Button::select;
  case SDL_GAMEPAD_BUTTON_START:
    return Button::start;
  case SDL_GAMEPAD_BUTTON_LEFT_STICK:
    return Button::leftStick;
  case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
    return Button::rightStick;
  case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
    return Button::leftShoulder;
  case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
    return Button::rightShoulder;
  case SDL_GAMEPAD_BUTTON_DPAD_UP:
    return Button::dpadUp;
  case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
    return Button::dpadDown;
  case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
    return Button::dpadLeft;
  case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
    return Button::dpadRight;
  case SDL_GAMEPAD_BUTTON_GUIDE:
    return Button::button16;
  default:
    return Button::unmapped;
  }
}

} // namespace alchemy::input
