#pragma once

#include <chrono>
#include <string>

#include "envoy/upstream/cluster_manager.h"

#include "common/protobuf/utility.h"

#include "nats_streaming_filter.pb.h"

namespace Envoy {
namespace Http {

class NatsStreamingFilterConfig {

  using ProtoConfig = envoy::api::v2::filter::http::NatsStreaming;

public:
  NatsStreamingFilterConfig(const ProtoConfig &proto_config,
                            Upstream::ClusterManager &clusterManager)
      : op_timeout_(PROTOBUF_GET_MS_OR_DEFAULT(proto_config, op_timeout, 5000)),
        cluster_(proto_config.cluster()),
        max_connections_(proto_config.max_connections()) {
    if (max_connections_ != 1) {
      throw EnvoyException("nats-streaming filter: only one concurrent "
                           "connection is currently supported");
    }
    if (!clusterManager.get(cluster_)) {
      throw EnvoyException(fmt::format(
          "nats-streaming filter: unknown cluster '{}' in config", cluster_));
    }
  }

  const std::chrono::milliseconds &opTimeout() const { return op_timeout_; }
  const std::string &cluster() const { return cluster_; }
  uint32_t maxConnections() const { return max_connections_; }

private:
  std::chrono::milliseconds op_timeout_;
  std::string cluster_;
  uint32_t max_connections_;
};

typedef std::shared_ptr<NatsStreamingFilterConfig>
    NatsStreamingFilterConfigSharedPtr;

} // namespace Http
} // namespace Envoy
