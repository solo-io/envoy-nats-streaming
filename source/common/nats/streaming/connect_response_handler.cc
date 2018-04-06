#include "common/nats/streaming/connect_response_handler.h"

#include <string>

#include "common/nats/streaming/message_utility.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

void ConnectResponseHandler::onMessage(absl::optional<std::string> &reply_to,
                                       const std::string &payload,
                                       Callbacks &callbacks) {
  if (reply_to.has_value()) {
    callbacks.onFailure(
        "incoming ConnectResponse with non-empty reply subject");
    return;
  }

  if (payload.empty()) {
    callbacks.onFailure("incoming ConnectResponse without payload");
    return;
  }

  callbacks.onConnected(MessageUtility::getPubPrefix(payload));
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
