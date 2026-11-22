#pragma once

#include <cstdint>

namespace stfuzz::core {

struct DeviceState;

// What happened to a device. Part of the context passed to Update().
enum class DeviceEvent : std::uint8_t { kAdded, kUpdated, kDisconnected };

// Subscriber interface. Every class that wants to be told about device changes
// implements Update() and subscribes itself to a DeviceManager (the publisher).
class IDeviceSubscriber {
 public:
  virtual ~IDeviceSubscriber() = default;

  // Called by the publisher for every event, with the event details as context.
  virtual void Update(DeviceEvent event, const DeviceState& device) = 0;
};

}  // namespace stfuzz::core
