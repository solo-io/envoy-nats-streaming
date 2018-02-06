#pragma once

#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

#include "server/config/network/http_connection_manager.h"

#include "metadata_topic_retriever.h"
#include "nats_streaming_filter.pb.h"
#include "nats_streaming_filter_config.h"

namespace Envoy {
namespace Http {

using Envoy::Upstream::ClusterManager;

class NatsStreamingFilter : public StreamDecoderFilter,
                            public Logger::Loggable<Logger::Id::filter> {
public:
  NatsStreamingFilter(NatsStreamingFilterConfigSharedPtr,
                      TopicRetrieverSharedPtr, ClusterManager &);
  ~NatsStreamingFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(HeaderMap &, bool) override;
  FilterDataStatus decodeData(Buffer::Instance &, bool) override;
  FilterTrailersStatus decodeTrailers(HeaderMap &) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks &) override;

private:
  void retrieveTopic();

  inline bool isActive() { return optionalTopic_.valid(); }

  void relayToNatsStreaming();

  const NatsStreamingFilterConfigSharedPtr config_;
  TopicRetrieverSharedPtr topicRetriever_;
  ClusterManager &cm_;
  StreamDecoderFilterCallbacks *callbacks_{};
  bool stream_destroyed_{};
  Optional<Topic> optionalTopic_;
};

} // namespace Http
} // namespace Envoy
