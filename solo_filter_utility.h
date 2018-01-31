#pragma once

#include "envoy/http/filter.h"
#include "envoy/upstream/cluster_manager.h"

namespace Envoy {
namespace Http {

/**
 * General utilities for HTTP filters.
 *
 * TODO(talnordan): Merge this class into
 * envoy/source/common.http/filter_utility.h.
 */
class SoloFilterUtility {
public:
  // TODO(talnordan): The envoyproxy/envoy convention seems to be not to
  // explicitly delete constructors.
  SoloFilterUtility() = delete;
  SoloFilterUtility(const SoloFilterUtility &) = delete;

  /**
   * Resolve the route entry.
   * @param decoder_callbacks supplies the decoder callback of filter.
   * @return the route entry selected for this request. Note: this will be
   * nullptr if no route was selected.
   */
  static const Router::RouteEntry *
  resolveRouteEntry(Http::StreamDecoderFilterCallbacks *decoder_callbacks);
};

} // namespace Http
} // namespace Envoy
