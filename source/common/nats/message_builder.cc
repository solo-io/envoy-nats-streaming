#include "common/nats/message_builder.h"

#include <sstream>

namespace Envoy {
namespace Nats {

Envoy::Nats::Message MessageBuilder::createConnectMessage() const {
  return Message(
      R"(CONNECT {"verbose":false,"pedantic":false,"tls_required":false,"name":"","lang":"cpp","version":"1.2.2","protocol":1})");
}

Envoy::Nats::Message
MessageBuilder::createPubMessage(const std::string &subject) const {
  std::stringstream ss;
  ss << "PUB " << subject << " 0\r\n";
  return Message(ss.str());
}

Envoy::Nats::Message
MessageBuilder::createSubMessage(const std::string &subject,
                                 const std::string &sid) const {
  std::stringstream ss;
  ss << "SUB " << subject << " " << sid;
  return Message(ss.str());
}

} // namespace Nats
} // namespace Envoy
