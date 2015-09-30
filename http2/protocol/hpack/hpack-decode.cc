#include "http2/protocol/hpack/hpack.h"

#include <cassert>
#include <cstdint>

#include <iostream>

#include "http2/headers/constants.h"

namespace http2 {
namespace protocol {
namespace hpack {

std::size_t decode_integer(const uint8_t* p, const uint8_t* q, unsigned int numbits,
                    uint32_t& output) {
  const uint8_t* pos = p;
  unsigned int shift = 0;
  uint8_t mask = (1U << numbits) - 1U;
  uint8_t byte;

  if (pos == q) goto fail;
  byte = *pos++ & mask;
  output = byte;
  if (byte != mask) {
    return (pos - p);
  }
  do {
    if (pos == q) goto fail;
    byte = *pos++;
    output += (byte & 0x7f) * (1 << shift);
    shift += 7;
  } while (byte & 0x80);
  return (pos - p);

fail:
  output = 0;
  return 0;
}

bool Decoder::decode_lowmem(const uint8_t* p, const uint8_t* q,
                            std::function<void(Header)> callback) {
  Header h;
  std::size_t n;
  uint32_t index, namelen, valuelen, new_max_size;
  bool should_add, namehuff, valuehuff;

  while (p != q) {
    // First, get the oddball cases out of the way.

    if ((*p & 0xe0) == 0x20) {
      // 6.3.  Dynamic Table Size Update

      n = decode_integer(p, q, 5, new_max_size);
      p += n;
      if (n == 0) return false;
      mutable_table().set_max_size(new_max_size);
      continue;
    }

    if (*p & 0x80) {
      // 6.1.  Indexed Header Field Representation

      n = decode_integer(p, q, 7, index);
      p += n;
      if (n == 0) return false;
      if (index == 0) return false;
      try {
        h = table().at(index);
      } catch (const std::out_of_range& e) {
        return false;
      }
      callback(std::move(h));
      continue;
    }

    // Now, the remaining cases are all Literal Header Field, either with or
    // without indexing.

    if ((*p & 0xc0) == 0x40) {
      // 6.2.1.  Literal Header Field with Incremental Indexing

      n = decode_integer(p, q, 6, index);
      p += n;
      if (n == 0) return false;
      should_add = true;
    } else {
      // 6.2.2.  Literal Header Field without Indexing
      // 6.2.3.  Literal Header Field Never Indexed

      // This branch covers the remaining two cases:
      //   (*p & 0xf0) == 0x00
      //   (*p & 0xf0) == 0x10

      n = decode_integer(p, q, 4, index);
      p += n;
      if (n == 0) return false;
      should_add = false;
    }

    if (index > 0) {
      h.name = table().at(index).name;
      goto have_name;
    }

    if (p == q) return false;
    namehuff = (*p & 0x80) != 0;
    n = decode_integer(p, q, 7, namelen);
    p += n;
    if (n == 0) return false;
    if (namelen > (q - p)) return false;
    if (namehuff) {
      std::vector<uint8_t> tmp;
      if (!decode_huffman(p, p + namelen, tmp)) return false;
      h.name.assign(tmp.begin(), tmp.end());
    } else {
      h.name.assign(p, p + namelen);
    }
    p += namelen;

  have_name:
    if (p == q) return false;
    valuehuff = (*p & 0x80) != 0;
    n = decode_integer(p, q, 7, valuelen);
    p += n;
    if (n == 0) return false;
    if (valuelen > (q - p)) return false;
    if (valuehuff) {
      std::vector<uint8_t> tmp;
      if (!decode_huffman(p, p + valuelen, tmp)) return false;
      h.value.assign(tmp.begin(), tmp.end());
    } else {
      h.value.assign(p, p + valuelen);
    }
    p += valuelen;
    if (should_add) mutable_table().add(h);
    callback(std::move(h));
  }
  return true;
}

}  // namespace hpack
}  // namespace protocol
}  // namespace http2
