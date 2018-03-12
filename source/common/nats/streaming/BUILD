licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "client_lib",
    srcs = ["client_impl.cc"],
    hdrs = ["client_impl.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:codec_interface",
        "//include/envoy/nats/streaming:client_interface",
        "//include/envoy/tcp:conn_pool_interface",
        "//source/common/nats:message_builder_lib",
        "//source/common/nats:subject_utility_lib",
        "//source/common/nats:token_generator_lib",
        "//source/common/nats/streaming:message_utility_lib",
        "@envoy//source/exe:envoy_common_lib",
    ],
)

envoy_cc_library(
    name = "message_utility_lib",
    srcs = ["message_utility.cc"],
    hdrs = ["message_utility.h"],
    repository = "@envoy",
    deps = [
        "//:protocol_proto_cc",
    ],
)
