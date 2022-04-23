cc_test(
    name = "allocTest",
    srcs = glob(["all_test.cpp"]),
    copts = ["-std=c++17"],
    deps = ["//memory:MemoryLib",
           "//logger:LogLib",
           "@googletest//:gtest_main"],
)

cc_test(
    name = "skiplistTest",
    srcs = glob(["skiplist_test.cpp"]),
    copts = ["-std=c++17"],
    deps = ["//memory:MemoryLib",
           "//db:DbLib",
           "@googletest//:gtest_main"],
)


cc_test(
    name = "bloomFilterTest",
    srcs = glob(["bloomfilter_test.cpp"]),
    copts = ["-std=c++17"],
    deps = ["//utils:UtilsLib",
           "//filter:FilterLib",
           "@googletest//:gtest_main"],
)