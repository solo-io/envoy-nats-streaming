#pragma once

#include "envoy/nats/codec.h"

namespace Envoy {
namespace Nats {

class MessageBuilder {
public:
  Envoy::Nats::Message createConnectMessage() const;
};

} // namespace Nats
} // namespace Envoy
