#pragma once

#include <string>

#include "envoy/common/pure.h"
#include "envoy/nats/codec.h"
#include "envoy/nats/streaming/inbox_handler.h"

#include "absl/types/optional.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class HeartbeatHandler {
public:
  class Callbacks : public virtual InboxCallbacks {
  public:
    virtual ~Callbacks() {}
    virtual void send(const Message &message) PURE;
  };

  // TODO(talnordan): For this handler, the payload is always empty. In the
  // genral case, use a NATS streaming message type instead of a raw payload.
  static void onMessage(absl::optional<std::string> &reply_to,
                        const std::string &payload, Callbacks &callbacks);
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
