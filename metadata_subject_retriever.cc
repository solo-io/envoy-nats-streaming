#include "metadata_subject_retriever.h"

#include "common/common/macros.h"
#include "common/config/metadata.h"

#include "nats_streaming_filter.pb.h"

namespace Envoy {
namespace Http {

using Config::Metadata;

MetadataSubjectRetriever::MetadataSubjectRetriever(
    const std::string &filter_key, const std::string &subject_key)
    : filter_key_(filter_key), subject_key_(subject_key) {}

Optional<Subject>
MetadataSubjectRetriever::getSubject(const RouteEntry &routeEntry,
                                     const ClusterInfo &info) {
  auto route_metadata_fields = filterMetadataFields(routeEntry, filter_key_);

  auto cluster_metadata_fields = filterMetadataFields(info, filter_key_);

  if (!route_metadata_fields.valid() || !cluster_metadata_fields.valid()) {
    return {};
  }

  return getSubject(*route_metadata_fields.value(),
                    *cluster_metadata_fields.value());
}

Optional<Subject>
MetadataSubjectRetriever::getSubject(const FieldMap &route_metadata_fields,
                                     const FieldMap &cluster_metadata_fields) {
  UNREFERENCED_PARAMETER(cluster_metadata_fields);
  auto subject = nonEmptyStringValue(route_metadata_fields, subject_key_);
  return (subject.valid()) ? Optional<Subject>(*subject.value())
                           : Optional<Subject>{};
}

/**
 * TODO(talnordan): Consider moving the `Struct` extraction logic to `Metadata`:
 * envoy/source/common/config/metadata.cc
 */
Optional<const MetadataSubjectRetriever::FieldMap *>
MetadataSubjectRetriever::filterMetadataFields(
    const envoy::api::v2::core::Metadata &metadata, const std::string &filter) {
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
MetadataSubjectRetriever::nonEmptyStringValue(const FieldMap &fields,
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
