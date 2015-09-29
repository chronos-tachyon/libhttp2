// String constants for HTTP/2 protocol magic strings.

#ifndef HTTP2_PROTOCOL_CONSTANTS_H
#define HTTP2_PROTOCOL_CONSTANTS_H

#include <cstdint>

namespace http2 {
namespace protocol {

// The ALPN token that identifies TLS-encrypted HTTP/2.
extern const uint8_t kEncryptedToken[2];

// The HTTP/1.1 "Upgrade:" token that identifies a switch to cleartext HTTP/2.
extern const uint8_t kCleartextToken[3];

// The connection preface sent by the HTTP/2 client.
extern const uint8_t kConnectionPreface[24];

}  // namespace protocol
}  // namespace http2

#endif  // HTTP2_PROTOCOL_CONSTANTS_H
