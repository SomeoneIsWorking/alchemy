#ifndef ALCHEMY_INPUT_SDL_CONTROLLER_HPP
#define ALCHEMY_INPUT_SDL_CONTROLLER_HPP

#include <SDL3/SDL_events.h>

#include <alchemy/input/controller.hpp>

#include <memory>
#include <string>

namespace alchemy::input {

enum class SdlDiagnosticSeverity : std::uint8_t { warning, error };
enum class SdlOperation : std::uint8_t {
  initializeSubsystem,
  enumerateDevices,
  openDevice,
  rumble,
};

struct SdlDiagnostic {
  SdlDiagnosticSeverity severity;
  SdlOperation operation;
  DeviceId device;
  std::string detail;
};

class SdlDiagnosticObserver {
public:
  virtual ~SdlDiagnosticObserver() = default;
  virtual void onSdlDiagnostic(const SdlDiagnostic &diagnostic) noexcept = 0;
};

struct SdlControllerSettings {
  float stickDeadzone = 0.1F;
  float triggerButtonThreshold = 0.5F;
};

enum class RumbleMotor : std::uint8_t { lowFrequency, highFrequency };

class SdlControllerBackend {
public:
  SdlControllerBackend(ControllerManager &controllers, SdlDiagnosticObserver &diagnostics,
                       SdlControllerSettings settings = {});
  ~SdlControllerBackend();

  SdlControllerBackend(const SdlControllerBackend &) = delete;
  SdlControllerBackend &operator=(const SdlControllerBackend &) = delete;
  SdlControllerBackend(SdlControllerBackend &&) = delete;
  SdlControllerBackend &operator=(SdlControllerBackend &&) = delete;

  [[nodiscard]] bool initialize();
  void handleEvent(const SDL_Event &event);
  void update();
  void shutdown() noexcept;
  [[nodiscard]] bool setRumble(DeviceId device, RumbleMotor motor, float speed);

  [[nodiscard]] static Button mapButton(SDL_GamepadButton button) noexcept;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace alchemy::input

#endif
