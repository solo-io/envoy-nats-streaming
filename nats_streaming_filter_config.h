#pragma once

#include <string>

#include "nats_streaming_filter.pb.h"

namespace Envoy {
namespace Http {

class NatsStreamingFilterConfig {

  using ProtoConfig = envoy::api::v2::filter::http::NatsStreaming;

public:
  NatsStreamingFilterConfig(const ProtoConfig &proto_config)
      : op_timeout_(PROTOBUF_GET_MS_REQUIRED(proto_config, op_timeout)) {}

  const std::chrono::milliseconds &op_timeout() const { return op_timeout_; }

private:
  const std::chrono::milliseconds op_timeout_;
};

typedef std::shared_ptr<NatsStreamingFilterConfig>
    NatsStreamingFilterConfigSharedPtr;

} // namespace Http
} // namespace Envoy
