licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "client_interface",
    hdrs = ["client.h"],
    repository = "@envoy",
)

envoy_cc_library(
    name = "inbox_handler_interface",
    hdrs = ["inbox_handler.h"],
    repository = "@envoy",
)
