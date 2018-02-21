#pragma once

#include <memory>

#include "envoy/common/optional.h"
#include "envoy/common/pure.h"
#include "envoy/http/metadata_accessor.h"

namespace Envoy {
namespace Http {

using Subject = const std::string *;

// TODO(talnordan): Make generic and move to `envoy-common`.
class SubjectRetriever {
public:
  virtual ~SubjectRetriever() {}
  virtual Optional<Subject>
  getSubject(const MetadataAccessor &metadataccessor) PURE;
};

typedef std::shared_ptr<SubjectRetriever> SubjectRetrieverSharedPtr;

} // namespace Http
} // namespace Envoy
