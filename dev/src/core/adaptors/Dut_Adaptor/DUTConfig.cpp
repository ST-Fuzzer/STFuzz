#include "core/adaptors/dut_adaptor/DUTConfig.hpp"

#include <optional>
#include <string>

#include "utils/ErrorCode.hpp"

namespace stfuzz::adaptor {

utils::ErrorCode DUTConfig::SetParameter(const std::string& key,
                                         const std::string& value) {
  if (key.empty()) {
    return utils::ErrorCode::kInvalidArgument;
  }

  parameters_[key] = value;
  return utils::ErrorCode::kOk;
}

std::optional<std::string> DUTConfig::GetValue(const std::string& key) const {
  const auto it = parameters_.find(key);

  if (it == parameters_.end()) {
    return std::nullopt;
  }

  return it->second;
}

bool DUTConfig::HasKey(const std::string& key) const {
  return parameters_.find(key) != parameters_.end();
}

utils::ErrorCode DUTConfig::Validate() const {
  for (const auto& parameter : parameters_) {
    if (parameter.first.empty()) {
      return utils::ErrorCode::kInvalidConfig;
    }
  }

  return utils::ErrorCode::kOk;
}

}  // namespace stfuzz::adaptor
