licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "codec_impl_test",
    srcs = ["codec_impl_test.cc"],
    repository = "@envoy",
    deps = [
        "//source/common/nats:codec_lib",
        "//test/mocks/nats:nats_mocks",
        "@envoy//source/common/buffer:buffer_lib",
        "@envoy//source/common/common:assert_lib",
        "@envoy//test/test_common:utility_lib",
    ],
)

envoy_cc_test(
    name = "message_builder_test",
    srcs = ["message_builder_test.cc"],
    repository = "@envoy",
    deps = [
        "//source/common/nats:message_builder_lib",
        "//test/mocks/nats:nats_mocks",
        "@envoy//source/common/common:assert_lib",
        "@envoy//test/test_common:utility_lib",
    ],
)

envoy_cc_test(
    name = "subject_utility_test",
    srcs = ["subject_utility_test.cc"],
    repository = "@envoy",
    deps = [
        "//source/common/nats:subject_utility_lib",
        "@envoy//source/common/common:assert_lib",
    ],
)
