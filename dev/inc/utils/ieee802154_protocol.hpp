#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace stfuzz::utils {

/**
 * @brief Descriptor for a single protocol field and its serialization hooks.
 */
struct FieldSpec {
  bool (*is_present)(const void* frame);
  void (*write_bytes)(const void* frame, std::vector<uint8_t>& out);
};

/**
 * @brief Ordered table of field descriptors defining a protocol's wire layout.
 */
struct ProtocolModel {
  const FieldSpec* fields;
  size_t num_fields;
};

// Frame & PHY constants.
inline constexpr size_t kIeee802154MaxFrameLen = 127;
inline constexpr size_t kIeee802154FcsLen = 2;
inline constexpr size_t kIeee802154CipherSize = 16;

inline constexpr uint16_t kIeee802154NoAddr16 = 0xFFFE;
inline constexpr uint16_t kIeee802154BcastAddr = 0xFFFF;
inline constexpr uint16_t kIeee802154BcastPan = 0xFFFF;

// Standard FCF bit-masks.
inline constexpr uint16_t kFcfTypeMask = 0x0007;
inline constexpr uint16_t kFcfSecEn = 0x0008;
inline constexpr uint16_t kFcfFramePending = 0x0010;
inline constexpr uint16_t kFcfAckReq = 0x0020;
inline constexpr uint16_t kFcfPanIdCompression = 0x0040;
inline constexpr uint16_t kFcfSeqnoSuppression = 0x0100;
inline constexpr uint16_t kFcfIePresent = 0x0200;
inline constexpr uint16_t kFcfDstAddrMask = 0x0C00;
inline constexpr uint16_t kFcfVersionMask = 0x3000;
inline constexpr uint16_t kFcfSrcAddrMask = 0xC000;

// Multipurpose FCF bit-masks (IEEE 802.15.4-2015).
inline constexpr uint16_t kMpfFcfTypeMask = 0x0007;
inline constexpr uint16_t kMpfFcfLongFc = 0x0008;
inline constexpr uint16_t kMpfFcfDstAddrMask = 0x0030;
inline constexpr uint16_t kMpfFcfSrcAddrMask = 0x00C0;
inline constexpr uint16_t kMpfFcfPanIdPresent = 0x0100;
inline constexpr uint16_t kMpfFcfSecEn = 0x0200;
inline constexpr uint16_t kMpfFcfSeqnoSuppression = 0x0400;
inline constexpr uint16_t kMpfFcfFramePending = 0x0800;
inline constexpr uint16_t kMpfFcfVersionMask = 0x3000;
inline constexpr uint16_t kMpfFcfAckReq = 0x4000;
inline constexpr uint16_t kMpfFcfIePresent = 0x8000;

// Auxiliary Security Control bit-masks.
inline constexpr uint8_t kAuxSecLevelMask = 0x07;
inline constexpr uint8_t kAuxKeyIdModeMask = 0x18;
inline constexpr uint8_t kAuxKeyIdModeShift = 3;
inline constexpr uint8_t kAuxFrameCounterSuppressionMask = 0x20;
inline constexpr uint8_t kAuxAsnInNonceMask = 0x40;
inline constexpr uint8_t kAuxCtrlReservedMask = 0x80;

enum class Ieee802154FcsType : uint8_t {
  kFcs16Bit = 0,
  kFcs32Bit = 1,
};

enum class Ieee802154FrameType : uint8_t {
  kBeacon = 0x0,
  kData = 0x1,
  kAck = 0x2,
  kCommand = 0x3,
  kReserved = 0x4,
  kMultipurpose = 0x5,
  kFragment = 0x6,
  kExtended = 0x7,
};

enum class Ieee802154Version : uint8_t {
  k2003 = 0x0,
  k2006 = 0x1,
  k2015 = 0x2,
  kReserved = 0x3,
};

enum class Ieee802154AddrMode : uint8_t {
  kNone = 0x0,
  kReserved = 0x1,
  kShort = 0x2,
  kExtended = 0x3,
};

