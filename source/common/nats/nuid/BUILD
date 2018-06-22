licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "nuid_lib",
    srcs = ["nuid.cc"],
    hdrs = ["nuid.h"],
    external_deps = ["abseil_optional"],
    repository = "@envoy",
    deps = [
        "@envoy//include/envoy/runtime:runtime_interface",
        "@envoy//source/common/common:assert_lib",
    ],
)
