#include "common/nats/streaming/pub_ack_handler.h"

#include "common/nats/streaming/message_utility.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

void PubAckHandler::onMessage(Optional<std::string> &reply_to,
                              const std::string &payload,
                              InboxCallbacks &inbox_callbacks,
                              PublishCallbacks &publish_callbacks) {
  if (reply_to.valid()) {
    inbox_callbacks.onFailure("incoming PubAck with non-empty reply subject");
    return;
  }

  if (payload.empty()) {
    inbox_callbacks.onFailure("incoming PubAck without payload");
    return;
  }

  auto &&pub_ack = MessageUtility::parsePubAckMessage(payload);

  if (pub_ack.error().empty()) {
    publish_callbacks.onResponse();
  } else {
    publish_callbacks.onFailure();
  }
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
