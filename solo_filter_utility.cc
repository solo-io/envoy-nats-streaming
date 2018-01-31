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

} // namespace Http
} // namespace Envoy
