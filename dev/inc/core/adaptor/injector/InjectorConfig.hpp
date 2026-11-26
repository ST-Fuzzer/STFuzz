#pragma once

#include <cstdint>
#include <string>

#include "core/adaptor/AdaptorConfigInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {

enum class Device : uint8_t { kStm32wb55 };
enum class Protocol : uint8_t { kIeee802154 };

class InjectorConfig final : public IAdaptorConfig {
 public:
  explicit InjectorConfig(Device device, Protocol protocol);

  [[nodiscard]] stfuzz::utils::ErrorCode SetParameter(
      const std::string& key, const std::string& value) override;
  [[nodiscard]] bool HasKey(const std::string& key) const override;
  [[nodiscard]] stfuzz::utils::ErrorCode GetValue(
      std::string& value, const std::string& key) const override;
  [[nodiscard]] const ConfigMap& Entries() const override { return config_; }

 private:
  ConfigMap config_;
};

}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
