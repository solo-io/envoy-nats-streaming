#pragma once

#include <string>

namespace Envoy {
namespace Nats {
namespace Streaming {

class MessageBuilder {
public:
  std::string
  createConnectRequestMessage(const std::string &client_id,
                              const std::string &heartbeat_inbox) const;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
