cc_library(
  name = "constants",
  srcs = ["constants.cc"],
  hdrs = ["constants.h"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "error",
  srcs = ["error.cc"],
  hdrs = ["error.h"],
)

cc_library(
  name = "frame",
  srcs = ["frame.cc"],
  hdrs = ["frame.h"],
)

cc_library(
  name = "headers",
  srcs = ["headers.cc"],
  hdrs = ["headers.h"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "hpack",
  srcs = [
    "hpack-decode.cc",
    "hpack-encode.cc",
    "hpack-huffman.cc",
    "hpack-table.cc",
  ],
  hdrs = ["hpack.h"],
  deps = [
    ":headers",
    "//http2/headers",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "settings",
  srcs = ["settings.cc"],
  hdrs = ["settings.h"],
)

cc_test(
  name = "headers_test",
  srcs = ["headers_test.cc"],
  deps = [
    ":headers",
    "//third_party:gtest",
  ],
  size = "small",
)

cc_test(
  name = "hpack_test",
  srcs = ["hpack_test.cc"],
  deps = [
    ":hpack",
    "//third_party:gtest",
  ],
  size = "small",
)
