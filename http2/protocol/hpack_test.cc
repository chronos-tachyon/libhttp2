#include "http2/protocol/hpack.h"

#include <cstdint>

#include <array>
#include <string>
#include <vector>

#include "gtest/gtest.h"

template <typename A, typename B>
testing::AssertionResult items_equal(const char* a_str, const char* b_str,
                                     const A& a, const B& b) {
  auto a_it = a.begin();
  auto b_it = b.begin();
  std::size_t index = 0;
  while (a_it != a.end() && b_it != b.end()) {
    if (*a_it != *b_it) {
      return testing::AssertionFailure()
             << a_str << "[" << index << "] != " << b_str << "[" << index
             << "] (" << *a_it << " vs. " << *b_it << ")";
    }
    ++a_it;
    ++b_it;
    ++index;
  }
  if (a_it != a.end()) {
    return testing::AssertionFailure() << a_str << " has " << (a.end() - a_it)
                                       << " extra items";
  }
  if (b_it != b.end()) {
    return testing::AssertionFailure() << b_str << " has " << (b.end() - b_it)
                                       << " extra items";
  }
  return testing::AssertionSuccess();
}

TEST(Integer, Decode) {
  using http2::protocol::hpack::decode_integer;
  std::vector<uint8_t> input;
  uint32_t i;

  // Examples from RFC 7541

  // C.1.1.  Example 1: Encoding 10 Using a 5-Bit Prefix
  input = {0xea};
  EXPECT_EQ(decode_integer(input, 5, i), 1);
  EXPECT_EQ(i, 10);

  // C.1.2.  Example 2: Encoding 1337 Using a 5-Bit Prefix
  input = {0xff, 0x9a, 0x0a};
  EXPECT_EQ(decode_integer(input, 5, i), 3);
  EXPECT_EQ(i, 1337);

  // C.1.3.  Example 3: Encoding 42 Starting at an Octet Boundary
  input = {0x2a};
  EXPECT_EQ(decode_integer(input, 8, i), 1);
  EXPECT_EQ(i, 42);

  // Variants of C.1.1 that use the don't-care bits.
  input = {0xea};
  EXPECT_EQ(decode_integer(input, 6, i), 1);
  EXPECT_EQ(i, 42);

  EXPECT_EQ(decode_integer(input, 7, i), 1);
  EXPECT_EQ(i, 106);

  EXPECT_EQ(decode_integer(input, 8, i), 1);
  EXPECT_EQ(i, 234);

  // Variants of the RFC examples that include trailing bytes

  input = {0xea, 0xaa};
  EXPECT_EQ(decode_integer(input, 5, i), 1);
  EXPECT_EQ(i, 10);

  input = {0xff, 0x9a, 0x0a, 0xaa};
  EXPECT_EQ(decode_integer(input, 5, i), 3);
  EXPECT_EQ(i, 1337);

  input = {0x2a, 0xaa};
  EXPECT_EQ(decode_integer(input, 8, i), 1);
  EXPECT_EQ(i, 42);

  // Variants of the RFC examples that are truncated prematurely.

  input = {};
  i = 42;
  EXPECT_EQ(decode_integer(input, 5, i), 0);
  EXPECT_EQ(i, 0);

  input = {0xff};
  i = 42;
  EXPECT_EQ(decode_integer(input, 5, i), 0);
  EXPECT_EQ(i, 0);

  input = {0xff, 0x9a};
  i = 42;
  EXPECT_EQ(decode_integer(input, 5, i), 0);
  EXPECT_EQ(i, 0);
}

