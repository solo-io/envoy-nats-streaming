#pragma once

#include <string>

#include "extensions/filters/http/nats/streaming/subject_retriever.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

class MockSubjectRetriever : public SubjectRetriever {
public:
  MockSubjectRetriever();
  ~MockSubjectRetriever();

  MOCK_METHOD1(getSubject, absl::optional<Subject>(
                               const Http::MetadataAccessor &metadataccessor));

  absl::optional<Subject> subject_;
};

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
