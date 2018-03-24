licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_cc_test(
    name = "nuid_test",
    srcs = ["nuid_test.cc"],
    repository = "@envoy",
    deps = [
        "//source/common/nats/nuid:nuid_lib",
        "@envoy//source/common/common:assert_lib",
        "@envoy//test/mocks/runtime:runtime_mocks",
    ],
)
