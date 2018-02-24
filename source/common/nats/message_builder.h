#pragma once

#include "envoy/nats/codec.h"

namespace Envoy {
namespace Nats {

class MessageBuilder {
public:
  Envoy::Nats::Message createNatsConnectRequest() const;
};

} // namespace Nats
} // namespace Envoy
