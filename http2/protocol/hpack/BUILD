cc_library(
  name = "hpack",
  srcs = [
    "hpack-decode.cc",
    "hpack-encode.cc",
    "hpack-huffman.cc",
    "hpack-table.cc",
  ],
  hdrs = ["hpack.h"],
  deps = ["//http2/headers"],
  visibility = ["//visibility:public"],
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
