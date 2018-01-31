#pragma once

#include <string>

#include "nats_streaming_filter.pb.h"

namespace Envoy {
namespace Http {

class NatsStreamingFilterConfig {

  using ProtoConfig = envoy::api::v2::filter::http::NatsStreaming;

public:
  NatsStreamingFilterConfig(const ProtoConfig &proto_config)
      : placeholder_(proto_config.placeholder()) {}

  const std::string &placeholder() const { return placeholder_; }

private:
  const std::string placeholder_;
};

typedef std::shared_ptr<NatsStreamingFilterConfig>
    NatsStreamingFilterConfigSharedPtr;

} // namespace Http
} // namespace Envoy
