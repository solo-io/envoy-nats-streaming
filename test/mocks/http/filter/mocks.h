#pragma once

#include <string>

#include "common/http/filter/subject_retriever.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Http {

class MockSubjectRetriever : public SubjectRetriever {
public:
  MockSubjectRetriever();
  ~MockSubjectRetriever();

  MOCK_METHOD1(getSubject,
               Optional<Subject>(const MetadataAccessor &metadataccessor));

  Optional<Subject> subject_;
};

} // namespace Http
} // namespace Envoy
