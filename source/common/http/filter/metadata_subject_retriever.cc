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
  Optional<const ProtobufWkt::Struct *> maybe_route_spec =
      metadataccessor.getRouteMetadata();

  if (!maybe_route_spec.valid()) {
    return {};
  }

  const ProtobufWkt::Struct &route_spec = *maybe_route_spec.value();

  auto subject = nonEmptyStringValue(
      route_spec, Config::MetadataNatsStreamingKeys::get().SUBJECT);
  return (subject.valid()) ? Optional<Subject>(*subject.value())
                           : Optional<Subject>{};
}

/**
 * TODO(talnordan): Consider moving the this logic to `Metadata`:
 * envoy/source/common/config/metadata.cc
 */
Optional<const std::string *>
MetadataSubjectRetriever::nonEmptyStringValue(const ProtobufWkt::Struct &spec,
                                              const std::string &key) {

  Optional<const Protobuf::Value *> maybe_value = value(spec, key);
  if (!maybe_value.valid()) {
    return {};
  }
  const auto &value = *maybe_value.value();
  if (value.kind_case() != ProtobufWkt::Value::kStringValue) {
    return {};
  }

  const auto &string_value = value.string_value();
  if (string_value.empty()) {
    return {};
  }

  return Optional<const std::string *>(&string_value);
}

Optional<const Protobuf::Value *>
MetadataSubjectRetriever::value(const Protobuf::Struct &spec,
                                const std::string &key) {
  const auto &fields = spec.fields();
  const auto fields_it = fields.find(key);
  if (fields_it == fields.end()) {
    return {};
  }

  const auto &value = fields_it->second;
  return &value;
}

} // namespace Http
} // namespace Envoy
