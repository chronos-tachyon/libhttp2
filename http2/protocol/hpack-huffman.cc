#include "http2/protocol/hpack.h"

#include <cassert>

#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>

namespace {

struct HuffmanEntry final {
  uint32_t bits;
  uint16_t numbits;
  uint16_t symbol;
};

static const HuffmanEntry kHuffmanTable[] = {
    {0x00001ff8, 13, 0x00},  //   0 NUL
    {0x007fffd8, 23, 0x01},  //   1 SOH
    {0x0fffffe2, 28, 0x02},  //   2 STX
    {0x0fffffe3, 28, 0x03},  //   3 ETX
    {0x0fffffe4, 28, 0x04},  //   4 EOT
    {0x0fffffe5, 28, 0x05},  //   5 ENQ
    {0x0fffffe6, 28, 0x06},  //   6 ACK
    {0x0fffffe7, 28, 0x07},  //   7 BEL
    {0x0fffffe8, 28, 0x08},  //   8 BS
    {0x00ffffea, 24, 0x09},  //   9 HT
    {0x3ffffffc, 30, 0x0a},  //  10 LF
    {0x0fffffe9, 28, 0x0b},  //  11 VT
    {0x0fffffea, 28, 0x0c},  //  12 FF
    {0x3ffffffd, 30, 0x0d},  //  13 CR
    {0x0fffffeb, 28, 0x0e},  //  14 SO
    {0x0fffffec, 28, 0x0f},  //  15 SI
    {0x0fffffed, 28, 0x10},  //  16 DLE
    {0x0fffffee, 28, 0x11},  //  17 DC1
    {0x0fffffef, 28, 0x12},  //  18 DC2
    {0x0ffffff0, 28, 0x13},  //  19 DC3
    {0x0ffffff1, 28, 0x14},  //  20 DC4
    {0x0ffffff2, 28, 0x15},  //  21 NAK
    {0x3ffffffe, 30, 0x16},  //  22 SYN
    {0x0ffffff3, 28, 0x17},  //  23 ETB
    {0x0ffffff4, 28, 0x18},  //  24 CAN
    {0x0ffffff5, 28, 0x19},  //  25 EM
    {0x0ffffff6, 28, 0x1a},  //  26 SUB
    {0x0ffffff7, 28, 0x1b},  //  27 ESC
    {0x0ffffff8, 28, 0x1c},  //  28 FS
    {0x0ffffff9, 28, 0x1d},  //  29 GS
    {0x0ffffffa, 28, 0x1e},  //  30 RS
    {0x0ffffffb, 28, 0x1f},  //  31 US
    {0x00000014, 6, 0x20},   //  32 ' '
    {0x000003f8, 10, 0x21},  //  33 '!'
    {0x000003f9, 10, 0x22},  //  34 '"'
    {0x00000ffa, 12, 0x23},  //  35 '#'
    {0x00001ff9, 13, 0x24},  //  36 '$'
    {0x00000015, 6, 0x25},   //  37 '%'
    {0x000000f8, 8, 0x26},   //  38 '&'
    {0x000007fa, 11, 0x27},  //  39 '''
    {0x000003fa, 10, 0x28},  //  40 '('
    {0x000003fb, 10, 0x29},  //  41 ')'
    {0x000000f9, 8, 0x2a},   //  42 '*'
    {0x000007fb, 11, 0x2b},  //  43 '+'
    {0x000000fa, 8, 0x2c},   //  44 ','
    {0x00000016, 6, 0x2d},   //  45 '-'
    {0x00000017, 6, 0x2e},   //  46 '.'
    {0x00000018, 6, 0x2f},   //  47 '/'
    {0x00000000, 5, 0x30},   //  48 '0'
    {0x00000001, 5, 0x31},   //  49 '1'
    {0x00000002, 5, 0x32},   //  50 '2'
    {0x00000019, 6, 0x33},   //  51 '3'
    {0x0000001a, 6, 0x34},   //  52 '4'
    {0x0000001b, 6, 0x35},   //  53 '5'
    {0x0000001c, 6, 0x36},   //  54 '6'
    {0x0000001d, 6, 0x37},   //  55 '7'
    {0x0000001e, 6, 0x38},   //  56 '8'
    {0x0000001f, 6, 0x39},   //  57 '9'
    {0x0000005c, 7, 0x3a},   //  58 ':'
    {0x000000fb, 8, 0x3b},   //  59 ';'
    {0x00007ffc, 15, 0x3c},  //  60 '<'
    {0x00000020, 6, 0x3d},   //  61 '='
    {0x00000ffb, 12, 0x3e},  //  62 '>'
    {0x000003fc, 10, 0x3f},  //  63 '?'
    {0x00001ffa, 13, 0x40},  //  64 '@'
    {0x00000021, 6, 0x41},   //  65 'A'
    {0x0000005d, 7, 0x42},   //  66 'B'
    {0x0000005e, 7, 0x43},   //  67 'C'
    {0x0000005f, 7, 0x44},   //  68 'D'
    {0x00000060, 7, 0x45},   //  69 'E'
    {0x00000061, 7, 0x46},   //  70 'F'
    {0x00000062, 7, 0x47},   //  71 'G'
    {0x00000063, 7, 0x48},   //  72 'H'
    {0x00000064, 7, 0x49},   //  73 'I'
    {0x00000065, 7, 0x4a},   //  74 'J'
    {0x00000066, 7, 0x4b},   //  75 'K'
    {0x00000067, 7, 0x4c},   //  76 'L'
    {0x00000068, 7, 0x4d},   //  77 'M'
    {0x00000069, 7, 0x4e},   //  78 'N'
    {0x0000006a, 7, 0x4f},   //  79 'O'
    {0x0000006b, 7, 0x50},   //  80 'P'
    {0x0000006c, 7, 0x51},   //  81 'Q'
    {0x0000006d, 7, 0x52},   //  82 'R'
    {0x0000006e, 7, 0x53},   //  83 'S'
    {0x0000006f, 7, 0x54},   //  84 'T'
    {0x00000070, 7, 0x55},   //  85 'U'
    {0x00000071, 7, 0x56},   //  86 'V'
    {0x00000072, 7, 0x57},   //  87 'W'
    {0x000000fc, 8, 0x58},   //  88 'X'
    {0x00000073, 7, 0x59},   //  89 'Y'
    {0x000000fd, 8, 0x5a},   //  90 'Z'
    {0x00001ffb, 13, 0x5b},  //  91 '['
    {0x0007fff0, 19, 0x5c},  //  92 '\'
    {0x00001ffc, 13, 0x5d},  //  93 ']'
    {0x00003ffc, 14, 0x5e},  //  94 '^'
    {0x00000022, 6, 0x5f},   //  95 '_'
    {0x00007ffd, 15, 0x60},  //  96 '`'
    {0x00000003, 5, 0x61},   //  97 'a'
    {0x00000023, 6, 0x62},   //  98 'b'
    {0x00000004, 5, 0x63},   //  99 'c'
    {0x00000024, 6, 0x64},   // 100 'd'
    {0x00000005, 5, 0x65},   // 101 'e'
    {0x00000025, 6, 0x66},   // 102 'f'
    {0x00000026, 6, 0x67},   // 103 'g'
    {0x00000027, 6, 0x68},   // 104 'h'
    {0x00000006, 5, 0x69},   // 105 'i'
    {0x00000074, 7, 0x6a},   // 106 'j'
    {0x00000075, 7, 0x6b},   // 107 'k'
    {0x00000028, 6, 0x6c},   // 108 'l'
    {0x00000029, 6, 0x6d},   // 109 'm'
    {0x0000002a, 6, 0x6e},   // 110 'n'
    {0x00000007, 5, 0x6f},   // 111 'o'
    {0x0000002b, 6, 0x70},   // 112 'p'
    {0x00000076, 7, 0x71},   // 113 'q'
    {0x0000002c, 6, 0x72},   // 114 'r'
    {0x00000008, 5, 0x73},   // 115 's'
    {0x00000009, 5, 0x74},   // 116 't'
    {0x0000002d, 6, 0x75},   // 117 'u'
    {0x00000077, 7, 0x76},   // 118 'v'
    {0x00000078, 7, 0x77},   // 119 'w'
    {0x00000079, 7, 0x78},   // 120 'x'
    {0x0000007a, 7, 0x79},   // 121 'y'
    {0x0000007b, 7, 0x7a},   // 122 'z'
    {0x00007ffe, 15, 0x7b},  // 123 '{'
    {0x000007fc, 11, 0x7c},  // 124 '|'
    {0x00003ffd, 14, 0x7d},  // 125 '}'
    {0x00001ffd, 13, 0x7e},  // 126 '~'
    {0x0ffffffc, 28, 0x7f},  // 127 DEL
    {0x000fffe6, 20, 0x80},  // 128
    {0x003fffd2, 22, 0x81},  // 129
    {0x000fffe7, 20, 0x82},  // 130
    {0x000fffe8, 20, 0x83},  // 131
    {0x003fffd3, 22, 0x84},  // 132
    {0x003fffd4, 22, 0x85},  // 133
    {0x003fffd5, 22, 0x86},  // 134
    {0x007fffd9, 23, 0x87},  // 135
    {0x003fffd6, 22, 0x88},  // 136
    {0x007fffda, 23, 0x89},  // 137
    {0x007fffdb, 23, 0x8a},  // 138
    {0x007fffdc, 23, 0x8b},  // 139
    {0x007fffdd, 23, 0x8c},  // 140
    {0x007fffde, 23, 0x8d},  // 141
    {0x00ffffeb, 24, 0x8e},  // 142
    {0x007fffdf, 23, 0x8f},  // 143
    {0x00ffffec, 24, 0x90},  // 144
    {0x00ffffed, 24, 0x91},  // 145
    {0x003fffd7, 22, 0x92},  // 146
    {0x007fffe0, 23, 0x93},  // 147
    {0x00ffffee, 24, 0x94},  // 148
    {0x007fffe1, 23, 0x95},  // 149
    {0x007fffe2, 23, 0x96},  // 150
    {0x007fffe3, 23, 0x97},  // 151
    {0x007fffe4, 23, 0x98},  // 152
    {0x001fffdc, 21, 0x99},  // 153
    {0x003fffd8, 22, 0x9a},  // 154
    {0x007fffe5, 23, 0x9b},  // 155
    {0x003fffd9, 22, 0x9c},  // 156
    {0x007fffe6, 23, 0x9d},  // 157
    {0x007fffe7, 23, 0x9e},  // 158
    {0x00ffffef, 24, 0x9f},  // 159
    {0x003fffda, 22, 0xa0},  // 160
    {0x001fffdd, 21, 0xa1},  // 161
    {0x000fffe9, 20, 0xa2},  // 162
    {0x003fffdb, 22, 0xa3},  // 163
    {0x003fffdc, 22, 0xa4},  // 164
    {0x007fffe8, 23, 0xa5},  // 165
    {0x007fffe9, 23, 0xa6},  // 166
    {0x001fffde, 21, 0xa7},  // 167
    {0x007fffea, 23, 0xa8},  // 168
    {0x003fffdd, 22, 0xa9},  // 169
    {0x003fffde, 22, 0xaa},  // 170
    {0x00fffff0, 24, 0xab},  // 171
    {0x001fffdf, 21, 0xac},  // 172
    {0x003fffdf, 22, 0xad},  // 173
    {0x007fffeb, 23, 0xae},  // 174
    {0x007fffec, 23, 0xaf},  // 175
    {0x001fffe0, 21, 0xb0},  // 176
    {0x001fffe1, 21, 0xb1},  // 177
    {0x003fffe0, 22, 0xb2},  // 178
    {0x001fffe2, 21, 0xb3},  // 179
    {0x007fffed, 23, 0xb4},  // 180
    {0x003fffe1, 22, 0xb5},  // 181
    {0x007fffee, 23, 0xb6},  // 182
    {0x007fffef, 23, 0xb7},  // 183
    {0x000fffea, 20, 0xb8},  // 184
    {0x003fffe2, 22, 0xb9},  // 185
    {0x003fffe3, 22, 0xba},  // 186
    {0x003fffe4, 22, 0xbb},  // 187
    {0x007ffff0, 23, 0xbc},  // 188
    {0x003fffe5, 22, 0xbd},  // 189
    {0x003fffe6, 22, 0xbe},  // 190
    {0x007ffff1, 23, 0xbf},  // 191
    {0x03ffffe0, 26, 0xc0},  // 192
    {0x03ffffe1, 26, 0xc1},  // 193
    {0x000fffeb, 20, 0xc2},  // 194
    {0x0007fff1, 19, 0xc3},  // 195
    {0x003fffe7, 22, 0xc4},  // 196
    {0x007ffff2, 23, 0xc5},  // 197
    {0x003fffe8, 22, 0xc6},  // 198
    {0x01ffffec, 25, 0xc7},  // 199
    {0x03ffffe2, 26, 0xc8},  // 200
    {0x03ffffe3, 26, 0xc9},  // 201
    {0x03ffffe4, 26, 0xca},  // 202
    {0x07ffffde, 27, 0xcb},  // 203
    {0x07ffffdf, 27, 0xcc},  // 204
    {0x03ffffe5, 26, 0xcd},  // 205
    {0x00fffff1, 24, 0xce},  // 206
    {0x01ffffed, 25, 0xcf},  // 207
    {0x0007fff2, 19, 0xd0},  // 208
    {0x001fffe3, 21, 0xd1},  // 209
    {0x03ffffe6, 26, 0xd2},  // 210
    {0x07ffffe0, 27, 0xd3},  // 211
    {0x07ffffe1, 27, 0xd4},  // 212
    {0x03ffffe7, 26, 0xd5},  // 213
    {0x07ffffe2, 27, 0xd6},  // 214
    {0x00fffff2, 24, 0xd7},  // 215
    {0x001fffe4, 21, 0xd8},  // 216
    {0x001fffe5, 21, 0xd9},  // 217
    {0x03ffffe8, 26, 0xda},  // 218
    {0x03ffffe9, 26, 0xdb},  // 219
    {0x0ffffffd, 28, 0xdc},  // 220
    {0x07ffffe3, 27, 0xdd},  // 221
    {0x07ffffe4, 27, 0xde},  // 222
    {0x07ffffe5, 27, 0xdf},  // 223
    {0x000fffec, 20, 0xe0},  // 224
    {0x00fffff3, 24, 0xe1},  // 225
    {0x000fffed, 20, 0xe2},  // 226
    {0x001fffe6, 21, 0xe3},  // 227
    {0x003fffe9, 22, 0xe4},  // 228
    {0x001fffe7, 21, 0xe5},  // 229
    {0x001fffe8, 21, 0xe6},  // 230
    {0x007ffff3, 23, 0xe7},  // 231
    {0x003fffea, 22, 0xe8},  // 232
    {0x003fffeb, 22, 0xe9},  // 233
    {0x01ffffee, 25, 0xea},  // 234
    {0x01ffffef, 25, 0xeb},  // 235
    {0x00fffff4, 24, 0xec},  // 236
    {0x00fffff5, 24, 0xed},  // 237
    {0x03ffffea, 26, 0xee},  // 238
    {0x007ffff4, 23, 0xef},  // 239
    {0x03ffffeb, 26, 0xf0},  // 240
    {0x07ffffe6, 27, 0xf1},  // 241
    {0x03ffffec, 26, 0xf2},  // 242
    {0x03ffffed, 26, 0xf3},  // 243
    {0x07ffffe7, 27, 0xf4},  // 244
    {0x07ffffe8, 27, 0xf5},  // 245
    {0x07ffffe9, 27, 0xf6},  // 246
    {0x07ffffea, 27, 0xf7},  // 247
    {0x07ffffeb, 27, 0xf8},  // 248
    {0x0ffffffe, 28, 0xf9},  // 249
    {0x07ffffec, 27, 0xfa},  // 250
    {0x07ffffed, 27, 0xfb},  // 251
    {0x07ffffee, 27, 0xfc},  // 252
    {0x07ffffef, 27, 0xfd},  // 253
    {0x07fffff0, 27, 0xfe},  // 254
    {0x03ffffee, 26, 0xff},  // 255
    {0x3fffffff, 30, 256},   // END OF SYMBOLS
};

}  // anonymous namespace

