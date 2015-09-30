#include "http2/protocol/hpack/hpack.h"

#include <vector>

#include "http2/headers/constants.h"

namespace http2 {
namespace protocol {
namespace hpack {

void encode_integer(uint8_t hibits, uint8_t numbits, uint32_t value, std::vector<uint8_t>& output) {
  uint8_t max = (1U << numbits) - 1;
  if (value < max) {
    output.push_back(value | hibits);
    return;
  }
  output.push_back(max | hibits);
  value -= max;
  while (value >= 0x80) {
    output.push_back(0x80 | (value & 0x7f));
    value >>= 7;
  }
  output.push_back(value);
}

Encoder::Encoder() { reset(); }

void Encoder::reset() {
  table_.reset();
  sensitive_ = {
      http2::headers::kCookie, http2::headers::kProxyAuthenticate,
      http2::headers::kSetCookie, http2::headers::kWwwAuthenticate,
  };
}

void Encoder::sensitive_header(std::string name) {
  sensitive_.insert(name);
}

void Encoder::encode(const Header& h, std::vector<uint8_t>& output) {
  if (h.name.size() >= 127) abort();
  if (h.value.size() >= 127) abort();

  bool is_sensitive = (sensitive_.find(h.name) != sensitive_.end());
  bool is_big = h.size() > 256;

  auto index = table().best_match(h);
  if (index == 0) {
    if (is_sensitive) {
      output.push_back(0x10);
    } else if (is_big) {
      output.push_back(0x00);
    } else {
      output.push_back(0x40);
    }
    encode_integer(0x00, 7, h.name.size(), output);
    output.insert(output.end(), h.name.begin(), h.name.end());
    encode_integer(0x00, 7, h.value.size(), output);
    output.insert(output.end(), h.value.begin(), h.value.end());
    return;
  }

  Header best = table().at(index);
  if (best == h) {
    encode_integer(0x80, 7, index, output);
    return;
  }

  if (is_sensitive) {
    encode_integer(0x10, 4, index, output);
  } else if (is_big) {
    encode_integer(0x00, 4, index, output);
  } else {
    encode_integer(0x40, 6, index, output);
  }
  encode_integer(0x00, 7, h.value.size(), output);
  output.insert(output.end(), h.value.begin(), h.value.end());
}

}  // namespace hpack
}  // namespace protocol
}  // namespace http2
