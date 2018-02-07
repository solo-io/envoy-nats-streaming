#include "mocks.h"

using testing::Invoke;
using testing::_;

namespace Envoy {
namespace Http {

MockSubjectRetriever::MockSubjectRetriever() {
  ON_CALL(*this, getSubject(_, _))
      .WillByDefault(Invoke([this](const RouteEntry &, const ClusterInfo &)
                                -> Optional<Subject> { return subject_; }));
}

MockSubjectRetriever::~MockSubjectRetriever() {}

} // namespace Http
} // namespace Envoy
