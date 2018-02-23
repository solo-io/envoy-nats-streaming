#pragma once

#include "envoy/nats/publisher.h"

#include "common/http/filter/nats_streaming_filter_config.h"
#include "common/http/filter/subject_retriever.h"
#include "common/http/functional_stream_decoder_base.h"

#include "nats_streaming_filter.pb.h"

namespace Envoy {
namespace Http {

using Envoy::Upstream::ClusterManager;

class NatsStreamingFilter : public FunctionalFilterBase,
                            public Nats::Publisher::PublishCallbacks {
public:
  NatsStreamingFilter(Server::Configuration::FactoryContext &ctx,
                      const std::string &name,
                      NatsStreamingFilterConfigSharedPtr config,
                      SubjectRetrieverSharedPtr retreiver,
                      Nats::Publisher::InstancePtr publisher);
  ~NatsStreamingFilter();

  // Http::FunctionalFilterBase
  FilterHeadersStatus functionDecodeHeaders(HeaderMap &, bool) override;
  FilterDataStatus functionDecodeData(Buffer::Instance &, bool) override;
  FilterTrailersStatus functionDecodeTrailers(HeaderMap &) override;
  bool retrieveFunction(const MetadataAccessor &meta_accessor) override;

  // Nats::Publisher::PublishCallbacks
  virtual void onResponse() override;
  virtual void onFailure() override;

private:
  void retrieveSubject(const MetadataAccessor &meta_accessor);

  inline bool isActive() { return optional_subject_.valid(); }

  void relayToNatsStreaming();

  const NatsStreamingFilterConfigSharedPtr config_;
  SubjectRetrieverSharedPtr subject_retriever_;
  Nats::Publisher::InstancePtr publisher_;
  bool stream_destroyed_{};
  Optional<Subject> optional_subject_;
};

} // namespace Http
} // namespace Envoy
