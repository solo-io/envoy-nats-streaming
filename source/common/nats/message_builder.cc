#include "common/nats/message_builder.h"

namespace Envoy {
namespace Nats {

Envoy::Nats::Message MessageBuilder::createConnectMessage() const {
  return Message(
      R"(CONNECT {"verbose":false,"pedantic":false,"tls_required":false,"name":"","lang":"cpp","version":"1.2.2","protocol":1})");
}

} // namespace Nats
} // namespace Envoy
