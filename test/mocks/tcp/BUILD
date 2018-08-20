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

    # TODO(yuval-k): this has the nats suffix as it clashes with envoy's conn_pool. In the future
    # we want to remove this implementation as use the envoy's
    hdrs = ["mocks_nats.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/tcp:conn_pool_interface",
        "//source/common/tcp:codec_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)
