#include "metadata_topic_retriever.h"

#include "common/common/macros.h"
#include "common/config/metadata.h"

#include "nats_streaming_filter.pb.h"

namespace Envoy {
namespace Http {

using Config::Metadata;

MetadataTopicRetriever::MetadataTopicRetriever(const std::string &filter_key,
                                               const std::string &topic_key)
    : filter_key_(filter_key), topic_key_(topic_key) {}

Optional<Topic> MetadataTopicRetriever::getTopic(const RouteEntry &routeEntry,
                                                 const ClusterInfo &info) {
  auto route_metadata_fields = filterMetadataFields(routeEntry, filter_key_);

  auto cluster_metadata_fields = filterMetadataFields(info, filter_key_);

  if (!route_metadata_fields.valid() || !cluster_metadata_fields.valid()) {
    return {};
  }

  return getTopic(*route_metadata_fields.value(),
                  *cluster_metadata_fields.value());
}

Optional<Topic>
MetadataTopicRetriever::getTopic(const FieldMap &route_metadata_fields,
                                 const FieldMap &cluster_metadata_fields) {
  UNREFERENCED_PARAMETER(cluster_metadata_fields);
  auto topic = nonEmptyStringValue(route_metadata_fields, topic_key_);
  return (topic.valid()) ? Optional<Topic>(*topic.value()) : Optional<Topic>{};
}

/**
 * TODO(talnordan): Consider moving the `Struct` extraction logic to `Metadata`:
 * envoy/source/common/config/metadata.cc
 */
Optional<const MetadataTopicRetriever::FieldMap *>
MetadataTopicRetriever::filterMetadataFields(
    const envoy::api::v2::Metadata &metadata, const std::string &filter) {
  const auto filter_it = metadata.filter_metadata().find(filter);
  if (filter_it == metadata.filter_metadata().end()) {
    return {};
  }

  const auto &filter_metadata_struct = filter_it->second;
  const auto &filter_metadata_fields = filter_metadata_struct.fields();
  return Optional<const FieldMap *>(&filter_metadata_fields);
}

/**
 * TODO(talnordan): Consider moving the this logic to `Metadata`:
 * envoy/source/common/config/metadata.cc
 */
Optional<const std::string *>
MetadataTopicRetriever::nonEmptyStringValue(const FieldMap &fields,
                                            const std::string &key) {
  const auto fields_it = fields.find(key);
  if (fields_it == fields.end()) {
    return {};
  }

  const auto &value = fields_it->second;
  if (value.kind_case() != ProtobufWkt::Value::kStringValue) {
    return {};
  }

  const auto &string_value = value.string_value();
  if (string_value.empty()) {
    return {};
  }

  return Optional<const std::string *>(&string_value);
}

} // namespace Http
} // namespace Envoy
