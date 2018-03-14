#include "common/nats/message_builder.h"

#include <sstream>

namespace Envoy {
namespace Nats {

Message MessageBuilder::createConnectMessage() {
  return Message(
      R"(CONNECT {"verbose":false,"pedantic":false,"tls_required":false,"name":"","lang":"cpp","version":"1.2.2","protocol":1})");
}

Message MessageBuilder::createPubMessage(const std::string &subject) {
  std::stringstream ss;
  ss << "PUB " << subject << " 0\r\n";
  return Message(ss.str());
}

Message MessageBuilder::createPubMessage(const std::string &subject,
                                         const std::string &reply_to,
                                         const std::string &payload) {
  // TODO(talnordan): Consider introducing a more explicit way to construct and
  // encode messages consisting of two lines.
  std::stringstream ss;
  ss << "PUB " << subject << " " << reply_to << " " << payload.length()
     << "\r\n"
     << payload;
  return Message(ss.str());
}

Message MessageBuilder::createSubMessage(const std::string &subject,
                                         uint64_t sid) {
  std::stringstream ss;
  ss << "SUB " << subject << " " << sid;
  return Message(ss.str());
}

Message MessageBuilder::createPongMessage() { return Message("PONG"); }

} // namespace Nats
} // namespace Envoy
