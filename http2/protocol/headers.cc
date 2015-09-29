#include "http2/protocol/headers.h"

namespace http2 {
namespace protocol {

std::vector<std::string> Headers::every(std::string name) const {
  std::vector<std::string> results;
  for (const auto& h : headers_) {
    if (h.name == name) {
      results.push_back(h.value);
    }
  }
  return results;
}

std::pair<bool, std::string> Headers::first(std::string name) const {
  for (const auto& h : headers_) {
    if (h.name == name) {
      return std::make_pair(true, h.value);
    }
  }
  return std::make_pair(false, std::string());
}

std::pair<bool, std::string> Headers::last(std::string name) const {
  auto result = std::make_pair(false, std::string());
  for (const auto& h : headers_) {
    if (h.name == name) {
      result.first = true;
      result.second = h.value;
    }
  }
  return result;
}

void Headers::replace(Header h) {
  bool found = false;
  auto it = headers_.begin();
  while (it != headers_.end()) {
    if (it->name == h.name) {
      if (found) {
        it = headers_.erase(it);
        continue;
      }
      found = true;
      it->value = std::move(h.value);
    }
    ++it;
  }
  if (!found) {
    headers_.push_back(std::move(h));
  }
}

void Headers::remove(std::string name) {
  auto it = headers_.begin();
  while (it != headers_.end()) {
    if (it->name == name) {
      it = headers_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace protocol
}  // namespace http2
