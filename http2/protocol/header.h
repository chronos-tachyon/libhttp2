#ifndef HTTP2_PROTOCOL_HEADER_H
#define HTTP2_PROTOCOL_HEADER_H

#include <cstddef>
#include <ostream>
#include <string>

namespace http2 {
namespace protocol {

// Header holds a single HTTP/2 header.
struct Header final {
  std::string name;
  std::string value;

  Header() = default;
  Header(std::string n, std::string v) : name(n), value(v) {}

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

}  // namespace protocol
}  // namespace http2

#endif  // HTTP2_PROTOCOL_HEADER_H
