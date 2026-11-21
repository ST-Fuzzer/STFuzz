#include "utils/ieee802154_protocol.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace stfuzz::utils {
namespace {

const Ieee802154FrameModel* AsFrame(const void* frame) {
  return static_cast<const Ieee802154FrameModel*>(frame);
}

// ---------------------------------------------------------------------------
// Low-level writers. They always append to the end of `out`.
// ---------------------------------------------------------------------------

// Appends the low `num_bytes` bytes of `value`, least significant byte first.
void WriteLe(uint64_t value, size_t num_bytes, std::vector<uint8_t>& out) {
  for (size_t i = 0; i < num_bytes; ++i) {
    out.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));
  }
}

void WriteU8(uint8_t value, std::vector<uint8_t>& out) {
  WriteLe(value, 1, out);
}

void WriteU16Le(uint16_t value, std::vector<uint8_t>& out) {
  WriteLe(value, 2, out);
}

void WriteU32Le(uint32_t value, std::vector<uint8_t>& out) {
  WriteLe(value, 4, out);
}

void WriteU64Le(uint64_t value, std::vector<uint8_t>& out) {
  WriteLe(value, 8, out);
}

void WriteBytes(const std::vector<uint8_t>& data, std::vector<uint8_t>& out) {
  out.insert(out.end(), data.begin(), data.end());
}

// Helpers to build the FCF bit by bit.
uint16_t Flag(bool condition, uint16_t mask) {
  return condition ? mask : static_cast<uint16_t>(0);
}

uint16_t Bits(uint8_t value, int shift, uint16_t mask) {
  return static_cast<uint16_t>((static_cast<uint16_t>(value) << shift) & mask);
}

// ---------------------------------------------------------------------------
// 1. Frame Control Field (FCF). Always present.
// ---------------------------------------------------------------------------

bool AlwaysPresent(const void* /*frame*/) { return true; }

void WriteFcf(const void* frame, std::vector<uint8_t>& out) {
  const auto* f = AsFrame(frame);
  const auto type = static_cast<uint8_t>(f->frame_type);
  const auto version = static_cast<uint8_t>(f->version);
  const auto dst_mode = static_cast<uint8_t>(f->dst_addr_mode);
  const auto src_mode = static_cast<uint8_t>(f->src_addr_mode);

  if (f->frame_type == Ieee802154FrameType::kMultipurpose) {
    uint16_t mpf = 0;
    mpf |= Bits(type, 0, kMpfFcfTypeMask);
    mpf |= Flag(f->long_frame_control, kMpfFcfLongFc);
    mpf |= Bits(dst_mode, 4, kMpfFcfDstAddrMask);
    mpf |= Bits(src_mode, 6, kMpfFcfSrcAddrMask);

    // Short form: the multipurpose FCF is a single octet.
    if (!f->long_frame_control) {
      WriteLe(mpf, 1, out);
      return;
    }

    mpf |= Flag(f->pan_id_present, kMpfFcfPanIdPresent);
    mpf |= Flag(f->security_enable, kMpfFcfSecEn);
    mpf |= Flag(f->seqno_suppression, kMpfFcfSeqnoSuppression);
    mpf |= Flag(f->frame_pending, kMpfFcfFramePending);
    mpf |= Bits(version, 12, kMpfFcfVersionMask);
    mpf |= Flag(f->ack_request, kMpfFcfAckReq);
    mpf |= Flag(f->ie_present, kMpfFcfIePresent);
    WriteU16Le(mpf, out);
    return;
  }

  uint16_t fcf = 0;
  fcf |= Bits(type, 0, kFcfTypeMask);
  fcf |= Flag(f->security_enable, kFcfSecEn);
  fcf |= Flag(f->frame_pending, kFcfFramePending);
  fcf |= Flag(f->ack_request, kFcfAckReq);
  fcf |= Flag(f->pan_id_compression, kFcfPanIdCompression);
  fcf |= Flag(f->seqno_suppression, kFcfSeqnoSuppression);
  fcf |= Flag(f->ie_present, kFcfIePresent);
  fcf |= Bits(dst_mode, 10, kFcfDstAddrMask);
  fcf |= Bits(version, 12, kFcfVersionMask);
  fcf |= Bits(src_mode, 14, kFcfSrcAddrMask);
  WriteU16Le(fcf, out);
}

