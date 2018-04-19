workspace(name="nats_streaming_filter")

# Use skylark for native Git.
load('@bazel_tools//tools/build_defs/repo:git.bzl', 'git_repository')

ENVOY_SHA = "d41d06eb614fd49f19422d9eed9235c320af9229"  # April 10, 2018 (gRPC/JSON transcoder: enable preserving route after headers modified)

http_archive(
    name = "envoy",
    strip_prefix = "envoy-" + ENVOY_SHA,
    url = "https://github.com/envoyproxy/envoy/archive/" + ENVOY_SHA + ".zip",
)

ENVOY_COMMON_SHA = "ef1a5da85d62ba90a28bb76a54912692b8850094"  # Apr 19, 2018 (Introduce `class BufferUtility`)

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