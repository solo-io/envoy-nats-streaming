licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_mock",
    "envoy_package",
)

envoy_package()

envoy_cc_mock(
    name = "tcp_mocks",
    srcs = ["mocks.cc"],
    hdrs = ["mocks_nats.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/tcp:conn_pool_interface",
        "//source/common/tcp:codec_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)
