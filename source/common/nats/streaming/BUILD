licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "message_builder_lib",
    srcs = ["message_builder.cc"],
    hdrs = ["message_builder.h"],
    repository = "@envoy",
    deps = [
        "//:protocol_proto_cc",
    ],
)
