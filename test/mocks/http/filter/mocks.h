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

  MOCK_METHOD2(getSubject, Optional<Subject>(const RouteEntry &routeEntry,
                                             const ClusterInfo &info));

  Optional<Subject> subject_;
};

} // namespace Http
} // namespace Envoy
