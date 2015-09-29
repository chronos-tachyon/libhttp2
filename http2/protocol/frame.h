// Tools for dealing with HTTP/2 frames.

#ifndef HTTP2_PROTOCOL_FRAME_H
#define HTTP2_PROTOCOL_FRAME_H

#include <cstdint>
#include <initializer_list>
#include <ostream>
#include <string>
#include <vector>

namespace http2 {
namespace protocol {

enum Flag {
  NO_FLAGS = 0x00,
  ACK = 0x01,
  END_STREAM = 0x01,
  // 0x02 not used
  END_HEADERS = 0x04,
  PADDED = 0x08,
  // 0x10 not used
  PRIORITY = 0x20,
};

enum FrameType {
  DATA_FRAME = 0x00,
  HEADERS_FRAME = 0x01,
  PRIORITY_FRAME = 0x02,
  RST_STREAM_FRAME = 0x03,
  SETTINGS_FRAME = 0x04,
  PUSH_PROMISE_FRAME = 0x05,
  PING_FRAME = 0x06,
  GOAWAY_FRAME = 0x07,
  WINDOW_UPDATE_FRAME = 0x08,
  CONTINUATION_FRAME = 0x09,
};

class Frame final {
 public:
  Frame(uint8_t type = PING_FRAME, uint8_t flags = NO_FLAGS,
        uint32_t stream_id = 0, std::initializer_list<uint8_t> il = {})
      : type_(type),
        flags_(flags),
        sid_(stream_id),
        payload_(il.begin(), il.end()) {}

  void clear() { *this = Frame(); }

  uint8_t type() const { return type_; }
  uint8_t flags() const { return flags_; }
  bool has_flag(uint8_t bit) const { return (flags_ & bit) == bit; }
  uint32_t stream_id() const { return sid_; }
  const std::vector<uint8_t>& payload() const { return payload_; }

  void set_type(uint8_t t) { type_ = t; }
  void set_flags(uint8_t f) { flags_ = f; }
  void set_stream_id(uint32_t sid) { sid_ = sid; }
  std::vector<uint8_t>& mutable_payload() { return payload_; }

  std::vector<uint8_t> encode() const;

  bool decode(const uint8_t* begin, const uint8_t* end);
  bool decode(const std::vector<uint8_t>& vec) {
    return decode(vec.data(), vec.data() + vec.size());
  }

  explicit operator std::string() const;

 private:
  uint8_t type_;
  uint8_t flags_;
  uint32_t sid_;
  std::vector<uint8_t> payload_;
};

inline std::ostream& operator<<(std::ostream& s, const Frame& t) {
  return (s << std::string(t));
}

}  // namespace protocol
}  // namespace http2

#endif  // HTTP2_PROTOCOL_FRAME_H
