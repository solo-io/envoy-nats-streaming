#pragma once

#include <string>

#include "envoy/router/router.h"
#include "envoy/upstream/upstream.h"

#include "common/protobuf/protobuf.h"

#include "subject_retriever.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

/**
 * TODO (talnordan):
 * template<typename... _Elements>
 * class MetadataRetriever {
 *   absl::optional<std::tuple<typename
 * __decay_and_strip<_Elements>::__type...>> get(const RouteEntry &routeEntry,
 * const ClusterInfo &info);
 * };
 */
class MetadataSubjectRetriever : public SubjectRetriever {
public:
  MetadataSubjectRetriever();

  absl::optional<Subject>
  getSubject(const Http::MetadataAccessor &metadataccessor);

private:
  static absl::optional<const std::string *>
  nonEmptyStringValue(const ProtobufWkt::Struct &spec, const std::string &key);
};

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
