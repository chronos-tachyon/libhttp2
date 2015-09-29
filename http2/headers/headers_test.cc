#include "http2/headers/headers.h"

#include "gtest/gtest.h"

using http2::headers::Header;
using http2::headers::Headers;

TEST(Header, EQ) {
  Header a = {"foo", "bar"};
  Header b = {"foo", "bar"};
  EXPECT_EQ(a, b);
  Header c = {"foo", "baz"};
  EXPECT_NE(a, c);
  Header d = {"baz", "bar"};
  EXPECT_NE(a, d);
}

TEST(Header, LT) {
  Header a = {"foo", "a"};
  EXPECT_FALSE(a < a);
  EXPECT_FALSE(a > a);
  EXPECT_GE(a, a);
  EXPECT_LE(a, a);

  Header b = {"foo", "b"};
  EXPECT_LT(a, b);
  EXPECT_LE(a, b);
  EXPECT_GT(b, a);
  EXPECT_GE(b, a);
}

static Headers dummy_request() {
  Headers headers;
  headers.add(":method", "GET");
  headers.add(":scheme", "http");
  headers.add(":path", "/");
  headers.add(":authority", "www.example.com");
  headers.add("cache-control", "max-age=60");
  headers.add("cache-control", "max-stale=3600");
  return headers;
}

static std::vector<std::string> header_names(const Headers& headers) {
  std::vector<std::string> result;
  for (const auto& header : headers.all()) {
    result.push_back(header.name);
  }
  return result;
}

TEST(Headers, Find) {
  Headers headers = dummy_request();

  auto first_method = headers.first(":method");
  EXPECT_TRUE(first_method.first);
  EXPECT_EQ(first_method.second, "GET");
  auto last_method = headers.last(":method");
  EXPECT_TRUE(last_method.first);
  EXPECT_EQ(last_method.second, "GET");
  auto every_method = headers.every(":method");
  EXPECT_EQ(every_method.size(), 1);
  EXPECT_EQ(every_method.at(0), "GET");

  auto first_cachecontrol = headers.first("cache-control");
  EXPECT_TRUE(first_cachecontrol.first);
  EXPECT_EQ(first_cachecontrol.second, "max-age=60");
  auto last_cachecontrol = headers.last("cache-control");
  EXPECT_TRUE(last_cachecontrol.first);
  EXPECT_EQ(last_cachecontrol.second, "max-stale=3600");
  auto every_cachecontrol = headers.every("cache-control");
  EXPECT_EQ(every_cachecontrol.size(), 2);
  EXPECT_EQ(every_cachecontrol.at(0), "max-age=60");
  EXPECT_EQ(every_cachecontrol.at(1), "max-stale=3600");

  auto first_status = headers.first(":status");
  EXPECT_FALSE(first_status.first);
  EXPECT_EQ(first_status.second, "");
  auto last_status = headers.last(":status");
  EXPECT_FALSE(last_status.first);
  EXPECT_EQ(last_status.second, "");
  auto every_status = headers.every(":status");
  EXPECT_EQ(every_status.size(), 0);
}

TEST(Headers, Replace) {
  Headers headers = dummy_request();
  headers.add("cookie", "a=b");

  auto names = header_names(headers);
  EXPECT_EQ(names.size(), 7);
  EXPECT_EQ(names.at(4), "cache-control");
  EXPECT_EQ(names.at(5), "cache-control");
  EXPECT_EQ(names.at(6), "cookie");

  headers.replace("cache-control", "no-cache");

  names = header_names(headers);
  EXPECT_EQ(names.size(), 6);
  EXPECT_EQ(names.at(4), "cache-control");
  EXPECT_EQ(names.at(5), "cookie");
 
  auto every_cachecontrol = headers.every("cache-control");
  EXPECT_EQ(every_cachecontrol.size(), 1);
  EXPECT_EQ(every_cachecontrol.at(0), "no-cache");
  auto every_cookie = headers.every("cookie");
  EXPECT_EQ(every_cookie.size(), 1);
  EXPECT_EQ(every_cookie.at(0), "a=b");
}

TEST(Headers, Remove) {
  Headers headers = dummy_request();
  headers.add("cookie", "a=b");

  auto names = header_names(headers);
  EXPECT_EQ(names.size(), 7);
  EXPECT_EQ(names.at(4), "cache-control");
  EXPECT_EQ(names.at(5), "cache-control");
  EXPECT_EQ(names.at(6), "cookie");

  headers.remove("cache-control");

  names = header_names(headers);
  EXPECT_EQ(names.size(), 5);
  EXPECT_EQ(names.at(4), "cookie");
}
