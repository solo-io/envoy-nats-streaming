#pragma once

#include <map>
#include <string>

#include "envoy/common/optional.h"
#include "envoy/nats/streaming/client.h"
#include "envoy/nats/streaming/inbox_handler.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class PubAckHandler {
public:
  static void onMessage(const Optional<std::string> &reply_to,
                        const std::string &payload,
                        InboxCallbacks &inbox_callbacks,
                        PublishCallbacks &publish_callbacks);

  static void onMessage(
      const std::string &inbox, const Optional<std::string> &reply_to,
      const std::string &payload, InboxCallbacks &inbox_callbacks,
      std::map<std::string, PublishCallbacks *> &publish_callbacks_per_inbox);
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
