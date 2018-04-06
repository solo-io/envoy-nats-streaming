#pragma once

#include <string>
#include <utility>

#include "absl/types/optional.h"
#include "protocol.pb.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class MessageUtility {
public:
  static std::string
  createConnectRequestMessage(const std::string &client_id,
                              const std::string &heartbeat_inbox);

  static std::string createConnectResponseMessage(
      const std::string &pub_prefix, const std::string &sub_requests,
      const std::string &unsub_requests, const std::string &close_requests);

  static std::string createPubMsgMessage(const std::string &client_id,
                                         const std::string &guid,
                                         const std::string &subject,
                                         const std::string &data);

  static std::string createPubAckMessage(const std::string &guid,
                                         const std::string &error);

  static absl::optional<pb::PubAck>
  parsePubAckMessage(const std::string &pub_ack_message);

  static std::string getPubPrefix(const std::string &connect_response_message);

private:
  template <typename T> static std::string serializeToString(T &&message) {
    std::string message_str;
    message.SerializeToString(&message_str);

    return message_str;
  }
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
