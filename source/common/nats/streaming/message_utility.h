#pragma once

#include <string>

namespace Envoy {
namespace Nats {
namespace Streaming {

class MessageUtility {
public:
  std::string
  createConnectRequestMessage(const std::string &client_id,
                              const std::string &heartbeat_inbox) const;

  std::string
  createConnectResponseMessage(const std::string &pub_prefix,
                               const std::string &sub_requests,
                               const std::string &unsub_requests,
                               const std::string &close_requests) const;

private:
  template <typename T> std::string serializeToString(T &&message) const {
    std::string message_str;
    message.SerializeToString(&message_str);

    return message_str;
  }
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
