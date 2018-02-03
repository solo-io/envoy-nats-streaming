licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "conn_pool_impl_test",
    srcs = ["conn_pool_impl_test.cc"],
    repository = "@envoy",
    deps = [
        "//source/common/tcp:conn_pool_lib",
        "//test/mocks/tcp:tcp_mocks",
        "@envoy//source/common/event:dispatcher_lib",
        "@envoy//source/common/network:utility_lib",
        "@envoy//source/common/upstream:upstream_includes",
        "@envoy//source/common/upstream:upstream_lib",
        "@envoy//test/mocks/network:network_mocks",
        "@envoy//test/mocks/thread_local:thread_local_mocks",
        "@envoy//test/mocks/upstream:upstream_mocks",
    ],
)