namespace http2 {
namespace protocol {
namespace hpack {

void encode_huffman(const uint8_t* p, const uint8_t* q,
                    std::vector<uint8_t>& output) {
  HuffmanEntry e;
  uint64_t partial = 0; // can contain up to 30+7 bits
  uint16_t partialbits = 0;
  uint8_t tmp;

  output.clear();
  while (p != q) {
    e = kHuffmanTable[*p++];
    partial = (partial << e.numbits) | e.bits;
    partialbits += e.numbits;
    while (partialbits >= 8) {
      tmp = partial >> (partialbits - 8);
      output.push_back(tmp);
      partialbits -= 8;
    }
  }
  if (partialbits > 0) {
    tmp = partial << (8 - partialbits);
    tmp |= (1U << (8 - partialbits)) - 1;
    output.push_back(tmp);
  }
}

bool decode_huffman(const uint8_t* p, const uint8_t* q,
                    std::vector<uint8_t>& output) {
  uint64_t partial = 0;
  uint16_t partialbits = 0;
  bool redo;

  output.clear();
  while (true) {
    if (partialbits >= 5) {
      do {
        redo = false;
        for (auto e : kHuffmanTable) {
          if (e.numbits <= partialbits &&
              e.bits == (partial >> (partialbits - e.numbits))) {
            if (e.symbol > 0xff) return false;
            output.push_back(e.symbol);
            partialbits -= e.numbits;
            partial &= (1ULL << partialbits) - 1;
            redo = true;
            break;
          }
        }
        if (partialbits >= 30) return false;
      } while (redo);
    }

    if (p == q) {
      return (partialbits < 8 && partial == (1ULL << partialbits) - 1);
    }
    partial = (partial << 8) | *p++;
    partialbits += 8;
  }
}

}  // namespace hpack
}  // namespace protocol
}  // namespace http2
