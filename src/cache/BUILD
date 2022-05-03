
cc_library(
    name = "CacheLib",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    copts = ["-std=c++17"],
    deps = ["//utils:UtilsLib"],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)