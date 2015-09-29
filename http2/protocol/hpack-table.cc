#include "http2/protocol/hpack.h"

#include "http2/headers/headers.h"

namespace http2 {
namespace protocol {
namespace hpack {

const std::vector<Header>& static_table() {
  const auto& table = *new std::vector<Header>{
      {},
      {http2::headers::kAuthority, ""},
      {http2::headers::kMethod, http2::headers::kMethodGET},
      {http2::headers::kMethod, http2::headers::kMethodPOST},
      {http2::headers::kPath, "/"},
      {http2::headers::kPath, "/index.html"},
      {http2::headers::kScheme, http2::headers::kSchemeHTTP},
      {http2::headers::kScheme, http2::headers::kSchemeHTTPS},
      {http2::headers::kStatus, "200"},
      {http2::headers::kStatus, "204"},
      {http2::headers::kStatus, "206"},
      {http2::headers::kStatus, "304"},
      {http2::headers::kStatus, "400"},
      {http2::headers::kStatus, "404"},
      {http2::headers::kStatus, "500"},
      {http2::headers::kAcceptCharset, ""},
      {http2::headers::kAcceptEncoding, "gzip, deflate"},
      {http2::headers::kAcceptLanguage, ""},
      {http2::headers::kAcceptRanges, ""},
      {http2::headers::kAccept, ""},
      {http2::headers::kAccessControlAllowOrigin, ""},
      {http2::headers::kAge, ""},
      {http2::headers::kAllow, ""},
      {http2::headers::kAuthorization, ""},
      {http2::headers::kCacheControl, ""},
      {http2::headers::kContentDisposition, ""},
      {http2::headers::kContentEncoding, ""},
      {http2::headers::kContentLanguage, ""},
      {http2::headers::kContentLength, ""},
      {http2::headers::kContentLocation, ""},
      {http2::headers::kContentRange, ""},
      {http2::headers::kContentType, ""},
      {http2::headers::kCookie, ""},
      {http2::headers::kDate, ""},
      {http2::headers::kETag, ""},
      {http2::headers::kExpect, ""},
      {http2::headers::kExpires, ""},
      {http2::headers::kFrom, ""},
      {http2::headers::kHost, ""},
      {http2::headers::kIfMatch, ""},
      {http2::headers::kIfModifiedSince, ""},
      {http2::headers::kIfNoneMatch, ""},
      {http2::headers::kIfRange, ""},
      {http2::headers::kIfUnmodifiedSince, ""},
      {http2::headers::kLastModified, ""},
      {http2::headers::kLink, ""},
      {http2::headers::kLocation, ""},
      {http2::headers::kMaxForwards, ""},
      {http2::headers::kProxyAuthenticate, ""},
      {http2::headers::kProxyAuthorization, ""},
      {http2::headers::kRange, ""},
      {http2::headers::kReferer, ""},
      {http2::headers::kRefresh, ""},
      {http2::headers::kRetryAfter, ""},
      {http2::headers::kServer, ""},
      {http2::headers::kSetCookie, ""},
      {http2::headers::kStrictTransportSecurity, ""},
      {http2::headers::kTransferEncoding, ""},
      {http2::headers::kUserAgent, ""},
      {http2::headers::kVary, ""},
      {http2::headers::kVia, ""},
      {http2::headers::kWwwAuthenticate, ""},
  };
  return table;
}

void Table::set_max_size(std::size_t sz) {
  max_size_ = sz;
  evict();
}

void Table::add(Header h) {
  dynamic_.emplace_front(std::move(h));
  size_ += dynamic_.front().size();
  evict();
}

void Table::evict() {
  while (size_ > max_size_) {
    size_ -= dynamic_.back().size();
    dynamic_.pop_back();
  }
}

std::size_t Table::best_match(const Header& h) const {
  const auto& s = static_table();
  for (std::size_t i = 1; i < s.size(); ++i) {
    if (h == s[i]) return i;
  }
  for (std::size_t i = 0; i < dynamic_.size(); ++i) {
    if (h == dynamic_[i]) return s.size() + i;
  }
  for (std::size_t i = 1; i < s.size(); ++i) {
    if (h.name == s[i].name) return i;
  }
  for (std::size_t i = 0; i < dynamic_.size(); ++i) {
    if (h.name == dynamic_[i].name) return s.size() + i;
  }
  return 0;
}

}  // namespace hpack
}  // namespace protocol
}  // namespace http2
