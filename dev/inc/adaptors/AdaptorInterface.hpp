#pragma once

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class IAdaptor {
 public:
  virtual ~IAdaptor() = default;

  virtual utils::ErrorCode Connect() = 0;
  virtual void Disconnect() = 0;
  virtual bool IsConnected() const = 0;
  virtual utils::ErrorCode Reset() = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
