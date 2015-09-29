#include "http2/headers/constants.h"

namespace http2 {
namespace headers {

const char* const kMethod = ":method";
const char* const kScheme = ":scheme";
const char* const kAuthority = ":authority";
const char* const kPath = ":path";
const char* const kStatus = ":status";

const char* const kAcceptCharset = "accept-charset";
const char* const kAcceptEncoding = "accept-encoding";
const char* const kAcceptLanguage = "accept-language";
const char* const kAcceptRanges = "accept-ranges";
const char* const kAccept = "accept";
const char* const kAccessControlAllowOrigin = "access-control-allow-origin";
const char* const kAge = "age";
const char* const kAllow = "allow";
const char* const kAuthorization = "authorization";
const char* const kCacheControl = "cache-control";
const char* const kContentDisposition = "content-disposition";
const char* const kContentEncoding = "content-encoding";
const char* const kContentLanguage = "content-language";
const char* const kContentLength = "content-length";
const char* const kContentLocation = "content-location";
const char* const kContentRange = "content-range";
const char* const kContentType = "content-type";
const char* const kCookie = "cookie";
const char* const kDate = "date";
const char* const kETag = "etag";
const char* const kExpect = "expect";
const char* const kExpires = "expires";
const char* const kFrom = "from";
const char* const kHost = "host";
const char* const kIfMatch = "if-match";
const char* const kIfModifiedSince = "if-modified-since";
const char* const kIfNoneMatch = "if-none-match";
const char* const kIfRange = "if-range";
const char* const kIfUnmodifiedSince = "if-unmodified-since";
const char* const kLastModified = "last-modified";
const char* const kLink = "link";
const char* const kLocation = "location";
const char* const kMaxForwards = "max-forwards";
const char* const kProxyAuthenticate = "proxy-authenticate";
const char* const kProxyAuthorization = "proxy-authorization";
const char* const kRange = "range";
const char* const kReferer = "referer";
const char* const kRefresh = "refresh";
const char* const kRetryAfter = "retry-after";
const char* const kServer = "server";
const char* const kSetCookie = "set-cookie";
const char* const kStrictTransportSecurity = "strict-transport-security";
const char* const kTransferEncoding = "transfer-encoding";
const char* const kUserAgent = "user-agent";
const char* const kVary = "vary";
const char* const kVia = "via";
const char* const kWwwAuthenticate = "www-authenticate";

const char* const kMethodHEAD = "HEAD";
const char* const kMethodGET = "GET";
const char* const kMethodPOST = "POST";
const char* const kMethodPUT = "PUT";
const char* const kMethodDELETE = "DELETE";
const char* const kMethodOPTIONS = "OPTIONS";

const char* const kSchemeHTTP = "http";
const char* const kSchemeHTTPS = "https";

}  // namespace headers
}  // namespace http2
