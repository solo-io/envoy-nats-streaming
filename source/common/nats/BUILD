licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "codec_lib",
    srcs = ["codec_impl.cc"],
    hdrs = ["codec_impl.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:codec_interface",
        "//include/envoy/tcp:codec_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
        "@envoy//source/common/common:utility_lib",
    ],
)

envoy_cc_library(
    name = "publisher_lib",
    srcs = ["publisher_impl.cc"],
    hdrs = ["publisher_impl.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:codec_interface",
        "//include/envoy/nats:publisher_interface",
        "//include/envoy/tcp:conn_pool_interface",
        "@envoy//source/exe:envoy_common_lib",
    ],
)
