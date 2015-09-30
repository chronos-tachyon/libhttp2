// Tools for dealing with HTTP/2 SETTINGS frame payloads.

#ifndef HTTP2_PROTOCOL_SETTINGS_H
#define HTTP2_PROTOCOL_SETTINGS_H

#include <cassert>
#include <cstdint>
#include <vector>

#include "http2/protocol/error.h"

namespace http2 {
namespace protocol {

// SettingsType enumerates the parameters that can appear in a SETTINGS frame.
enum SettingsType {
  SETTINGS_HEADER_TABLE_SIZE = 0x0001,
  SETTINGS_ENABLE_PUSH = 0x0002,
  SETTINGS_MAX_CONCURRENT_STREAMS = 0x0003,
  SETTINGS_INITIAL_WINDOW_SIZE = 0x0004,
  SETTINGS_MAX_FRAME_SIZE = 0x0005,
  SETTINGS_MAX_HEADER_LIST_SIZE = 0x0006,
};

class Settings final {
 public:
  Settings();

  uint32_t header_table_size() const { return header_table_size_; }
  bool enable_push() const { return enable_push_ != 0; }
  uint32_t max_concurrent_streams() const { return max_concurrent_streams_; }
  uint32_t initial_window_size() const { return initial_window_size_; }
  uint32_t max_frame_size() const { return max_frame_size_; }
  uint32_t max_header_list_size() const { return max_header_list_size_; }

  void set_header_table_size(uint32_t header_table_size);
  void set_enable_push(bool enable_push);
  void set_max_concurrent_streams(uint32_t max_concurrent_streams);
  void set_initial_window_size(uint32_t initial_window_size);
  void set_max_frame_size(uint32_t max_frame_size);
  void set_max_header_list_size(uint32_t max_header_list_size);

  void encode(std::vector<uint8_t>& output) const;
  Error decode(const uint8_t* begin, const uint8_t* end);
  Error decode(const std::vector<uint8_t>& input) {
    return decode(input.data(), input.data() + input.size());
  }
  void mark_clean() { dirty_ = 0; }

 private:
  uint32_t header_table_size_;
  uint32_t enable_push_;
  uint32_t max_concurrent_streams_;
  uint32_t initial_window_size_;
  uint32_t max_frame_size_;
  uint32_t max_header_list_size_;
  uint8_t dirty_;
};

}  // namespace protocol
}  // namespace http2

#endif  // HTTP2_PROTOCOL_SETTINGS_H
