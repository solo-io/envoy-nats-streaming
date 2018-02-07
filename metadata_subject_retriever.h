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

  using FieldMap = Protobuf::Map<std::string, Protobuf::Value>;

public:
  MetadataSubjectRetriever(const std::string &filter_key,
                           const std::string &subject_key);

  Optional<Subject> getSubject(const RouteEntry &routeEntry,
                               const ClusterInfo &info);
  Optional<Subject> getSubject(const FieldMap &route_metadata_fields,
                               const FieldMap &cluster_metadata_fields);

private:
  /**
   * Resolve the filter metadata fields.
   * @param filter_name an entity that has metadata.
   * @param filter_name the reverse DNS filter name.
   */
  template <typename T>
  static inline Optional<const FieldMap *>
  filterMetadataFields(const T &entity, const std::string &filter_name);

  static inline Optional<const FieldMap *>
  filterMetadataFields(const envoy::api::v2::Metadata &metadata,
                       const std::string &filter_name);

  static inline Optional<const std::string *>
  nonEmptyStringValue(const FieldMap &fields, const std::string &key);

  const std::string &filter_key_;
  const std::string &subject_key_;
};

template <typename T>
Optional<const MetadataSubjectRetriever::FieldMap *>
MetadataSubjectRetriever::filterMetadataFields(const T &entity,
                                               const std::string &filter_name) {
  return filterMetadataFields(entity.metadata(), filter_name);
}

} // namespace Http
} // namespace Envoy
