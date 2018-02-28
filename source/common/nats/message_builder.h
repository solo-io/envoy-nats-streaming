#pragma once

#include <string>

#include "envoy/nats/codec.h"

namespace Envoy {
namespace Nats {

class MessageBuilder {
public:
  Envoy::Nats::Message createConnectMessage() const;
  Envoy::Nats::Message createPubMessage(const std::string &subject) const;
  Envoy::Nats::Message createPubMessage(const std::string &subject,
                                        const std::string &reply_to,
                                        const std::string &payload) const;
  Envoy::Nats::Message createSubMessage(const std::string &subject,
                                        const std::string &sid) const;
};

} // namespace Nats
} // namespace Envoy