// ---------------------------------------------------------------------------
// 2. Sequence Number.
// ---------------------------------------------------------------------------

bool HasSeqno(const void* frame) {
  const auto* f = AsFrame(frame);
  if (f->frame_type == Ieee802154FrameType::kMultipurpose &&
      !f->long_frame_control) {
    return true;
  }
  return !f->seqno_suppression;
}

void WriteSeqno(const void* frame, std::vector<uint8_t>& out) {
  WriteU8(AsFrame(frame)->seqno, out);
}

// ---------------------------------------------------------------------------
// 3. Addressing fields, in standard order: Dst PAN, Dst Addr, Src PAN, Src
// Addr.
// ---------------------------------------------------------------------------

bool HasDstPan(const void* frame) { return AsFrame(frame)->dst_pan_present; }

void WriteDstPan(const void* frame, std::vector<uint8_t>& out) {
  WriteU16Le(AsFrame(frame)->dst_pan, out);
}

bool HasDstAddr(const void* frame) {
  const auto mode = AsFrame(frame)->dst_addr_mode;
  return mode == Ieee802154AddrMode::kShort ||
         mode == Ieee802154AddrMode::kExtended;
}

void WriteDstAddr(const void* frame, std::vector<uint8_t>& out) {
  const auto* f = AsFrame(frame);
  if (f->dst_addr_mode == Ieee802154AddrMode::kShort) {
    WriteU16Le(f->dst16, out);
  } else {
    WriteU64Le(f->dst64, out);
  }
}

bool HasSrcPan(const void* frame) { return AsFrame(frame)->src_pan_present; }

void WriteSrcPan(const void* frame, std::vector<uint8_t>& out) {
  WriteU16Le(AsFrame(frame)->src_pan, out);
}

bool HasSrcAddr(const void* frame) {
  const auto mode = AsFrame(frame)->src_addr_mode;
  return mode == Ieee802154AddrMode::kShort ||
         mode == Ieee802154AddrMode::kExtended;
}

void WriteSrcAddr(const void* frame, std::vector<uint8_t>& out) {
  const auto* f = AsFrame(frame);
  if (f->src_addr_mode == Ieee802154AddrMode::kShort) {
    WriteU16Le(f->src16, out);
  } else {
    WriteU64Le(f->src64, out);
  }
}

// ---------------------------------------------------------------------------
// 4. Auxiliary Security Header.
// ---------------------------------------------------------------------------

bool HasSecurityHeader(const void* frame) {
  return AsFrame(frame)->security_enable;
}

void WriteSecurityHeader(const void* frame, std::vector<uint8_t>& out) {
  const auto& f = *AsFrame(frame);
  const auto level = static_cast<uint8_t>(
      static_cast<uint8_t>(f.security_level) & kAuxSecLevelMask);

  // 802.15.4-2003: Security Control (level only), Frame Counter, Key Sequence
  // Counter.
  if (f.version == Ieee802154Version::k2003) {
    WriteU8(level, out);
    WriteU32Le(f.frame_counter, out);
    WriteU8(f.key_sequence_counter, out);
    return;
  }

  // 802.15.4-2006/2015: Security Control, Frame Counter (optional), Key
  // Identifier (by mode). The same 2-bit value is used for the control byte and
  // for the layout below, so the header is always self-consistent.
  const auto key_id_mode =
      static_cast<uint8_t>(static_cast<uint8_t>(f.key_id_mode) & 0x03);

  auto sec_ctrl = static_cast<uint8_t>(
      level | ((key_id_mode << kAuxKeyIdModeShift) & kAuxKeyIdModeMask));
  if (f.frame_counter_suppression) sec_ctrl |= kAuxFrameCounterSuppressionMask;
  if (f.asn_in_nonce) sec_ctrl |= kAuxAsnInNonceMask;
  // Bit 7 (kAuxCtrlReservedMask) is reserved and always written as 0.

  WriteU8(sec_ctrl, out);
  if (!f.frame_counter_suppression) WriteU32Le(f.frame_counter, out);

  switch (static_cast<Ieee802154KeyIdMode>(key_id_mode)) {
    case Ieee802154KeyIdMode::kImplicit:
      break;
    case Ieee802154KeyIdMode::kKeyIndex:
      WriteU8(f.key_index, out);
      break;
    case Ieee802154KeyIdMode::kKeyExplicit4:
      WriteU32Le(f.key_source.addr32, out);
      WriteU8(f.key_index, out);
      break;
    case Ieee802154KeyIdMode::kKeyExplicit8:
      WriteU64Le(f.key_source.addr64, out);
      WriteU8(f.key_index, out);
      break;
  }
}