enum class Ieee802154SecurityLevel : uint8_t {
  kNone = 0x00,
  kMic32 = 0x01,
  kMic64 = 0x02,
  kMic128 = 0x03,
  kEnc = 0x04,
  kEncMic32 = 0x05,
  kEncMic64 = 0x06,
  kEncMic128 = 0x07,
};

enum class Ieee802154KeyIdMode : uint8_t {
  kImplicit = 0x00,
  kKeyIndex = 0x01,
  kKeyExplicit4 = 0x02,
  kKeyExplicit8 = 0x03,
};

enum class Ieee802154CommandId : uint8_t {
  kAssocReq = 0x01,
  kAssocRsp = 0x02,
  kDisassocNotify = 0x03,
  kDataReq = 0x04,
  kPanIdConflict = 0x05,
  kOrphanNotify = 0x06,
  kBeaconReq = 0x07,
  kCoordRealign = 0x08,
  kGtsReq = 0x09,
  kTrleMgmtReq = 0x0A,
  kTrleMgmtRsp = 0x0B,
  kDsmeAssocReq = 0x13,
  kDsmeAssocRsp = 0x14,
  kDsmeGtsReq = 0x15,
  kDsmeGtsRsp = 0x16,
  kDsmeGtsNotify = 0x17,
  kDsmeInfoReq = 0x18,
  kDsmeInfoRsp = 0x19,
  kDsmeBeaconAllocNotify = 0x1A,
  kDsmeBeaconCollNotify = 0x1B,
  kDsmeLinkReport = 0x1C,
  kRitDataReq = 0x20,
  kDbsReq = 0x21,
  kDbsRsp = 0x22,
  kRitDataRsp = 0x23,
  kVendorSpecific = 0x24,
};

/**
 * @brief Structured representation of an IEEE 802.15.4 MAC frame.
 */
struct Ieee802154FrameModel {
  // Frame Control Field (FCF).
  Ieee802154Version version{};
  Ieee802154FrameType frame_type{};
  Ieee802154AddrMode dst_addr_mode{};
  Ieee802154AddrMode src_addr_mode{};
  bool security_enable{};
  bool frame_pending{};
  bool ack_request{};
  bool pan_id_compression{};
  bool seqno_suppression{};
  bool ie_present{};

  // IEEE 802.15.4-2015 multipurpose FCF fields.
  bool long_frame_control{};
  bool pan_id_present{};

  uint8_t seqno{};

  // Addressing fields.
  bool dst_pan_present{};
  bool src_pan_present{};
  uint16_t dst_pan{};
  uint16_t src_pan{};
  uint16_t dst16{};
  uint64_t dst64{};
  uint16_t src16{};
  uint64_t src64{};

  // Auxiliary Security Header fields.
  Ieee802154SecurityLevel security_level{};
  Ieee802154KeyIdMode key_id_mode{};
  bool frame_counter_suppression{};
  bool asn_in_nonce{};
  uint32_t frame_counter{};
  uint8_t key_sequence_counter{};  // 802.15.4-2003 security suite only.
  uint64_t asn{};
  union {
    uint32_t addr32;
    uint64_t addr64;
  } key_source{};
  uint8_t key_index{};

  // Information Elements (IEs).
  bool payload_ie_present{};
  std::vector<uint8_t> header_ies{};
  std::vector<uint8_t> payload_ies{};

  // MAC Command ID (valid when frame_type == kCommand).
  Ieee802154CommandId command_id{};

  // Payload & FCS.
  std::vector<uint8_t> payload{};
  bool fcs_present{};
  Ieee802154FcsType fcs_type{};
  uint32_t fcs{};
};

/**
 * @brief Returns the IEEE 802.15.4 protocol model used to serialize frames.
 * @return Reference to the static IEEE 802.15.4 ProtocolModel descriptor.
 */
// TODO(nader): Replace temporary stub implementation with teammate's protocol
// model.
const ProtocolModel& GetIeee802154ProtocolModel();

}  // namespace stfuzz::utils
