#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "core/adaptors/SerialInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz::adaptor {

struct SerialPortSettings {
  std::string port_name;
  std::uint32_t baud_rate;
};

class SerialLink : public ISerial {
 public:
  explicit SerialLink(SerialPortSettings settings);
  ~SerialLink() override;

  SerialLink(const SerialLink&) = delete;
  SerialLink& operator=(const SerialLink&) = delete;

  utils::ErrorCode Open() override;
  void Close() override;
  bool IsOpen() const override;

  utils::ErrorCode Write(const std::vector<uint8_t>& data) override;

  utils::ErrorCode Read(std::vector<uint8_t>& buffer,
                        size_t max_bytes) override;

 private:
  SerialPortSettings settings_;
  void* handle_ = nullptr;
};

}  // namespace stfuzz::adaptor
