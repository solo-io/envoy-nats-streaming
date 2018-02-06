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
    TopicRetrieverSharedPtr topicRetriever, ClusterManager &cm)
    : config_(config), topicRetriever_(topicRetriever), cm_(cm) {}

NatsStreamingFilter::~NatsStreamingFilter() {}

void NatsStreamingFilter::onDestroy() { stream_destroyed_ = true; }

Envoy::Http::FilterHeadersStatus
NatsStreamingFilter::decodeHeaders(Envoy::Http::HeaderMap &headers,
                                   bool end_stream) {
  UNREFERENCED_PARAMETER(headers);

  retrieveTopic();

  if (!isActive()) {
    return Envoy::Http::FilterHeadersStatus::Continue;
  }

  ENVOY_LOG(debug, "decodeHeaders called end = {}", end_stream);

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
  if (isActive()) {
    relayToNatsStreaming();
  }

  return Envoy::Http::FilterTrailersStatus::Continue;
}

void NatsStreamingFilter::setDecoderFilterCallbacks(
    Envoy::Http::StreamDecoderFilterCallbacks &callbacks) {
  callbacks_ = &callbacks;
}

void NatsStreamingFilter::retrieveTopic() {
  const Envoy::Router::RouteEntry *routeEntry =
      SoloFilterUtility::resolveRouteEntry(callbacks_);
  Upstream::ClusterInfoConstSharedPtr info =
      FilterUtility::resolveClusterInfo(callbacks_, cm_);
  if (routeEntry == nullptr || info == nullptr) {
    return;
  }

  optionalTopic_ = topicRetriever_->getTopic(*routeEntry, *info);
}

void NatsStreamingFilter::relayToNatsStreaming() {
  // TODO(talnordan): This is a dummy implementation that aborts the request.

  callbacks_->requestInfo().setResponseFlag(
      RequestInfo::ResponseFlag::FaultInjected);
  Http::Utility::sendLocalReply(*callbacks_, stream_destroyed_,
                                static_cast<Http::Code>(500),
                                "nats streaming filter abort");
}

} // namespace Http
} // namespace Envoy
