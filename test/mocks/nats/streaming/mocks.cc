#include "mocks.h"

#include "common/common/macros.h"

using testing::Invoke;
using testing::_;

namespace Envoy {
namespace Nats {
namespace Streaming {

MockPublishCallbacks::MockPublishCallbacks() {}
MockPublishCallbacks::~MockPublishCallbacks() {}

MockClient::MockClient() {
  ON_CALL(*this, makeRequest(_, _, _, _, _))
      .WillByDefault(Invoke(
          [this](const std::string &subject, const std::string &cluster_id,
                 const std::string &discover_prefix, Buffer::Instance &payload,
                 PublishCallbacks &callbacks) -> PublishRequestPtr {
            UNREFERENCED_PARAMETER(subject);
            UNREFERENCED_PARAMETER(cluster_id);
            UNREFERENCED_PARAMETER(discover_prefix);

            last_payload_.drain(last_payload_.length());
            last_payload_.move(payload);

            callbacks.onResponse();
            return nullptr;
          }));
}

MockClient::~MockClient() {}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
