#pragma once

#include <string>

#include "envoy/common/optional.h"
#include "envoy/nats/streaming/client.h"
#include "envoy/nats/streaming/inbox_handler.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class PubAckHandler {
public:
  static void onMessage(Optional<std::string> &reply_to,
                        const std::string &payload,
                        InboxCallbacks &inbox_callbacks,
                        PublishCallbacks &publish_callbacks);
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
