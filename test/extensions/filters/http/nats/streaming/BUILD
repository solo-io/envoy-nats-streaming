licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_mock",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "metadata_subject_retriever_test",
    srcs = ["metadata_subject_retriever_test.cc"],
    repository = "@envoy",
    deps = [
        "//source/extensions/filters/http/nats/streaming:nats_streaming_filter_lib",
        "@envoy//test/test_common:utility_lib",
    ],
)

envoy_cc_test(
    name = "nats_streaming_filter_test",
    srcs = ["nats_streaming_filter_test.cc"],
    repository = "@envoy",
    deps = [
        ":mocks",
        "//source/extensions/filters/http/nats/streaming:nats_streaming_filter_config_lib",
        "//test/mocks/nats/streaming:nats_streaming_mocks",
        "@envoy//test/mocks/http:http_mocks",
        "@envoy//test/mocks/server:server_mocks",
        "@envoy//test/mocks/upstream:upstream_mocks",
    ],
)

envoy_cc_mock(
    name = "mocks",
    srcs = ["mocks.cc"],
    hdrs = ["mocks.h"],
    repository = "@envoy",
    deps = [
        "//source/extensions/filters/http/nats/streaming:nats_streaming_filter_lib",
    ],
)
