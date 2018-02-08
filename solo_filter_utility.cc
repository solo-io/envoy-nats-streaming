#include "solo_filter_utility.h"

namespace Envoy {
namespace Http {

const Router::RouteEntry *SoloFilterUtility::resolveRouteEntry(
    Http::StreamDecoderFilterCallbacks *decoder_callbacks) {
  Router::RouteConstSharedPtr route = decoder_callbacks->route();
  if (!route) {
    return nullptr;
  }

  return route->routeEntry();
}

const std::string *SoloFilterUtility::resolveClusterName(
    Http::StreamDecoderFilterCallbacks *decoder_callbacks) {
  const Router::RouteEntry *route_entry = resolveRouteEntry(decoder_callbacks);
  if (!route_entry) {
    return nullptr;
  }

  return &route_entry->clusterName();
}

} // namespace Http
} // namespace Envoy
