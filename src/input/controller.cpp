#include <alchemy/input/controller.hpp>

#include <algorithm>
#include <limits>

namespace alchemy::input {
namespace {

constexpr bool validButtonIndex(std::size_t index) { return index < kButtonCount; }

} // namespace

std::uint32_t ControllerState::buttons() const noexcept { return buttons_; }

bool ControllerState::pressed(Button button) const noexcept {
  const auto index = buttonIndex(button);
  return index && ((buttons_ >> *index) & 1U) != 0;
}

float ControllerState::pressure(Button button) const noexcept {
  const auto index = buttonIndex(button);
  return index ? pressure_[*index] : 0.0F;
}

Stick ControllerState::stick(std::size_t index) const noexcept {
  return index < sticks_.size() ? sticks_[index] : Stick{};
}

void ControllerState::setPressed(Button button, bool pressedValue) noexcept {
  const auto index = buttonIndex(button);
  if (!index) {
    return;
  }
  const auto mask = std::uint32_t{1} << *index;
  if (pressedValue) {
    buttons_ |= mask;
  } else {
    buttons_ &= ~mask;
  }
}

void ControllerState::setPressure(Button button, float pressureValue) noexcept {
  const auto index = buttonIndex(button);
  if (index) {
    pressure_[*index] = std::clamp(pressureValue, 0.0F, 1.0F);
  }
}

void ControllerState::setStick(std::size_t index, Stick value) noexcept {
  if (index < sticks_.size()) {
    sticks_[index] = value;
  }
}

std::optional<std::size_t> ControllerState::buttonIndex(Button button) noexcept {
  const auto index = static_cast<std::size_t>(button);
  return validButtonIndex(index) ? std::optional{index} : std::nullopt;
}

ControllerManager::ControllerManager(ConnectionObserver *observer) noexcept : observer_(observer) {}

ConnectResult ControllerManager::connect(const DeviceDescriptor &device) noexcept {
  if (!device.id) {
    return {ConnectStatus::invalidDevice, nullptr};
  }
  if (find(device.id) != nullptr) {
    return {ConnectStatus::alreadyConnected, nullptr};
  }

  for (std::size_t index = 0; index < slots_.size(); ++index) {
    if (slots_[index]) {
      continue;
    }
    Controller &connected = slots_[index].emplace(
        Controller{SlotId{static_cast<std::uint8_t>(index)}, device, ControllerState{}});
    ++count_;
    notify(ConnectionChange::connected, connected);
    return {ConnectStatus::connected, &connected};
  }
  return {ConnectStatus::full, nullptr};
}

bool ControllerManager::disconnect(DeviceId device) noexcept {
  for (auto &slot : slots_) {
    if (!slot || slot->device.id != device) {
      continue;
    }
    const Controller disconnected = *slot;
    slot.reset();
    --count_;
    notify(ConnectionChange::disconnected, disconnected);
    return true;
  }
  return false;
}

bool ControllerManager::updateDescriptor(const DeviceDescriptor &device) noexcept {
  Controller *controller = findMutable(device.id);
  if (controller == nullptr || !device.id) {
    return false;
  }
  controller->device = device;
  return true;
}

bool ControllerManager::publish(DeviceId device, const ControllerState &state) noexcept {
  Controller *controller = findMutable(device);
  if (controller == nullptr) {
    return false;
  }
  controller->state = state;
  return true;
}

void ControllerManager::disconnectAll() noexcept {
  for (auto &slot : slots_) {
    if (!slot) {
      continue;
    }
    const DeviceId device = slot->device.id;
    static_cast<void>(disconnect(device));
  }
}

std::size_t ControllerManager::size() const noexcept { return count_; }

const Controller *ControllerManager::controllerAt(std::size_t connectedIndex) const noexcept {
  if (connectedIndex >= count_) {
    return nullptr;
  }
  for (const auto &slot : slots_) {
    if (!slot) {
      continue;
    }
    if (connectedIndex-- == 0) {
      return &*slot;
    }
  }
  return nullptr;
}

const Controller *ControllerManager::controllerInSlot(SlotId slot) const noexcept {
  const auto index = static_cast<std::size_t>(slot.value);
  if (index >= slots_.size() || !slots_[index]) {
    return nullptr;
  }
  return slots_[index].operator->();
}

const Controller *ControllerManager::find(DeviceId device) const noexcept {
  for (const auto &slot : slots_) {
    if (slot && slot->device.id == device) {
      return &*slot;
    }
  }
  return nullptr;
}

Controller *ControllerManager::findMutable(DeviceId device) noexcept {
  for (auto &slot : slots_) {
    if (slot && slot->device.id == device) {
      return &*slot;
    }
  }
  return nullptr;
}

void ControllerManager::notify(ConnectionChange change,
                               const Controller &controller) const noexcept {
  if (observer_ != nullptr) {
    observer_->onConnectionEvent(ConnectionEvent{change, controller});
  }
}

} // namespace alchemy::input
