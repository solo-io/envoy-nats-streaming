#include "common/nats/streaming/heartbeat_handler.h"

#include <string>

#include "common/nats/message_builder.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

void HeartbeatHandler::onMessage(Optional<std::string> &reply_to,
                                 const std::string &payload,
                                 Callbacks &callbacks) {
  if (!reply_to.valid()) {
    callbacks.onFailure("incoming heartbeat without reply subject");
    return;
  }

  if (!payload.empty()) {
    callbacks.onFailure("incoming heartbeat with non-empty payload");
    return;
  }

  callbacks.send(MessageBuilder::createPubMessage(reply_to.value()));
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
