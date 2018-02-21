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
  return metadataccessor.getFunctionName();
}

} // namespace Http
} // namespace Envoy
