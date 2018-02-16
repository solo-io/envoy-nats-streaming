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
    name = "nats_streaming_filter_config",
    hdrs = [
        "nats_streaming_filter_config.h",
    ],
    repository = "@envoy",
    deps = [
        "//:nats_streaming_filter_proto_cc",
        "//source/common/config:solo_well_known_names_lib",
        "@envoy//source/exe:envoy_common_lib",
    ],
)

envoy_cc_library(
    name = "subject_retriever_lib",
    srcs = [
        "metadata_subject_retriever.cc",
    ],
    hdrs = [
        "metadata_subject_retriever.h",
        "subject_retriever.h",
    ],
    repository = "@envoy",
    deps = [
        "@envoy//source/exe:envoy_common_lib",
        "@envoy_common//source/common/http:solo_filter_utility_lib",
    ],
)

envoy_cc_library(
    name = "nats_streaming_filter_lib",
    srcs = [
        "nats_streaming_filter.cc",
    ],
    hdrs = [
        "nats_streaming_filter.h",
    ],
    repository = "@envoy",
    deps = [
        "//include/envoy/nats:publisher_interface",
        "//source/common/http/filter:nats_streaming_filter_config",
        "//source/common/http/filter:subject_retriever_lib",
        "@envoy//source/exe:envoy_common_lib",
        "@envoy_common//source/common/http:solo_filter_utility_lib",
    ],
)