TEST(Huffman, Decode) {
  using http2::protocol::hpack::decode_huffman;
  std::vector<uint8_t> input, output, expected;

  input = {0xf1, 0xe3, 0xc2, 0xe5, 0xf2, 0x3a,
           0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff};
  expected = {'w', 'w', 'w', '.', 'e', 'x', 'a', 'm',
              'p', 'l', 'e', '.', 'c', 'o', 'm'};
  EXPECT_TRUE(decode_huffman(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);

  input = {0xa8, 0xeb, 0x10, 0x64, 0x9c, 0xbf};
  expected = {'n', 'o', '-', 'c', 'a', 'c', 'h', 'e'};
  EXPECT_TRUE(decode_huffman(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);

  input = {0x25, 0xa8, 0x49, 0xe9, 0x5b, 0xa9, 0x7d, 0x7f};
  expected = {'c', 'u', 's', 't', 'o', 'm', '-', 'k', 'e', 'y'};
  EXPECT_TRUE(decode_huffman(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);

  input = {0x25, 0xa8, 0x49, 0xe9, 0x5b, 0xb8, 0xe8, 0xb4, 0xbf};
  expected = {'c', 'u', 's', 't', 'o', 'm', '-', 'v', 'a', 'l', 'u', 'e'};
  EXPECT_TRUE(decode_huffman(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
}

TEST(Header, Decode) {
  http2::protocol::hpack::Decoder d;
  std::vector<uint8_t> input;
  std::vector<http2::protocol::hpack::Header> output, expected;

  // C.2.1.  Literal Header Field with Indexing
  input = {0x40, 0x0a, 0x63, 0x75, 0x73, 0x74, 0x6f, 0x6d, 0x2d,
           0x6b, 0x65, 0x79, 0x0d, 0x63, 0x75, 0x73, 0x74, 0x6f,
           0x6d, 0x2d, 0x68, 0x65, 0x61, 0x64, 0x65, 0x72};
  expected = {{"custom-key", "custom-header"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  EXPECT_FALSE(d.table().empty());
  output.clear();
  d.reset();

  // C.2.2.  Literal Header Field without Indexing
  input = {0x04, 0x0c, 0x2f, 0x73, 0x61, 0x6d, 0x70,
           0x6c, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};
  expected = {{":path", "/sample/path"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  EXPECT_TRUE(d.table().empty());
  output.clear();
  d.reset();

  // C.2.3.  Literal Header Field Never Indexed
  input = {0x10, 0x08, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72,
           0x64, 0x06, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74};
  expected = {{"password", "secret"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  EXPECT_TRUE(d.table().empty());
  output.clear();
  d.reset();

  // C.2.4.  Indexed Header Field
  input = {0x82};
  expected = {{":method", "GET"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  EXPECT_TRUE(d.table().empty());
  output.clear();
  d.reset();

  // C.3.  Request Examples without Huffman Coding
  input = {0x82, 0x86, 0x84, 0x41, 0x0f, 0x77, 0x77, 0x77, 0x2e, 0x65,
           0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d};
  expected = {{":method", "GET"},
              {":scheme", "http"},
              {":path", "/"},
              {":authority", "www.example.com"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
  input = {0x82, 0x86, 0x84, 0xbe, 0x58, 0x08, 0x6e,
           0x6f, 0x2d, 0x63, 0x61, 0x63, 0x68, 0x65};
  expected = {{":method", "GET"},
              {":scheme", "http"},
              {":path", "/"},
              {":authority", "www.example.com"},
              {"cache-control", "no-cache"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
  input = {0x82, 0x87, 0x85, 0xbf, 0x40, 0x0a, 0x63, 0x75, 0x73, 0x74,
           0x6f, 0x6d, 0x2d, 0x6b, 0x65, 0x79, 0x0c, 0x63, 0x75, 0x73,
           0x74, 0x6f, 0x6d, 0x2d, 0x76, 0x61, 0x6c, 0x75, 0x65};
  expected = {{":method", "GET"},
              {":scheme", "https"},
              {":path", "/index.html"},
              {":authority", "www.example.com"},
              {"custom-key", "custom-value"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();

  // C.4.  Request Examples with Huffman Coding
  input = {0x82, 0x86, 0x84, 0x41, 0x8c, 0xf1, 0xe3, 0xc2, 0xe5,
           0xf2, 0x3a, 0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff};
  expected = {{":method", "GET"},
              {":scheme", "http"},
              {":path", "/"},
              {":authority", "www.example.com"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();

  // Variant of C.4 with Huffman-coded name
  input = {0x82, 0x86, 0x84, 0x41, 0x8c, 0xf1, 0xe3, 0xc2, 0xe5, 0xf2,
           0x3a, 0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff, 0x40, 0x88, 0x25,
           0xa8, 0x49, 0xe9, 0x5b, 0xa9, 0x7d, 0x7f, 0x89, 0x25, 0xa8,
           0x49, 0xe9, 0x5b, 0xb8, 0xe8, 0xb4, 0xbf};
  expected = {{":method", "GET"},
              {":scheme", "http"},
              {":path", "/"},
              {":authority", "www.example.com"},
              {"custom-key", "custom-value"}};
  EXPECT_TRUE(d.decode(input, output));
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
}

TEST(Integer, Encode) {
  using http2::protocol::hpack::encode_integer;
  std::vector<uint8_t> output, expected;

  // Examples from RFC 7541

  // C.1.1.  Example 1: Encoding 10 Using a 5-Bit Prefix
  expected = {0x0a};
  encode_integer(0x00, 5, 10, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
  expected = {0xea};
  encode_integer(0xe0, 5, 10, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();

  // C.1.2.  Example 2: Encoding 1337 Using a 5-Bit Prefix
  expected = {0x1f, 0x9a, 0x0a};
  encode_integer(0x00, 5, 1337, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
  expected = {0xff, 0x9a, 0x0a};
  encode_integer(0xe0, 5, 1337, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();

  // C.1.3.  Example 3: Encoding 42 Starting at an Octet Boundary
  expected = {0x2a};
  encode_integer(0, 8, 42, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();

  // Variants of C.1.1 with fewer don't-care bits.
  expected = {0xea};
  encode_integer(0xc0, 6, 42, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
  expected = {0xea};
  encode_integer(0x80, 7, 106, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
  expected = {0xea};
  encode_integer(0, 8, 234, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();

  // Verify that output is appended
  expected = {0x0a, 0x1f, 0x9a, 0x0a, 0x2a};
  encode_integer(0, 5, 10, output);
  encode_integer(0, 5, 1337, output);
  encode_integer(0, 8, 42, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
  output.clear();
}

TEST(Huffman, Encode) {
  using http2::protocol::hpack::encode_huffman;
  std::vector<uint8_t> input, output, expected;

  input = {'w', 'w', 'w', '.', 'e', 'x', 'a', 'm',
           'p', 'l', 'e', '.', 'c', 'o', 'm'};
  expected = {0xf1, 0xe3, 0xc2, 0xe5, 0xf2, 0x3a,
              0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff};
  encode_huffman(input, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);

  input = {'n', 'o', '-', 'c', 'a', 'c', 'h', 'e'};
  expected = {0xa8, 0xeb, 0x10, 0x64, 0x9c, 0xbf};
  encode_huffman(input, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);

  input = {'c', 'u', 's', 't', 'o', 'm', '-', 'k', 'e', 'y'};
  expected = {0x25, 0xa8, 0x49, 0xe9, 0x5b, 0xa9, 0x7d, 0x7f};
  encode_huffman(input, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);

  input = {'c', 'u', 's', 't', 'o', 'm', '-', 'v', 'a', 'l', 'u', 'e'};
  expected = {0x25, 0xa8, 0x49, 0xe9, 0x5b, 0xb8, 0xe8, 0xb4, 0xbf};
  encode_huffman(input, output);
  EXPECT_PRED_FORMAT2(items_equal, expected, output);
}

TEST(Header, RoundTrip) {
  http2::protocol::hpack::Encoder e;
  http2::protocol::hpack::Decoder d;
  std::vector<http2::protocol::hpack::Header> input, back;
  std::vector<uint8_t> forward;

  input = {{"custom-key", "custom-header"}};
  e.encode_all(input, forward);
  EXPECT_TRUE(d.decode(forward, back));
  EXPECT_PRED_FORMAT2(items_equal, input, back);
  forward.clear();
  back.clear();
  d.reset();
  e.reset();

  input = {{":method", "GET"}};
  e.encode_all(input, forward);
  EXPECT_TRUE(d.decode(forward, back));
  EXPECT_PRED_FORMAT2(items_equal, input, back);
  forward.clear();
  back.clear();
  d.reset();
  e.reset();

  input = {{":method", "GET"},
           {":scheme", "http"},
           {":path", "/"},
           {":authority", "www.example.com"}};
  e.encode_all(input, forward);
  EXPECT_TRUE(d.decode(forward, back));
  EXPECT_PRED_FORMAT2(items_equal, input, back);
  forward.clear();
  back.clear();
  input = {{":method", "GET"},
           {":scheme", "http"},
           {":path", "/"},
           {":authority", "www.example.com"},
           {"cache-control", "no-cache"}};
  e.encode_all(input, forward);
  EXPECT_TRUE(d.decode(forward, back));
  EXPECT_PRED_FORMAT2(items_equal, input, back);
  forward.clear();
  back.clear();
  input = {{":method", "GET"},
           {":scheme", "https"},
           {":path", "/index.html"},
           {":authority", "www.example.com"},
           {"custom-key", "custom-value"}};
  e.encode_all(input, forward);
  EXPECT_TRUE(d.decode(forward, back));
  EXPECT_PRED_FORMAT2(items_equal, input, back);
  forward.clear();
  back.clear();
  d.reset();
  e.reset();

  input = {{":method", "GET"},
           {":scheme", "http"},
           {":path", "/"},
           {":authority", "www.example.com"},
           {"cookie", "sekret"}};
  e.encode_all(input, forward);
  EXPECT_TRUE(d.decode(forward, back));
  EXPECT_PRED_FORMAT2(items_equal, input, back);
  forward.clear();
  back.clear();
  d.reset();
  e.reset();
}
