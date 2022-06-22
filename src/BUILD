cc_binary(
    name = "corekv",
    srcs = ["main.cpp"],
    deps = ["//:CorekvLib"]
)

cc_library(
    name = "CorekvLib",
    srcs = glob([
        "cache/**/*.cpp",
        "config/**/*.cpp",
        "table/**/*.cpp",
        "db/**/*.cpp",
        "file/**/*.cpp",
        "filter/**/*.cpp",
        "logger/**/*.cpp",
        "manifest/**/*.cpp",
        "utils/**/*.cpp",
        "memory/**/*.cpp"
    ]),
    hdrs = glob([
        "cache/**/*.h",
        "config/**/*.h",
        "db/**/*.h",
        "file/**/*.h",
        "table/**/*.h",
        "filter/**/*.h",
        "logger/**/*.h",
        "manifest/**/*.h",
        "utils/**/*.h",
        "memory/**/*.h"
    ]),
    linkstatic = True,
    strip_include_prefix = "",
    copts = ["-std=c++17"],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)