# Envoy NATS Streaming filter

This project links a NATS Streaming HTTP filter with the Envoy binary.
A new filter `io.solo.nats_streaming` which redirects requests to NATS Streaming is introduced.

## Building

To build the Envoy static binary:

`bazel build //:envoy`

## Testing

To run the all tests:

`bazel test //test/... -c dbg`

To run integration tests using a clang build:

`CXX=clang++-5.0 CC=clang-5.0  bazel test -c dbg --config=clang-tsan //test/integration:nats_streaming_filter_integration_test --runs_per_test=10`
