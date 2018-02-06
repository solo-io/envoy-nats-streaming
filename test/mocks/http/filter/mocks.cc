#include "mocks.h"

using testing::Invoke;
using testing::_;

namespace Envoy {
namespace Http {

MockTopicRetriever::MockTopicRetriever() {
  ON_CALL(*this, getTopic(_, _))
      .WillByDefault(Invoke([this](const RouteEntry &, const ClusterInfo &)
                                -> Optional<Topic> { return topic_; }));
}

MockTopicRetriever::~MockTopicRetriever() {}

} // namespace Http
} // namespace Envoy
