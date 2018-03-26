licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "codec_lib",
    srcs = ["codec_impl.cc"],
    hdrs = ["codec_impl.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:codec_interface",
        "//include/envoy/tcp:codec_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
        "@envoy//source/common/common:utility_lib",
    ],
)

envoy_cc_library(
    name = "message_builder_lib",
    srcs = ["message_builder.cc"],
    hdrs = ["message_builder.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:codec_interface",
    ],
)

envoy_cc_library(
    name = "subject_utility_lib",
    hdrs = ["subject_utility.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:token_generator_interface",
        "@envoy//source/exe:envoy_common_lib",
    ],
)

envoy_cc_library(
    name = "token_generator_lib",
    srcs = ["token_generator_impl.cc"],
    hdrs = ["token_generator_impl.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:token_generator_interface",
        "//source/common/nats/nuid:nuid_lib",
        "@envoy//source/exe:envoy_common_lib",
    ],
)
