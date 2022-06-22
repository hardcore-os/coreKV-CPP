cc_library(
    name = "TestLib",
    srcs = glob(["test_frame.cpp"]),
    hdrs = glob(["**/*.h"]),
    visibility = ["//visibility:public"],
)
cc_binary(
    name = "AllocTest",
    srcs = glob(["test_all.cpp"]),
    deps = [
        "//:CorekvLib",
        ":TestLib"],
)