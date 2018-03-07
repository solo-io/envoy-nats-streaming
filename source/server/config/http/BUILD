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

envoy_cc_library(
    name = "nats_streaming_filter_config_lib",
    srcs = ["nats_streaming_filter_config_factory.cc"],
    hdrs = ["nats_streaming_filter_config_factory.h"],
    repository = "@envoy",
    deps = [
        "//source/common/config:nats_streaming_well_known_names_lib",
        "//source/common/http/filter:nats_streaming_filter_lib",
        "//source/common/nats:codec_lib",
        "//source/common/nats/streaming:publisher_lib",
        "//source/common/tcp:conn_pool_lib",
        "@envoy//source/exe:envoy_common_lib",
    ],
)
