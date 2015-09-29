// String constants for frequently occurring HTTP/2 headers.
// To prevent typos, use these instead of typing the strings by hand.

#ifndef HTTP2_HEADERS_HEADERS_H
#define HTTP2_HEADERS_HEADERS_H

namespace http2 {
namespace headers {

extern const char* const kMethod;
extern const char* const kScheme;
extern const char* const kAuthority;
extern const char* const kPath;
extern const char* const kStatus;

extern const char* const kAcceptCharset;
extern const char* const kAcceptEncoding;
extern const char* const kAcceptLanguage;
extern const char* const kAcceptRanges;
extern const char* const kAccept;
extern const char* const kAccessControlAllowOrigin;
extern const char* const kAge;
extern const char* const kAllow;
extern const char* const kAuthorization;
extern const char* const kCacheControl;
extern const char* const kContentDisposition;
extern const char* const kContentEncoding;
extern const char* const kContentLanguage;
extern const char* const kContentLength;
extern const char* const kContentLocation;
extern const char* const kContentRange;
extern const char* const kContentType;
extern const char* const kCookie;
extern const char* const kDate;
extern const char* const kETag;
extern const char* const kExpect;
extern const char* const kExpires;
extern const char* const kFrom;
extern const char* const kHost;
extern const char* const kIfMatch;
extern const char* const kIfModifiedSince;
extern const char* const kIfNoneMatch;
extern const char* const kIfRange;
extern const char* const kIfUnmodifiedSince;
extern const char* const kLastModified;
extern const char* const kLink;
extern const char* const kLocation;
extern const char* const kMaxForwards;
extern const char* const kProxyAuthenticate;
extern const char* const kProxyAuthorization;
extern const char* const kRange;
extern const char* const kReferer;
extern const char* const kRefresh;
extern const char* const kRetryAfter;
extern const char* const kServer;
extern const char* const kSetCookie;
extern const char* const kStrictTransportSecurity;
extern const char* const kTransferEncoding;
extern const char* const kUserAgent;
extern const char* const kVary;
extern const char* const kVia;
extern const char* const kWwwAuthenticate;

extern const char* const kMethodHEAD;
extern const char* const kMethodGET;
extern const char* const kMethodPOST;
extern const char* const kMethodPUT;
extern const char* const kMethodDELETE;
extern const char* const kMethodOPTIONS;

extern const char* const kSchemeHTTP;
extern const char* const kSchemeHTTPS;

}  // namespace headers
}  // namespace http2

#endif  // HTTP2_HEADERS_HEADERS_H
