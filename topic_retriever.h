#pragma once

#include <memory>

#include "envoy/common/optional.h"
#include "envoy/common/pure.h"
#include "envoy/router/router.h"
#include "envoy/upstream/upstream.h"

namespace Envoy {
namespace Http {

using Router::RouteEntry;
using Upstream::ClusterInfo;

using Topic = std::string;

class TopicRetriever {
public:
  virtual ~TopicRetriever() {}
  virtual Optional<Topic> getTopic(const RouteEntry &routeEntry,
                                   const ClusterInfo &info) PURE;
};

typedef std::shared_ptr<TopicRetriever> TopicRetrieverSharedPtr;

} // namespace Http
} // namespace Envoy
