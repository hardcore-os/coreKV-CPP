
cc_library(
    name = "TableLib",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    copts = ["-std=c++17"],
    deps = ["//utils:UtilsLib",
    "//db:DbLib",
    "//filter:FilterLib",
    "//file:FileLib"],
    visibility = ["//visibility:public"],
)