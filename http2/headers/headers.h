#ifndef HTTP2_PROTOCOL_HEADER_H
#define HTTP2_PROTOCOL_HEADER_H

#include <cstddef>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace http2 {
namespace headers {

// Header holds a single HTTP/2 header.
struct Header final {
  std::string name;
  std::string value;

  Header() = default;
  Header(std::string n, std::string v)
      : name(std::move(n)), value(std::move(v)) {}

  // size computes the bytes used, as specified by RFC 7540 section 4.1.
  std::size_t size() const { return 32 + name.size() + value.size(); }
};

inline std::ostream& operator<<(std::ostream& s, const Header& t) {
  return (s << "{" << t.name << ": " << t.value << "}");
}
inline bool operator==(const Header& a, const Header& b) {
  return a.name == b.name && a.value == b.value;
}
inline bool operator<(const Header& a, const Header& b) {
  return a.name < b.name || (a.name == b.name && a.value < b.value);
}
inline bool operator!=(const Header& a, const Header& b) { return !(a == b); }
inline bool operator>(const Header& a, const Header& b) { return (b < a); }
inline bool operator>=(const Header& a, const Header& b) { return !(a < b); }
inline bool operator<=(const Header& a, const Header& b) { return !(b < a); }

class Headers final {
 public:
  Headers() = default;

  const std::vector<Header>& all() const { return headers_; }

  std::vector<std::string> every(std::string name) const;
  std::pair<bool, std::string> first(std::string name) const;
  std::pair<bool, std::string> last(std::string name) const;

  void add(Header h) { headers_.push_back(std::move(h)); }
  void add(std::string name, std::string value) {
    headers_.emplace_back(std::move(name), std::string(value));
  }

  void replace(Header h);
  void replace(std::string name, std::string value) {
    replace(Header{std::move(name), std::move(value)});
  }

  void remove(std::string name);

 private:
  std::vector<Header> headers_;
};

}  // namespace headers
}  // namespace http2

#endif  // HTTP2_PROTOCOL_HEADER_H
