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
    # TODO(yuval-k): this has the nats suffix as it clashes with envoy's conn_pool. In the future
    # we want to remove this implementation as use the envoy's
    hdrs = ["conn_pool_nats.h"],
    repository = "@envoy",
    deps = [
        ":codec_interface",
        "@envoy//include/envoy/upstream:cluster_manager_interface",
    ],
)
