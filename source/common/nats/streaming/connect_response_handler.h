#pragma once

#include <string>

#include "envoy/common/pure.h"
#include "envoy/nats/streaming/inbox_handler.h"

#include "absl/types/optional.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class ConnectResponseHandler {
public:
  class Callbacks : public virtual InboxCallbacks {
  public:
    virtual ~Callbacks() {}
    virtual void onConnected(const std::string &pub_prefix) PURE;
  };

  static void onMessage(absl::optional<std::string> &reply_to,
                        const std::string &payload, Callbacks &callbacks);
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
