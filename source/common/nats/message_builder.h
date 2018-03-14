#pragma once

#include <string>

#include "envoy/nats/codec.h"

namespace Envoy {
namespace Nats {

class MessageBuilder {
public:
  static Message createConnectMessage();
  static Message createPubMessage(const std::string &subject);
  static Message createPubMessage(const std::string &subject,
                                  const std::string &reply_to,
                                  const std::string &payload);
  static Message createSubMessage(const std::string &subject, uint64_t sid);
  static Message createPongMessage();
};

} // namespace Nats
} // namespace Envoy
