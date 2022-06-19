
cc_library(
    name = "FileLib",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    copts = ["-std=c++17"],
    deps = ["//db:StatusLib",
            "//logger:LogLib"],
    visibility = ["//visibility:public"],
)