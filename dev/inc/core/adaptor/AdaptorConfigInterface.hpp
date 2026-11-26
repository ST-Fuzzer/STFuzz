#pragma once

#include <string>
#include <unordered_map>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {

using ConfigMap = std::unordered_map<std::string, std::string>;

class IAdaptorConfig {
 public:
  virtual ~IAdaptorConfig() = default;

  [[nodiscard]] virtual stfuzz::utils::ErrorCode SetParameter(
      const std::string& key, const std::string& value) = 0;
  [[nodiscard]] virtual bool HasKey(const std::string& key) const = 0;
  [[nodiscard]] virtual stfuzz::utils::ErrorCode GetValue(
      std::string& value, const std::string& key) const = 0;
  [[nodiscard]] virtual const ConfigMap& Entries() const = 0;
};

}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