// ---------------------------------------------------------------------------
// 5. Header IEs and Payload IEs (already-encoded bytes).
// ---------------------------------------------------------------------------

bool HasHeaderIes(const void* frame) { return AsFrame(frame)->ie_present; }

void WriteHeaderIes(const void* frame, std::vector<uint8_t>& out) {
  WriteBytes(AsFrame(frame)->header_ies, out);
}

bool HasPayloadIes(const void* frame) {
  return AsFrame(frame)->payload_ie_present;
}

void WritePayloadIes(const void* frame, std::vector<uint8_t>& out) {
  WriteBytes(AsFrame(frame)->payload_ies, out);
}

// ---------------------------------------------------------------------------
// 6. MAC Command ID.
// ---------------------------------------------------------------------------

bool HasCommandId(const void* frame) {
  return AsFrame(frame)->frame_type == Ieee802154FrameType::kCommand;
}

void WriteCommandId(const void* frame, std::vector<uint8_t>& out) {
  WriteU8(static_cast<uint8_t>(AsFrame(frame)->command_id), out);
}

// ---------------------------------------------------------------------------
// 7. Payload.
// ---------------------------------------------------------------------------

bool HasPayload(const void* frame) { return !AsFrame(frame)->payload.empty(); }

void WritePayload(const void* frame, std::vector<uint8_t>& out) {
  WriteBytes(AsFrame(frame)->payload, out);
}

// ---------------------------------------------------------------------------
// 8. FCS. The value is written as stored in the model (it is not computed
// here).
// ---------------------------------------------------------------------------

bool HasFcs(const void* frame) { return AsFrame(frame)->fcs_present; }

void WriteFcs(const void* frame, std::vector<uint8_t>& out) {
  const auto* f = AsFrame(frame);
  switch (f->fcs_type) {
    case Ieee802154FcsType::kFcs32Bit:
      WriteU32Le(f->fcs, out);
      break;
    case Ieee802154FcsType::kFcs16Bit:
      WriteU16Le(static_cast<uint16_t>(f->fcs & 0xFFFF), out);
      break;
  }
}

// Fields in on-air order.
constexpr std::array<FieldSpec, 12> kIeee802154Fields = {{
    {&AlwaysPresent, &WriteFcf},
    {&HasSeqno, &WriteSeqno},
    {&HasDstPan, &WriteDstPan},
    {&HasDstAddr, &WriteDstAddr},
    {&HasSrcPan, &WriteSrcPan},
    {&HasSrcAddr, &WriteSrcAddr},
    {&HasSecurityHeader, &WriteSecurityHeader},
    {&HasHeaderIes, &WriteHeaderIes},
    {&HasPayloadIes, &WritePayloadIes},
    {&HasCommandId, &WriteCommandId},
    {&HasPayload, &WritePayload},
    {&HasFcs, &WriteFcs},
}};

constexpr ProtocolModel kIeee802154Model{kIeee802154Fields.data(),
                                         kIeee802154Fields.size()};

}  // namespace

// TODO(nader): Replace temporary stub implementation with teammate's protocol
// model.
const ProtocolModel& GetIeee802154ProtocolModel() { return kIeee802154Model; }

}  // namespace stfuzz::utils
