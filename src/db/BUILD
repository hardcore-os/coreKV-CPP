
cc_library(
    name = "DbLib",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    copts = ["-std=c++17"],
    deps = ["//logger:LogLib",
            "//cache:CacheLib",
            "//table:TableLib",
            "//utils:UtilsLib"],
    visibility = ["//visibility:public"],
)


cc_library(
    name = "StatusLib",
    srcs = glob(["status.cpp"]),
    hdrs = glob(["status.h"]),
    copts = ["-std=c++17"],
    visibility = ["//visibility:public"],
)