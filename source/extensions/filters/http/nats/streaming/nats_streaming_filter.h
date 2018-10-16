#pragma once

#include "envoy/nats/streaming/client.h"

#include "common/http/functional_stream_decoder_base.h"

#include "extensions/filters/http/nats/streaming/nats_streaming_filter_config.h"
#include "extensions/filters/http/nats/streaming/subject_retriever.h"

#include "nats_streaming_filter.pb.h"
#include "payload.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

using Upstream::ClusterManager;

class NatsStreamingFilter : public Http::StreamDecoderFilter,
                            public Http::FunctionalFilter,
                            public Envoy::Nats::Streaming::PublishCallbacks {
public:
  NatsStreamingFilter(NatsStreamingFilterConfigSharedPtr config,
                      SubjectRetrieverSharedPtr retreiver,
                      Envoy::Nats::Streaming::ClientPtr nats_streaming_client);
  ~NatsStreamingFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::HeaderMap &, bool) override;
  Http::FilterDataStatus decodeData(Buffer::Instance &, bool) override;
  Http::FilterTrailersStatus decodeTrailers(Http::HeaderMap &) override;

  void setDecoderFilterCallbacks(
      Http::StreamDecoderFilterCallbacks &decoder_callbacks) override {
    decoder_callbacks_ = &decoder_callbacks;
    auto decoder_limit = decoder_callbacks.decoderBufferLimit();
    if (decoder_limit > 0) {
      decoder_buffer_limit_ = decoder_limit;
    }
  }

  bool retrieveFunction(const Http::MetadataAccessor &meta_accessor) override;

  // Nats::Streaming::PublishCallbacks
  virtual void onResponse() override;
  virtual void onFailure() override;
  virtual void onTimeout() override;

private:
  void retrieveSubject(const Http::MetadataAccessor &meta_accessor);

  inline bool isActive() { return optional_subject_.has_value(); }

  void relayToNatsStreaming();

  inline void onCompletion(Http::Code response_code,
                           const std::string &body_text);

  inline void onCompletion(Http::Code response_code,
                           const std::string &body_text,
                           RequestInfo::ResponseFlag response_flag);

  const NatsStreamingFilterConfigSharedPtr config_;
  SubjectRetrieverSharedPtr subject_retriever_;
  Envoy::Nats::Streaming::ClientPtr nats_streaming_client_;
  absl::optional<Subject> optional_subject_;
  Http::StreamDecoderFilterCallbacks *decoder_callbacks_{};
  absl::optional<uint32_t> decoder_buffer_limit_{};
  pb::Payload payload_;
  Buffer::OwnedImpl body_{};
  Envoy::Nats::Streaming::PublishRequestPtr in_flight_request_{};
};

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
