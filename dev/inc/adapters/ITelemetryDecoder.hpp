#pragma once

#include <QByteArray>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class ITelemetryDecoder {
 public:
  virtual ~ITelemetryDecoder() = default;
  virtual DecodedMessage decodeFrame(const QByteArray& frame) = 0;
};

}  // namespace stfuzz::adapters
