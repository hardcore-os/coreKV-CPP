
cc_library(
    name = "LogLib",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    linkopts = ["-pthread"],
    copts = ["-std=c++17"],
    deps = ["//utils:UtilsLib"],
    visibility = ["//visibility:public"],
)