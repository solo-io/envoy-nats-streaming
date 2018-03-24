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
    repository = "@envoy",
    deps = [
        "@envoy//source/exe:envoy_common_lib",
    ],
)
