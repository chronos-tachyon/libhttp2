// Tools for dealing with HTTP/2 SETTINGS frame payloads.

#ifndef HTTP2_PROTOCOL_SETTINGS_H
#define HTTP2_PROTOCOL_SETTINGS_H

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

}  // namespace protocol
}  // namespace http2

#endif  // HTTP2_PROTOCOL_SETTINGS_H
