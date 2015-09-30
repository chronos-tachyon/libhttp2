#include "http2/protocol/settings.h"

static constexpr uint32_t kDefaultHeaderTableSize = 4096;
static constexpr uint32_t kDefaultEnablePush = 1;
static constexpr uint32_t kDefaultMaxConcurrentStreams = ~uint32_t(0);
static constexpr uint32_t kDefaultInitialWindowSize = 65535;
static constexpr uint32_t kDefaultMaxFrameSize = 16384;
static constexpr uint32_t kDefaultMaxHeaderListSize = ~uint32_t(0);

namespace http2 {
namespace protocol {

Settings::Settings()
    : header_table_size_(kDefaultHeaderTableSize),
      enable_push_(kDefaultEnablePush),
      max_concurrent_streams_(kDefaultMaxConcurrentStreams),
      initial_window_size_(kDefaultInitialWindowSize),
      max_frame_size_(kDefaultMaxFrameSize),
      max_header_list_size_(kDefaultMaxHeaderListSize),
      dirty_(0) {}

void Settings::set_header_table_size(uint32_t header_table_size) {
  header_table_size_ = header_table_size;
  dirty_ |= 1U << (SETTINGS_HEADER_TABLE_SIZE - 1);
}

void Settings::set_enable_push(bool enable_push) {
  enable_push_ = enable_push ? 1 : 0;
  dirty_ |= 1U << (SETTINGS_ENABLE_PUSH - 1);
}

void Settings::set_max_concurrent_streams(uint32_t max_concurrent_streams) {
  max_concurrent_streams_ = max_concurrent_streams;
  dirty_ |= 1U << (SETTINGS_MAX_CONCURRENT_STREAMS - 1);
}

void Settings::set_initial_window_size(uint32_t initial_window_size) {
  assert(initial_window_size <= 2147483647);
  initial_window_size_ = initial_window_size;
  dirty_ |= 1U << (SETTINGS_INITIAL_WINDOW_SIZE - 1);
}

void Settings::set_max_frame_size(uint32_t max_frame_size) {
  assert(max_frame_size >= 16384);
  assert(max_frame_size <= 16777215);
  max_frame_size_ = max_frame_size;
  dirty_ |= 1U << (SETTINGS_MAX_FRAME_SIZE - 1);
}

void Settings::set_max_header_list_size(uint32_t max_header_list_size) {
  max_header_list_size_ = max_header_list_size;
  dirty_ |= 1U << (SETTINGS_MAX_HEADER_LIST_SIZE - 1);
}

void Settings::encode(std::vector<uint8_t>& output) const {
  if (dirty_ & (1U << (SETTINGS_HEADER_TABLE_SIZE - 1))) {
    output.push_back((SETTINGS_HEADER_TABLE_SIZE >> 8) & 0xff);
    output.push_back(SETTINGS_HEADER_TABLE_SIZE & 0xff);
    output.push_back((header_table_size_ >> 24) & 0xff);
    output.push_back((header_table_size_ >> 16) & 0xff);
    output.push_back((header_table_size_ >> 8) & 0xff);
    output.push_back(header_table_size_ & 0xff);
  }
  if (dirty_ & (1U << (SETTINGS_ENABLE_PUSH - 1))) {
    output.push_back((SETTINGS_ENABLE_PUSH >> 8) & 0xff);
    output.push_back(SETTINGS_ENABLE_PUSH & 0xff);
    output.push_back((enable_push_ >> 24) & 0xff);
    output.push_back((enable_push_ >> 16) & 0xff);
    output.push_back((enable_push_ >> 8) & 0xff);
    output.push_back(enable_push_ & 0xff);
  }
  if (dirty_ & (1U << (SETTINGS_MAX_CONCURRENT_STREAMS - 1))) {
    output.push_back((SETTINGS_MAX_CONCURRENT_STREAMS >> 8) & 0xff);
    output.push_back(SETTINGS_MAX_CONCURRENT_STREAMS & 0xff);
    output.push_back((max_concurrent_streams_ >> 24) & 0xff);
    output.push_back((max_concurrent_streams_ >> 16) & 0xff);
    output.push_back((max_concurrent_streams_ >> 8) & 0xff);
    output.push_back(max_concurrent_streams_ & 0xff);
  }
  if (dirty_ & (1U << (SETTINGS_INITIAL_WINDOW_SIZE - 1))) {
    output.push_back((SETTINGS_INITIAL_WINDOW_SIZE >> 8) & 0xff);
    output.push_back(SETTINGS_INITIAL_WINDOW_SIZE & 0xff);
    output.push_back((initial_window_size_ >> 24) & 0xff);
    output.push_back((initial_window_size_ >> 16) & 0xff);
    output.push_back((initial_window_size_ >> 8) & 0xff);
    output.push_back(initial_window_size_ & 0xff);
  }
  if (dirty_ & (1U << (SETTINGS_MAX_FRAME_SIZE - 1))) {
    output.push_back((SETTINGS_MAX_FRAME_SIZE >> 8) & 0xff);
    output.push_back(SETTINGS_MAX_FRAME_SIZE & 0xff);
    output.push_back((max_frame_size_ >> 24) & 0xff);
    output.push_back((max_frame_size_ >> 16) & 0xff);
    output.push_back((max_frame_size_ >> 8) & 0xff);
    output.push_back(max_frame_size_ & 0xff);
  }
  if (dirty_ & (1U << (SETTINGS_MAX_HEADER_LIST_SIZE - 1))) {
    output.push_back((SETTINGS_MAX_HEADER_LIST_SIZE >> 8) & 0xff);
    output.push_back(SETTINGS_MAX_HEADER_LIST_SIZE & 0xff);
    output.push_back((max_header_list_size_ >> 24) & 0xff);
    output.push_back((max_header_list_size_ >> 16) & 0xff);
    output.push_back((max_header_list_size_ >> 8) & 0xff);
    output.push_back(max_header_list_size_ & 0xff);
  }
}

Error Settings::decode(const uint8_t* p, const uint8_t* q) {
  while (p != q) {
    if ((q - p) < 6) return FRAME_SIZE_ERROR;
    uint16_t type = (p[0] << 8) | p[1];
    uint32_t value = (p[2] << 24) | (p[3] << 16) | (p[4] << 8) | p[5];
    p += 6;
    switch (type) {
      case SETTINGS_HEADER_TABLE_SIZE:
        header_table_size_ = value;
        break;
      case SETTINGS_ENABLE_PUSH:
        if (value != 0 && value != 1) return PROTOCOL_ERROR;
        enable_push_ = value;
        break;
      case SETTINGS_MAX_CONCURRENT_STREAMS:
        max_concurrent_streams_ = value;
        break;
      case SETTINGS_INITIAL_WINDOW_SIZE:
        if (value > 2147483647) return FLOW_CONTROL_ERROR;
        initial_window_size_ = value;
        break;
      case SETTINGS_MAX_FRAME_SIZE:
        if (value < 16384 || value > 16777215) return PROTOCOL_ERROR;
        max_frame_size_ = value;
        break;
      case SETTINGS_MAX_HEADER_LIST_SIZE:
        max_header_list_size_ = value;
        break;
    }
  }
  return NO_ERROR;
}

}  // namespace protocol
}  // namespace http2
