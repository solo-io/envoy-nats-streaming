#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "envoy/router/router.h"
#include "envoy/upstream/upstream.h"

#include "common/protobuf/protobuf.h"

namespace Envoy {
namespace Http {

using Router::RouteEntry;
using Upstream::ClusterInfo;

using Topic = std::string;

/**
 * TODO (talnordan):
 * template<typename... _Elements>
 * class MetadataRetriever {
 *   Optional<std::tuple<typename __decay_and_strip<_Elements>::__type...>>
 *   get(const RouteEntry &routeEntry, const ClusterInfo &info);
 * };
 */
class MetadataTopicRetriever {

  using FieldMap = Protobuf::Map<std::string, Protobuf::Value>;

public:
  MetadataTopicRetriever(const std::string &filter_key,
                         const std::string &topic_key);

  Optional<Topic> getTopic(const RouteEntry &routeEntry,
                           const ClusterInfo &info);
  Optional<Topic> getTopic(const FieldMap &route_metadata_fields,
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
  const std::string &topic_key_;
};

template <typename T>
Optional<const MetadataTopicRetriever::FieldMap *>
MetadataTopicRetriever::filterMetadataFields(const T &entity,
                                             const std::string &filter_name) {
  return filterMetadataFields(entity.metadata(), filter_name);
}

typedef std::shared_ptr<MetadataTopicRetriever> TopicRetrieverSharedPtr;

} // namespace Http
} // namespace Envoy
