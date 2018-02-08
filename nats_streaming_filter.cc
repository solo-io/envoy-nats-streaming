#include "nats_streaming_filter.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "envoy/http/header_map.h"

#include "common/common/empty_string.h"
#include "common/common/macros.h"
#include "common/common/utility.h"
#include "common/http/filter_utility.h"
#include "common/http/utility.h"

#include "server/config/network/http_connection_manager.h"

#include "solo_filter_utility.h"

namespace Envoy {
namespace Http {

NatsStreamingFilter::NatsStreamingFilter(
    NatsStreamingFilterConfigSharedPtr config,
    SubjectRetrieverSharedPtr subject_retriever, ClusterManager &cm,
    Nats::Publisher::InstancePtr publisher)
    : config_(config), subject_retriever_(subject_retriever), cm_(cm),
      publisher_(publisher) {}

NatsStreamingFilter::~NatsStreamingFilter() {}

void NatsStreamingFilter::onDestroy() { stream_destroyed_ = true; }

Envoy::Http::FilterHeadersStatus
NatsStreamingFilter::decodeHeaders(Envoy::Http::HeaderMap &headers,
                                   bool end_stream) {
  UNREFERENCED_PARAMETER(headers);

  retrieveSubject();

  if (!isActive()) {
    return Envoy::Http::FilterHeadersStatus::Continue;
  }

  ENVOY_LOG(debug, "decodeHeaders called end = {}", end_stream);

  if (end_stream) {
    relayToNatsStreaming();
  }

  return Envoy::Http::FilterHeadersStatus::StopIteration;
}

Envoy::Http::FilterDataStatus
NatsStreamingFilter::decodeData(Envoy::Buffer::Instance &data,
                                bool end_stream) {

  if (!isActive()) {
    return Envoy::Http::FilterDataStatus::Continue;
  }

  ENVOY_LOG(debug, "decodeData called end = {} data = {}", end_stream,
            data.length());

  if (end_stream) {
    relayToNatsStreaming();
    return Envoy::Http::FilterDataStatus::StopIterationNoBuffer;
  }

  return Envoy::Http::FilterDataStatus::StopIterationAndBuffer;
}

Envoy::Http::FilterTrailersStatus
NatsStreamingFilter::decodeTrailers(Envoy::Http::HeaderMap &) {
  if (!isActive()) {
    return Envoy::Http::FilterTrailersStatus::Continue;
  }

  ENVOY_LOG(debug, "decodeTrailers called");

  relayToNatsStreaming();
  return Envoy::Http::FilterTrailersStatus::StopIteration;
}

void NatsStreamingFilter::setDecoderFilterCallbacks(
    Envoy::Http::StreamDecoderFilterCallbacks &callbacks) {
  callbacks_ = &callbacks;
}

void NatsStreamingFilter::onResponse() {
  // TODO(talnordan): This is a dummy implementation that aborts the request.
  callbacks_->requestInfo().setResponseFlag(
      RequestInfo::ResponseFlag::FaultInjected);
  Http::Utility::sendLocalReply(*callbacks_, stream_destroyed_,
                                static_cast<Http::Code>(500),
                                "nats streaming filter abort");
}

void NatsStreamingFilter::retrieveSubject() {
  const Envoy::Router::RouteEntry *routeEntry =
      SoloFilterUtility::resolveRouteEntry(callbacks_);
  Upstream::ClusterInfoConstSharedPtr info =
      FilterUtility::resolveClusterInfo(callbacks_, cm_);
  if (routeEntry == nullptr || info == nullptr) {
    return;
  }

  optional_subject_ = subject_retriever_->getSubject(*routeEntry, *info);
}

void NatsStreamingFilter::relayToNatsStreaming() {
  ASSERT(optional_subject_.valid());

  const std::string *cluster_name =
      SoloFilterUtility::resolveClusterName(callbacks_);
  if (!cluster_name) {
    // TODO(talnordan): Consider changing the return type to `bool` and
    // returning `false`.
    return;
  }

  const std::string &subject = optional_subject_.value();
  const Buffer::Instance *payload = callbacks_->decodingBuffer();

  // TODO(talnordan): Keep the return value of `makeRequest()`.
  publisher_->makeRequest(*cluster_name, subject, payload, *this);
}

} // namespace Http
} // namespace Envoy
