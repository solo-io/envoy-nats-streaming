licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "nats_streaming_filter_test",
    srcs = ["nats_streaming_filter_test.cc"],
    repository = "@envoy",
    deps = [
        "//:nats_streaming_filter_lib",
        "//test/mocks/http/filter:http_filter_mocks",
        "//test/mocks/nats:nats_mocks",
        "@envoy//test/mocks/http:http_mocks",
        "@envoy//test/mocks/upstream:upstream_mocks",
    ],
)
