#include "core/adaptor/injector/InjectorConfig.hpp"

#include <string>
#include <unordered_map>

#include "core/adaptor/AdaptorConfigInterface.hpp"
#include "core/adaptor/injector/data/Entry.hpp"
#include "core/adaptor/injector/data/stm32wb55/Common.hpp"
#include "core/adaptor/injector/data/stm32wb55/Ieee802154.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {

namespace {

template <typename Table>
void AddAll(ConfigMap& map, const Table& table) {
  for (const auto& entry : table) {
    map.emplace(std::string(entry.key), std::string(entry.value));
  }
}

}  // namespace

InjectorConfig::InjectorConfig(Device device, Protocol protocol) {
  switch (device) {
    case Device::kStm32wb55:
      AddAll(config_, data::stm32wb55::common::kEntries);
      switch (protocol) {
        case Protocol::kIeee802154:
          AddAll(config_, data::stm32wb55::ieee802154::kEntries);
          break;
      }
      break;
  }
}

stfuzz::utils::ErrorCode InjectorConfig::SetParameter(
    const std::string& key, const std::string& value) {
  if (key.empty()) {
    return stfuzz::utils::ErrorCode::kInvalidArgument;
  }
  config_[key] = value;
  return stfuzz::utils::ErrorCode::kOk;
}

bool InjectorConfig::HasKey(const std::string& key) const {
  return config_.find(key) != config_.end();
}

stfuzz::utils::ErrorCode InjectorConfig::GetValue(
    std::string& value, const std::string& key) const {
  const auto it = config_.find(key);
  if (it == config_.end()) {
    return stfuzz::utils::ErrorCode::kConfigKeyNotFound;
  }
  value = it->second;
  return stfuzz::utils::ErrorCode::kOk;
}

}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
