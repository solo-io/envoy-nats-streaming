#include "mocks.h"

using testing::_;
using testing::Invoke;

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

MockSubjectRetriever::MockSubjectRetriever() {
  ON_CALL(*this, getSubject(_))
      .WillByDefault(Invoke(
          [this](const Http::MetadataAccessor &) -> absl::optional<Subject> {
            return subject_;
          }));
}

MockSubjectRetriever::~MockSubjectRetriever() {}

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
