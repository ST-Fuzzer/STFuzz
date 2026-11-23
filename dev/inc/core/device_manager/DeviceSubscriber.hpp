#pragma once

#include <cstdint>

namespace stfuzz {
namespace core {
namespace device_manager {

struct DeviceState;

enum class DeviceEvent : std::uint8_t { kAdded, kUpdated, kDisconnected };

class IDeviceSubscriber {
 public:
  virtual ~IDeviceSubscriber() = default;

  virtual void Update(DeviceEvent event, const DeviceState& device) = 0;
};

}  // namespace device_manager
}  // namespace core
}  // namespace stfuzz

