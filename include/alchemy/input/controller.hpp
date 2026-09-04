#ifndef ALCHEMY_INPUT_CONTROLLER_HPP
#define ALCHEMY_INPUT_CONTROLLER_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace alchemy::input {

inline constexpr std::size_t kMaxControllerCount = 4;
inline constexpr std::size_t kButtonCount = 32;
inline constexpr std::size_t kStickCount = 2;

enum class Button : std::uint16_t {
  select = 0,
  leftStick = 1,
  rightStick = 2,
  start = 3,
  dpadUp = 4,
  dpadRight = 5,
  dpadDown = 6,
  dpadLeft = 7,
  leftTrigger = 8,
  rightTrigger = 9,
  leftShoulder = 10,
  rightShoulder = 11,
  faceUp = 12,
  faceRight = 13,
  faceDown = 14,
  faceLeft = 15,
  button16 = 16,
  button17 = 17,
  button18 = 18,
  button19 = 19,
  button20 = 20,
  button21 = 21,
  button22 = 22,
  button23 = 23,
  button24 = 24,
  button25 = 25,
  button26 = 26,
  button27 = 27,
  button28 = 28,
  button29 = 29,
  button30 = 30,
  button31 = 31,
  unmapped = 0xffff,
};

enum class ControllerType : std::uint8_t {
  unknown = 0,
  ps2Pelican16Buttons,
  ps2Smartjoy12Buttons,
  ps2Xseries12Buttons,
  ps2Elecom12ButtonsPov,
  ps2Elecom16Buttons,
  ps2Sanwa16Buttons,
  xbox360Microsoft10ButtonsPov,
};

struct DeviceId {
  std::uint32_t value = 0;

  [[nodiscard]] explicit constexpr operator bool() const noexcept { return value != 0; }
  friend constexpr bool operator==(DeviceId, DeviceId) = default;
};

struct SlotId {
  std::uint8_t value = 0;

  friend constexpr bool operator==(SlotId, SlotId) = default;
};

struct Stick {
  float x = 0.0F;
  float y = 0.0F;

  friend constexpr bool operator==(Stick, Stick) = default;
};

class ControllerState {
public:
  [[nodiscard]] std::uint32_t buttons() const noexcept;
  [[nodiscard]] bool pressed(Button button) const noexcept;
  [[nodiscard]] float pressure(Button button) const noexcept;
  [[nodiscard]] Stick stick(std::size_t index) const noexcept;

  void setPressed(Button button, bool pressed) noexcept;
  void setPressure(Button button, float pressure) noexcept;
  void setStick(std::size_t index, Stick value) noexcept;

private:
  [[nodiscard]] static std::optional<std::size_t> buttonIndex(Button button) noexcept;

  std::uint32_t buttons_ = 0;
  std::array<float, kButtonCount> pressure_{};
  std::array<Stick, kStickCount> sticks_{};
};

struct DeviceDescriptor {
  DeviceId id;
  ControllerType type = ControllerType::unknown;
  bool isConsole = false;
};

struct Controller {
  SlotId slot;
  DeviceDescriptor device;
  ControllerState state;
};

enum class ConnectionChange : std::uint8_t { connected, disconnected };

struct ConnectionEvent {
  ConnectionChange change;
  Controller controller;
};

class ConnectionObserver {
public:
  virtual ~ConnectionObserver() = default;
  virtual void onConnectionEvent(const ConnectionEvent &event) noexcept = 0;
};

enum class ConnectStatus : std::uint8_t { connected, invalidDevice, alreadyConnected, full };

struct ConnectResult {
  ConnectStatus status = ConnectStatus::invalidDevice;
  const Controller *controller = nullptr;
};

class ControllerManager {
public:
  explicit ControllerManager(ConnectionObserver *observer = nullptr) noexcept;

  ControllerManager(const ControllerManager &) = delete;
  ControllerManager &operator=(const ControllerManager &) = delete;
  ControllerManager(ControllerManager &&) = delete;
  ControllerManager &operator=(ControllerManager &&) = delete;

  [[nodiscard]] ConnectResult connect(const DeviceDescriptor &device) noexcept;
  [[nodiscard]] bool disconnect(DeviceId device) noexcept;
  [[nodiscard]] bool updateDescriptor(const DeviceDescriptor &device) noexcept;
  [[nodiscard]] bool publish(DeviceId device, const ControllerState &state) noexcept;
  void disconnectAll() noexcept;

  [[nodiscard]] std::size_t size() const noexcept;
  [[nodiscard]] const Controller *controllerAt(std::size_t connectedIndex) const noexcept;
  [[nodiscard]] const Controller *controllerInSlot(SlotId slot) const noexcept;
  [[nodiscard]] const Controller *find(DeviceId device) const noexcept;

private:
  [[nodiscard]] Controller *findMutable(DeviceId device) noexcept;
  void notify(ConnectionChange change, const Controller &controller) const noexcept;

  std::array<std::optional<Controller>, kMaxControllerCount> slots_{};
  std::size_t count_ = 0;
  ConnectionObserver *observer_ = nullptr;
};

} // namespace alchemy::input

#endif
