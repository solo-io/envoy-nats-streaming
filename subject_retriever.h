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

using Subject = std::string;

class SubjectRetriever {
public:
  virtual ~SubjectRetriever() {}
  virtual Optional<Subject> getSubject(const RouteEntry &routeEntry,
                                       const ClusterInfo &info) PURE;
};

typedef std::shared_ptr<SubjectRetriever> SubjectRetrieverSharedPtr;

} // namespace Http
} // namespace Envoy
