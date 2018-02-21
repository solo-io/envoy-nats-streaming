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

sh_test(
    name = "e2e_test",
    srcs = ["e2e_test.sh"],
    data = [
        "//:envoy",
        "//e2e:create_config.sh",
    ],
)
