#include "mocks.h"

using testing::Invoke;
using testing::_;

namespace Envoy {
namespace Http {

MockSubjectRetriever::MockSubjectRetriever() {
  ON_CALL(*this, getSubject(_))
      .WillByDefault(
          Invoke([this](const MetadataAccessor &) -> absl::optional<Subject> {
            return subject_;
          }));
}

MockSubjectRetriever::~MockSubjectRetriever() {}

} // namespace Http
} // namespace Envoy
