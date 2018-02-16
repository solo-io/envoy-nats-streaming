#pragma once

#include "envoy/nats/publisher.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

#include "server/config/network/http_connection_manager.h"

#include "nats_streaming_filter.pb.h"
#include "nats_streaming_filter_config.h"
#include "subject_retriever.h"

namespace Envoy {
namespace Http {

using Envoy::Upstream::ClusterManager;

class NatsStreamingFilter : public StreamDecoderFilter,
                            public Nats::Publisher::PublishCallbacks,
                            public Logger::Loggable<Logger::Id::filter> {
public:
  NatsStreamingFilter(NatsStreamingFilterConfigSharedPtr,
                      SubjectRetrieverSharedPtr, ClusterManager &,
                      Nats::Publisher::InstancePtr);
  ~NatsStreamingFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(HeaderMap &, bool) override;
  FilterDataStatus decodeData(Buffer::Instance &, bool) override;
  FilterTrailersStatus decodeTrailers(HeaderMap &) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks &) override;

  // Nats::Publisher::PublishCallbacks
  virtual void onResponse() override;

private:
  void retrieveSubject();

  inline bool isActive() { return optional_subject_.valid(); }

  void relayToNatsStreaming();

  const NatsStreamingFilterConfigSharedPtr config_;
  SubjectRetrieverSharedPtr subject_retriever_;
  ClusterManager &cm_;
  Nats::Publisher::InstancePtr publisher_;
  StreamDecoderFilterCallbacks *callbacks_{};
  bool stream_destroyed_{};
  Optional<Subject> optional_subject_;
};

} // namespace Http
} // namespace Envoy
