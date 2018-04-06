workspace(name="nats_streaming_filter")

# Use skylark for native Git.
load('@bazel_tools//tools/build_defs/repo:git.bzl', 'git_repository')

ENVOY_SHA = "4dd49d8809f7aaa580538b3c228dd99a2fae92a4"  # Mar 20, 2018 (v1.6.0)

http_archive(
    name = "envoy",
    strip_prefix = "envoy-" + ENVOY_SHA,
    url = "https://github.com/envoyproxy/envoy/archive/" + ENVOY_SHA + ".zip",
)

ENVOY_COMMON_SHA = "2bc9569aec056df65bb4f67f0c47be968cac6256"  # Apr 3, 2018 (Fix compilation of `SoloMetadata` (#8))

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