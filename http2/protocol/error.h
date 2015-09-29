// Tools for dealing with HTTP/2 errors.

#ifndef HTTP2_PROTOCOL_ERROR_H
#define HTTP2_PROTOCOL_ERROR_H

namespace http2 {
namespace protocol {

// Error enumerates the possible error codes in RST_STREAM and GOAWAY frames.
enum Error {
  NO_ERROR = 0x0,
  PROTOCOL_ERROR = 0x1,
  INTERNAL_ERROR = 0x2,
  FLOW_CONTROL_ERROR = 0x3,
  SETTINGS_TIMEOUT = 0x4,
  STREAM_CLOSED = 0x5,
  FRAME_SIZE_ERROR = 0x6,
  REFUSED_STREAM = 0x7,
  CANCEL = 0x8,
  COMPRESSION_ERROR = 0x9,
  CONNECT_ERROR = 0xa,
  ENHANCE_YOUR_CALM = 0xb,
  INADEQUATE_SECURITY = 0xc,
  HTTP_1_1_REQUIRED = 0xd,
};

}  // namespace protocol
}  // namespace http2

#endif  // HTTP2_PROTOCOL_ERROR_H
