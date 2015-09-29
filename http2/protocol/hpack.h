// Tools for dealing with HTTP/2 headers.

#ifndef HTTP2_PROTOCOL_HPACK_H
#define HTTP2_PROTOCOL_HPACK_H

#include <cstdint>
#include <cstdlib>

#include <deque>
#include <functional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "http2/protocol/headers.h"

namespace http2 {
namespace protocol {
namespace hpack {

// static_table returns the HTTP/2 static table, as specified by RFC 7540
// Appendix A.
const std::vector<Header>& static_table();

// Table manages the HTTP/2 dynamic table, and handles index lookups for both
// the static and dynamic tables.
class Table final {
 public:
  Table() : size_(0), max_size_(4096) {}

  // empty returns true iff the dynamic table contains no entries.
  bool empty() const { return dynamic_.empty(); }

  // size returns the bytes used, as specified by RFC 7540 section 4.1.
  std::size_t size() const { return size_; }

  // max_size returns the current maximum size of the dynamic table.
  std::size_t max_size() const { return max_size_; }

  void reset() {
    max_size_ = 0;
    evict();
    max_size_ = 4096;
  }

  // set_max_size changes the maximum size of the dynamic table, evicting old
  // entries as necessary to bring size() to within the new bounds.
  void set_max_size(std::size_t sz);

  // at returns the cached Header with the given index.  Indices [1,61] point
  // to the static table; indices (61,k) point to the dynamic table, for
  // k=61+[num dynamic table entries].
  //
  // THROWS std::out_of_range if index is 0 or past the dynamic table.
  Header at(std::size_t index) const {
    if (index < 1) throw std::out_of_range("illegal index 0");
    if (index < 62) return static_table().at(index);
    return dynamic_.at(index - 62);
  }

  // add inserts a new Header into the dynamic table.
  //
  // After insertion, entries will be evicted oldest-first until size() <
  // max_size().  If h.size() > max_size(), all entries will be evicted!
  void add(Header h);

  // best_match returns the index of the best-matching existing header, or 0 if
  // nothing matches.
  std::size_t best_match(const Header& h) const;

 private:
  void evict();

  std::size_t size_;
  std::size_t max_size_;
  std::deque<Header> dynamic_;
};

// decode_integer reads an HPACK-style variable-length integer from the given
// region of bytes.  On success, sets the output to the integer read and
// returns the number of bytes to advance.  On failure, sets the output to 0
// and returns 0.
std::size_t decode_integer(const uint8_t* begin, const uint8_t* end,
                           unsigned int numbits, uint32_t& output);
inline std::size_t decode_integer(const std::vector<uint8_t>& input,
                                  unsigned int numbits, uint32_t& output) {
  return decode_integer(input.data(), input.data() + input.size(), numbits,
                        output);
}

// decode_huffman decompresses the input data from the given region, and
// appends the decompressed data to the given output vector.
bool decode_huffman(const uint8_t* begin, const uint8_t* end,
                    std::vector<uint8_t>& output);
inline bool decode_huffman(const std::vector<uint8_t>& input,
                           std::vector<uint8_t>& output) {
  return decode_huffman(input.data(), input.data() + input.size(), output);
}

// Decoder manages the state for receiving HPACK-encoded HTTP/2 headers.
class Decoder final {
 public:
  const Table& table() const { return table_; }
  Table& mutable_table() { return table_; }

  // reset returns this Decoder to its initial state.
  void reset() { table_.reset(); }

  // decode_lowmem scans the given byte region as a headers block, streaming
  // the headers via the provided callback as they are decoded, and returns
  // true on success or false on decode failure.
  bool decode_lowmem(const uint8_t* begin, const uint8_t* end,
                     std::function<void(Header)> callback);

  bool decode_lowmem(const std::vector<uint8_t>& input,
                     std::function<void(Header)> callback) {
    return decode_lowmem(input.data(), input.data() + input.size(), callback);
  }

  // decode scans the given byte region as a headers block, placing the decoded
  // headers in the provided vector, and returns true on success or false on
  // decode failure.
  bool decode(const uint8_t* begin, const uint8_t* end,
              std::vector<Header>& output) {
    output.clear();
    return decode_lowmem(
        begin, end, [&output](Header h) { output.push_back(std::move(h)); });
  }
  bool decode(const std::vector<uint8_t>& input, std::vector<Header>& output) {
    return decode(input.data(), input.data() + input.size(), output);
  }

 private:
  Table table_;
};

// encode_integer encodes an integer into the HPACK variable-length encoding,
// and appends it to the given output vector.
//
// Args:
//   hibits: high-bit flags to set on the first byte.
//   numbits: the number of bits reserved for the integer being encoded.
//   value: the integer being encoded.
//   output: a vector to which the encoded bytes will be appended.
void encode_integer(uint8_t hibits, uint8_t numbits, uint32_t value,
                    std::vector<uint8_t>& output);

// encode_huffman compresses the input data from the given region, and appends
// the compressed data to the given output vector.
void encode_huffman(const uint8_t* begin, const uint8_t* end,
                    std::vector<uint8_t>& output);
inline void encode_huffman(const std::vector<uint8_t>& input,
                           std::vector<uint8_t>& output) {
  encode_huffman(input.data(), input.data() + input.size(), output);
}

// Encoder manages the state for sending HPACK-encoded HTTP/2 headers.
class Encoder final {
 public:
  Encoder();

  const Table& table() const { return table_; }
  Table& mutable_table() { return table_; }

  // reset returns this Encoder to its initial state.
  void reset();

  // sensitive_header marks the named header as "sensitive".  A sensitive
  // header is never indexed in the dynamic table.
  void sensitive_header(std::string name);

  // encode marshals the given header to form an HPACK-formatted payload, and
  // appends that payload to the given output vector.
  void encode(const Header& h, std::vector<uint8_t>& output);

  // encode_all marshals each of the given headers, in the order provided, to
  // form an HPACK-formatted payload, and appends that payload to the given
  // output vector.
  void encode_all(const std::vector<Header>& input,
                  std::vector<uint8_t>& output) {
    for (const auto& h : input) {
      encode(h, output);
    }
  }

 private:
  Table table_;
  std::set<std::string> sensitive_;
};

}  // namespace hpack
}  // namespace protocol
}  // namespace http2

#endif  // HTTP2_PROTOCOL_HPACK_H
