#pragma once

namespace stfuzz {
namespace adaptor {

struct StatusReport;

class IStatusSubscriber {
 public:
  virtual ~IStatusSubscriber() = default;

  virtual void Update(const StatusReport& report) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
