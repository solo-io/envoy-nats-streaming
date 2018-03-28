#include "common/http/filter/metadata_subject_retriever.h"

#include "common/common/macros.h"
#include "common/config/metadata.h"
#include "common/config/nats_streaming_well_known_names.h"
#include "common/config/solo_metadata.h"

namespace Envoy {
namespace Http {

using Config::SoloMetadata;

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

  auto maybe_discover_prefix = SoloMetadata::nonEmptyStringValue(
      *cluster_meta, Config::MetadataNatsStreamingKeys::get().DISCOVER_PREFIX);
  if (!maybe_discover_prefix.valid()) {
    return {};
  }

  auto maybe_cluster_id = SoloMetadata::nonEmptyStringValue(
      *cluster_meta, Config::MetadataNatsStreamingKeys::get().CLUSTER_ID);
  if (!maybe_cluster_id.valid()) {
    return {};
  }

  Subject subject{maybe_subject.value(), maybe_cluster_id.value(),
                  maybe_discover_prefix.value()};

  return subject;
}

} // namespace Http
} // namespace Envoy
