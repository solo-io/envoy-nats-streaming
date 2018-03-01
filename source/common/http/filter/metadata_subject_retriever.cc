#include "common/http/filter/metadata_subject_retriever.h"

#include "common/common/macros.h"
#include "common/config/metadata.h"
#include "common/config/nats_streaming_well_known_names.h"

namespace Envoy {
namespace Http {

using Config::Metadata;

MetadataSubjectRetriever::MetadataSubjectRetriever() {}

Optional<Subject>
MetadataSubjectRetriever::getSubject(const MetadataAccessor &metadataccessor) {
  auto maybe_subject = metadataccessor.getFunctionName();
  if (!maybe_subject.valid()) {
    return {};
  }

  Optional<const ProtobufWkt::Struct *> maybe_cluster_meta =
      metadataccessor.getClusterMetadata();
  if (!maybe_cluster_meta.valid()) {
    return {};
  }
  const ProtobufWkt::Struct *cluster_meta = maybe_cluster_meta.value();

  auto maybe_discover_prefix = nonEmptyStringValue(
      *cluster_meta, Config::MetadataNatsStreamingKeys::get().DISCOVER_PREFIX);
  if (!maybe_discover_prefix.valid()) {
    return {};
  }

  auto maybe_cluster_id = nonEmptyStringValue(
      *cluster_meta, Config::MetadataNatsStreamingKeys::get().CLUSTER_ID);
  if (!maybe_cluster_id.valid()) {
    return {};
  }

  Subject subject{maybe_subject.value(), maybe_cluster_id.value(),
                  maybe_discover_prefix.value()};

  return subject;
}

/**
 * TODO(yuval-k): Consider moving the this logic to `Metadata`:
 * envoy/source/common/config/metadata.cc
 */
Optional<const std::string *>
MetadataSubjectRetriever::nonEmptyStringValue(const ProtobufWkt::Struct &spec,
                                              const std::string &key) {
  const auto &fields = spec.fields();
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
