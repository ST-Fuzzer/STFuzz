#pragma once

#include <cstdint>

#include "core/device_manager/DeviceTypes.hpp"

namespace stfuzz {
namespace core {
namespace device_manager {

enum class DeviceEvent : std::uint8_t { kAdded, kUpdated, kDisconnected };

class IDeviceSubscriber {
 public:
  virtual ~IDeviceSubscriber() = default;

  virtual void UpdateDeviceEvent(DeviceEvent event,
                                 const DeviceState& device) = 0;
};

}  // namespace device_manager
}  // namespace core
}  // namespace stfuzz
