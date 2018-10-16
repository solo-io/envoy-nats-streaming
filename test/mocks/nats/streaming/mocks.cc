#include "mocks.h"

#include "common/common/macros.h"

using testing::_;
using testing::Invoke;

namespace Envoy {
namespace Nats {
namespace Streaming {

MockPublishCallbacks::MockPublishCallbacks() {}
MockPublishCallbacks::~MockPublishCallbacks() {}

MockClient::MockClient() {
  ON_CALL(*this, makeRequest_(_, _, _, _, _))
      .WillByDefault(Invoke(
          [this](const std::string &subject, const std::string &cluster_id,
                 const std::string &discover_prefix, const std::string &payload,
                 PublishCallbacks &callbacks) -> PublishRequestPtr {
            UNREFERENCED_PARAMETER(subject);
            UNREFERENCED_PARAMETER(cluster_id);
            UNREFERENCED_PARAMETER(discover_prefix);

            last_payload_ = payload;
            callbacks.onResponse();
            return nullptr;
          }));
}

MockClient::~MockClient() {}

PublishRequestPtr MockClient::makeRequest(const std::string &subject,
                                          const std::string &cluster_id,
                                          const std::string &discover_prefix,
                                          std::string &&payload,
                                          PublishCallbacks &callbacks) {
  return makeRequest_(subject, cluster_id, discover_prefix, payload, callbacks);
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
