licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "nats_streaming_filter_config_test",
    srcs = ["nats_streaming_filter_config_test.cc"],
    repository = "@envoy",
    deps = [
        "//source/server/config/http:nats_streaming_filter_config_lib",
        "@envoy//test/test_common:utility_lib",
    ],
)
