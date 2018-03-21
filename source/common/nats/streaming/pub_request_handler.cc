#include "common/nats/streaming/pub_request_handler.h"

#include "common/nats/streaming/message_utility.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

void PubRequestHandler::onMessage(const Optional<std::string> &reply_to,
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

void PubRequestHandler::onMessage(
    const std::string &inbox, const Optional<std::string> &reply_to,
    const std::string &payload, InboxCallbacks &inbox_callbacks,
    std::map<std::string, PublishCallbacks *> &publish_callbacks_per_inbox) {
  // Find the inbox in the map.
  auto it = publish_callbacks_per_inbox.find(inbox);

  // Gracefully ignore a missing inbox.
  if (it == publish_callbacks_per_inbox.end()) {
    // TODO(talnordan): consider logging the message and/or updating stats.
    return;
  }

  // Handle the message using the publish callbacks associated with the inbox.
  PublishCallbacks &publish_callbacks = *it->second;
  onMessage(reply_to, payload, inbox_callbacks, publish_callbacks);

  // Remove the inbox from the map.
  publish_callbacks_per_inbox.erase(it);
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
