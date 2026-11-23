#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/adaptors/SerialInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class DUTConfig;

class SerialLink : public ISerial {
 public:
  explicit SerialLink(const DUTConfig& config);

  utils::ErrorCode Open() override;
  void Close() override;
  bool IsOpen() const override;

  utils::ErrorCode Write(const std::vector<uint8_t>& data) override;
  utils::ErrorCode Read(std::vector<uint8_t>& buffer,
                        size_t max_bytes) override;

 private:
  const DUTConfig& config_;
};

}  // namespace adaptor
}  // namespace stfuzz
