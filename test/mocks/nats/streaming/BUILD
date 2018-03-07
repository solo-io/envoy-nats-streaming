licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_mock",
    "envoy_package",
)

envoy_package()

envoy_cc_mock(
    name = "nats_streaming_mocks",
    srcs = ["mocks.cc"],
    hdrs = ["mocks.h"],
    repository = "@envoy",
    deps = [
        "//source/common/nats/streaming:publisher_lib",
    ],
)
