#pragma once

#include "common/nats/codec_impl.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs);

} // namespace Nats
} // namespace Envoy
