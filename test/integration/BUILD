licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "nats_streaming_filter_integration_test",
    srcs = ["nats_streaming_filter_integration_test.cc"],
    repository = "@envoy",
    deps = [
        "//:filter_lib",
        "@envoy//test/integration:http_integration_lib",
        "@envoy//test/integration:integration_lib",
    ],
)
