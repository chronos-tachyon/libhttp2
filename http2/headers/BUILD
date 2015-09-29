cc_library(
  name = "headers",
  srcs = [
    "constants.cc",
    "headers.cc",
  ],
  hdrs = [
    "constants.h",
    "headers.h",
  ],
  visibility = ["//visibility:public"],
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
