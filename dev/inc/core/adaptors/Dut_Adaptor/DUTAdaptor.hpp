#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/adaptors/AdaptorInterface.hpp"
#include "core/adaptors/ConfigInterface.hpp"
#include "core/adaptors/FeedbackInterface.hpp"
#include "core/adaptors/SerialInterface.hpp"
#include "core/adaptors/StatusInterface.hpp"
#include "core/adaptors/dut_adaptor/DUTConfig.hpp"
#include "core/adaptors/dut_adaptor/FeedbackParser.hpp"
#include "core/adaptors/dut_adaptor/SerialLink.hpp"
#include "core/adaptors/dut_adaptor/StatusManager.hpp"

namespace stfuzz {
namespace adaptor {

class DUTAdaptor : public IAdaptor,
                   public IConfig,
                   public ISerial,
                   public IStatus,
                   public IFeedback {
 public:
  // IAdaptor
  DUTAdaptor();
  utils::ErrorCode Connect() override;
  void Disconnect() override;
  bool IsConnected() const override;
  utils::ErrorCode Reset() override;

  // IConfig
  utils::ErrorCode SetParameter(const std::string& key,
                                const std::string& value) override;
  std::optional<std::string> GetValue(const std::string& key) const override;
  bool HasKey(const std::string& key) const override;
  utils::ErrorCode Validate() const override;

  // ISerial
  utils::ErrorCode Open() override;
  void Close() override;
  bool IsOpen() const override;
  utils::ErrorCode Write(const std::vector<uint8_t>& data) override;
  utils::ErrorCode Read(std::vector<uint8_t>& buffer,
                        size_t max_bytes) override;

  // IStatus
  utils::ErrorCode Subscribe(IStatusSubscriber* subscriber) override;
  void Unsubscribe(IStatusSubscriber* subscriber) override;

  // IFeedback
  utils::ErrorCode Subscribe(IFeedbackSubscriber* subscriber) override;
  void Unsubscribe(IFeedbackSubscriber* subscriber) override;

 private:
  DUTConfig config_;
  SerialLink serial_link_;
  FeedbackParser feedback_parser_;
  StatusManager status_manager_;
};

}  // namespace adaptor
}  // namespace stfuzz
