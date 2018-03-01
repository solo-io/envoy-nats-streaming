workspace(name="nats_streaming_filter")

# Use skylark for native Git.
load('@bazel_tools//tools/build_defs/repo:git.bzl', 'git_repository')

ENVOY_SHA = "f79a62b7cc9ca55d20104379ee0576617630cdaa"  # Feb 15, 2018 ( test: fix nit after #2591 (#2601) )

http_archive(
    name = "envoy",
    strip_prefix = "envoy-" + ENVOY_SHA,
    url = "https://github.com/envoyproxy/envoy/archive/" + ENVOY_SHA + ".zip",
)

ENVOY_COMMON_SHA = "ee625c53848d990dd7e5b87c5fbcd2776054e9f8"  # Feb 26, 2018 ( split metadata accessor)

git_repository(
    name = "solo_envoy_common",
    remote = "git@github.com:solo-io/envoy-common",
    commit = ENVOY_COMMON_SHA,
)

load("@envoy//bazel:repositories.bzl", "envoy_dependencies")
load("@envoy//bazel:cc_configure.bzl", "cc_configure")

envoy_dependencies()

cc_configure()

load("@envoy_api//bazel:repositories.bzl", "api_dependencies")
api_dependencies()

load("@io_bazel_rules_go//go:def.bzl", "go_rules_dependencies", "go_register_toolchains")
load("@com_lyft_protoc_gen_validate//bazel:go_proto_library.bzl", "go_proto_repositories")
go_proto_repositories(shared=0)
go_rules_dependencies()
go_register_toolchains()
load("@io_bazel_rules_go//proto:def.bzl", "proto_register_toolchains")
proto_register_toolchains()