licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "codec_lib",
    hdrs = ["codec_impl.h"],
    repository = "@envoy",
    deps = [
        "//include/envoy/tcp:codec_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
        "@envoy//source/common/common:utility_lib",
    ],
)

envoy_cc_library(
    name = "conn_pool_lib",
    hdrs = ["conn_pool_impl.h"],
    repository = "@envoy",
    deps = [
        ":codec_lib",
        "//include/envoy/tcp:conn_pool_interface",
        "@envoy//include/envoy/router:router_interface",
        "@envoy//include/envoy/thread_local:thread_local_interface",
        "@envoy//include/envoy/upstream:cluster_manager_interface",
        "@envoy//source/common/buffer:buffer_lib",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/network:filter_lib",
        "@envoy//source/common/protobuf:utility_lib",
    ],
)
