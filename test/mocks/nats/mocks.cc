#include "mocks.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs) {
  return lhs.asString() == rhs.asString();
}

} // namespace Nats
} // namespace Envoy
