#include <alchemy/input/controller.hpp>

#include <cassert>
#include <cstdint>
#include <iostream>
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

DeviceDescriptor device(std::uint32_t id) {
  return DeviceDescriptor{DeviceId{id}, ControllerType::unknown, true};
}

void testStateModel() {
  ControllerState state;
  state.setPressed(Button::start, true);
  state.setPressed(Button::faceDown, true);
  state.setPressure(Button::leftTrigger, 2.0F);
  state.setPressure(Button::rightTrigger, -1.0F);
  state.setStick(1, {1.0F, -1.0F});

  assert(state.pressed(Button::start));
  assert(state.pressed(Button::faceDown));
  assert(!state.pressed(Button::unmapped));
  assert(state.buttons() == ((1U << 3U) | (1U << 14U)));
  assert(state.pressure(Button::leftTrigger) == 1.0F);
  assert(state.pressure(Button::rightTrigger) == 0.0F);
  assert(state.pressure(Button::unmapped) == 0.0F);
  assert(state.stick(1) == (Stick{1.0F, -1.0F}));
  assert(state.stick(2) == Stick{});

  state.setPressed(Button::start, false);
  assert(!state.pressed(Button::start));
  std::cout << "state: exact button bits, pressure bounds, and two axes passed\n";
}

void testManagerAndExternalSnapshot() {
  EventRecorder recorder;
  ControllerManager controllers(&recorder);

  assert(controllers.connect(device(0)).status == ConnectStatus::invalidDevice);
  const ConnectResult first = controllers.connect(device(10));
  const ConnectResult second = controllers.connect(device(11));
  assert(first.status == ConnectStatus::connected && first.controller->slot == SlotId{0});
  assert(second.status == ConnectStatus::connected && second.controller->slot == SlotId{1});
  assert(controllers.connect(device(10)).status == ConnectStatus::alreadyConnected);
  assert(recorder.events.size() == 2);
  assert(recorder.events[0].change == ConnectionChange::connected);
  assert(recorder.events[0].controller.device.id == DeviceId{10});

  ControllerState externalState;
  externalState.setPressed(Button::faceLeft, true);
  externalState.setPressure(Button::leftTrigger, 0.25F);
  externalState.setStick(0, {-0.75F, 0.5F});
  assert(controllers.publish(DeviceId{11}, externalState));
  const Controller *published = controllers.find(DeviceId{11});
  assert(published != nullptr);
  assert(published->state.pressed(Button::faceLeft));
  assert(published->state.pressure(Button::leftTrigger) == 0.25F);
  assert(published->state.stick(0) == (Stick{-0.75F, 0.5F}));

  assert(controllers.disconnect(DeviceId{10}));
  assert(recorder.events.back().change == ConnectionChange::disconnected);
  assert(recorder.events.back().controller.slot == SlotId{0});
  assert(controllers.find(DeviceId{11}) == published);

  const ConnectResult replacement = controllers.connect(device(12));
  assert(replacement.status == ConnectStatus::connected);
  assert(replacement.controller->slot == SlotId{0});
  assert(controllers.controllerAt(0) == replacement.controller);
  assert(controllers.controllerAt(1) == published);
  assert(controllers.controllerInSlot(SlotId{1}) == published);

  assert(controllers.connect(device(13)).status == ConnectStatus::connected);
  assert(controllers.connect(device(14)).status == ConnectStatus::connected);
  assert(controllers.connect(device(15)).status == ConnectStatus::full);
  controllers.disconnectAll();
  assert(controllers.size() == 0);
  assert(recorder.events.size() == 10);
  std::cout << "manager: stable slots, typed lifecycle, capacity, and external "
               "snapshots passed\n";
}

} // namespace

int main() {
  testStateModel();
  testManagerAndExternalSnapshot();
  std::cout << "all platform-neutral input tests passed\n";
  return 0;
}
