#pragma once

#include <string>

#include "envoy/router/router.h"
#include "envoy/upstream/upstream.h"

#include "common/protobuf/protobuf.h"

#include "subject_retriever.h"

namespace Envoy {
namespace Http {

/**
 * TODO (talnordan):
 * template<typename... _Elements>
 * class MetadataRetriever {
 *   Optional<std::tuple<typename __decay_and_strip<_Elements>::__type...>>
 *   get(const RouteEntry &routeEntry, const ClusterInfo &info);
 * };
 */
class MetadataSubjectRetriever : public SubjectRetriever {
public:
  MetadataSubjectRetriever();

  Optional<Subject> getSubject(const MetadataAccessor &metadataccessor);

private:
  // TODO(talnordan): Move to `envoy-common`.
  static inline Optional<const std::string *>
  nonEmptyStringValue(const ProtobufWkt::Struct &spec, const std::string &key);

  // TODO(talnordan): Move to `envoy-common`.
  static inline Optional<const Protobuf::Value *>
  value(const Protobuf::Struct &spec, const std::string &key);
};

} // namespace Http
} // namespace Envoy
