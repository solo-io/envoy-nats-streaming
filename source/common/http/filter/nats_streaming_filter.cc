#include "common/http/filter/nats_streaming_filter.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "envoy/http/header_map.h"
#include "envoy/nats/streaming/client.h"

#include "common/common/empty_string.h"
#include "common/common/macros.h"
#include "common/common/utility.h"
#include "common/http/filter_utility.h"
#include "common/http/solo_filter_utility.h"
#include "common/http/utility.h"

namespace Envoy {
namespace Http {

NatsStreamingFilter::NatsStreamingFilter(
    NatsStreamingFilterConfigSharedPtr config,
    SubjectRetrieverSharedPtr retreiver,
    Nats::Streaming::ClientPtr nats_streaming_client)
    : config_(config), subject_retriever_(retreiver),
      nats_streaming_client_(nats_streaming_client) {}

NatsStreamingFilter::~NatsStreamingFilter() {}

void NatsStreamingFilter::onDestroy() {

  if (in_flight_request_ != nullptr) {
    in_flight_request_->cancel();
    in_flight_request_ = nullptr;
  }
}

Envoy::Http::FilterHeadersStatus
NatsStreamingFilter::decodeHeaders(Envoy::Http::HeaderMap &headers,
                                   bool end_stream) {
  UNREFERENCED_PARAMETER(headers);
  RELEASE_ASSERT(isActive());

  if (end_stream) {
    relayToNatsStreaming();
  }

  return Envoy::Http::FilterHeadersStatus::StopIteration;
}

Envoy::Http::FilterDataStatus
NatsStreamingFilter::decodeData(Envoy::Buffer::Instance &data,
                                bool end_stream) {
  RELEASE_ASSERT(isActive());
  body_.move(data);

  if ((decoder_buffer_limit_.has_value()) &&
      ((body_.length() + data.length()) > decoder_buffer_limit_.value())) {

    decoder_callbacks_->sendLocalReply(Http::Code::PayloadTooLarge,
                                       "nats streaming paylaod too large",
                                       nullptr);
    return FilterDataStatus::StopIterationNoBuffer;
  }

  body_.move(data);

  if (end_stream) {
    relayToNatsStreaming();

    // TODO(talnordan): We need to make sure that life time of the buffer makes
    // sense.
    return Envoy::Http::FilterDataStatus::StopIterationNoBuffer;
  }

  return Envoy::Http::FilterDataStatus::StopIterationNoBuffer;
}

Envoy::Http::FilterTrailersStatus
NatsStreamingFilter::decodeTrailers(Envoy::Http::HeaderMap &) {
  RELEASE_ASSERT(isActive());

  relayToNatsStreaming();
  return Envoy::Http::FilterTrailersStatus::StopIteration;
}

bool NatsStreamingFilter::retrieveFunction(
    const MetadataAccessor &meta_accessor) {
  retrieveSubject(meta_accessor);
  return isActive();
}

void NatsStreamingFilter::onResponse() { onCompletion(Http::Code::OK, ""); }

void NatsStreamingFilter::onFailure() {
  onCompletion(Http::Code::InternalServerError, "nats streaming filter abort",
               RequestInfo::ResponseFlag::NoHealthyUpstream);
}

void NatsStreamingFilter::onTimeout() {
  onCompletion(Http::Code::RequestTimeout, "nats streaming filter timeout",
               RequestInfo::ResponseFlag::UpstreamRequestTimeout);
}

void NatsStreamingFilter::retrieveSubject(
    const MetadataAccessor &meta_accessor) {
  optional_subject_ = subject_retriever_->getSubject(meta_accessor);
}

void NatsStreamingFilter::relayToNatsStreaming() {
  RELEASE_ASSERT(optional_subject_.has_value());
  RELEASE_ASSERT(!optional_subject_.value().subject->empty());

  const std::string *cluster_name =
      SoloFilterUtility::resolveClusterName(decoder_callbacks_);
  if (!cluster_name) {
    // TODO(talnordan): Consider changing the return type to `bool` and
    // returning `false`.
    return;
  }

  auto &&subject_entry = optional_subject_.value();
  const std::string &subject = *subject_entry.subject;
  const std::string &cluster_id = *subject_entry.cluster_id;
  const std::string &discover_prefix = *subject_entry.discover_prefix;

  in_flight_request_ = nats_streaming_client_->makeRequest(
      subject, cluster_id, discover_prefix, body_, *this);
}

void NatsStreamingFilter::onCompletion(Code response_code,
                                       const std::string &body_text) {
  in_flight_request_ = nullptr;

  decoder_callbacks_->sendLocalReply(response_code, body_text, nullptr);
}

void NatsStreamingFilter::onCompletion(
    Code response_code, const std::string &body_text,
    RequestInfo::ResponseFlag response_flag) {
  decoder_callbacks_->requestInfo().setResponseFlag(response_flag);
  onCompletion(response_code, body_text);
}

} // namespace Http
} // namespace Envoy
