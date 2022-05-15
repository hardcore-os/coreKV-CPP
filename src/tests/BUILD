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


cc_test(
    name = "cacheTest",
    srcs = glob(["cache_test.cpp"]),
    copts = ["-std=c++17"],
    deps = ["//utils:UtilsLib",
           "//cache:CacheLib",
           "@googletest//:gtest_main"],
)

cc_test(
    name = "footerTest",
    srcs = glob(["footer_test.cpp"]),
    copts = ["-std=c++17"],
    deps = ["//utils:UtilsLib",
           "//table:TableLib",
           "@googletest//:gtest_main"],
)


cc_test(
    name = "tableBuilderTest",
    srcs = glob(["table_builder_test.cpp"]),
    copts = ["-std=c++17"],
    deps = ["//utils:UtilsLib",
           "//table:TableLib",
           "//db:DbLib",
           "//logger:LogLib",
           "//file:FileLib",
           "//filter:FilterLib",
           "@googletest//:gtest_main"],
)