#pragma once

#include <memory>

#include "envoy/common/pure.h"
#include "envoy/http/metadata_accessor.h"

#include "absl/types/optional.h"

namespace Envoy {
namespace Http {

// TODO(yuval-k): rename this to something more descriptive?
struct Subject {
  const std::string *subject;
  const std::string *cluster_id;
  const std::string *discover_prefix;
};

// TODO(talnordan): Make generic and move to `envoy-common`.
class SubjectRetriever {
public:
  virtual ~SubjectRetriever() {}
  virtual absl::optional<Subject>
  getSubject(const MetadataAccessor &metadataccessor) PURE;
};

typedef std::shared_ptr<SubjectRetriever> SubjectRetrieverSharedPtr;

} // namespace Http
} // namespace Envoy
