licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

load("@envoy_api//bazel:api_build_system.bzl", "api_proto_library")

api_proto_library(
    name = "nats_streaming_filter_proto",
    srcs = ["nats_streaming_filter.proto"],
)

api_proto_library(
    name = "payload_proto",
    srcs = ["payload.proto"],
)

api_proto_library(
    name = "protocol_proto",
    srcs = ["protocol.proto"],
)

envoy_cc_library(
    name = "filter_lib",
    repository = "@envoy",
    visibility = ["//visibility:public"],
    deps = [
        "//source/extensions/filters/http/nats/streaming:nats_streaming_filter_config_lib",
    ],
)

envoy_cc_binary(
    name = "envoy",
    repository = "@envoy",
    deps = [
        ":filter_lib",
        "@envoy//source/exe:envoy_main_entry_lib",
    ],
)
