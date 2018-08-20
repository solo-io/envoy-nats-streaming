#include "extensions/filters/http/nats/streaming/metadata_subject_retriever.h"

#include "common/common/macros.h"
#include "common/config/metadata.h"
#include "common/config/nats_streaming_well_known_names.h"
#include "common/config/solo_metadata.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

using Config::SoloMetadata;

MetadataSubjectRetriever::MetadataSubjectRetriever() {}

absl::optional<Subject> MetadataSubjectRetriever::getSubject(
    const Http::MetadataAccessor &metadataccessor) {
  auto maybe_subject = metadataccessor.getFunctionName();
  if (!maybe_subject.has_value()) {
    return {};
  }

  absl::optional<const ProtobufWkt::Struct *> maybe_cluster_meta =
      metadataccessor.getClusterMetadata();
  if (!maybe_cluster_meta.has_value()) {
    return {};
  }
  const ProtobufWkt::Struct *cluster_meta = maybe_cluster_meta.value();

  auto maybe_discover_prefix = SoloMetadata::nonEmptyStringValue(
      *cluster_meta, Config::MetadataNatsStreamingKeys::get().DISCOVER_PREFIX);
  if (!maybe_discover_prefix.has_value()) {
    return {};
  }

  auto maybe_cluster_id = SoloMetadata::nonEmptyStringValue(
      *cluster_meta, Config::MetadataNatsStreamingKeys::get().CLUSTER_ID);
  if (!maybe_cluster_id.has_value()) {
    return {};
  }

  Subject subject{maybe_subject.value(), maybe_cluster_id.value(),
                  maybe_discover_prefix.value()};

  return subject;
}

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
