licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "codec_interface",
    hdrs = ["codec.h"],
    repository = "@envoy",
    deps = ["@envoy//include/envoy/buffer:buffer_interface"],
)

envoy_cc_library(
    name = "conn_pool_interface",
    hdrs = ["conn_pool.h"],
    repository = "@envoy",
    deps = [
        ":codec_interface",
        "@envoy//include/envoy/upstream:cluster_manager_interface",
    ],
)
