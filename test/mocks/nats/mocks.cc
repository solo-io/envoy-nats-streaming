#include "mocks.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs) {
  return lhs.asString() == rhs.asString();
}

namespace ConnPool {

MockInstance::MockInstance() {}
MockInstance::~MockInstance() {}

} // namespace ConnPool

} // namespace Nats
} // namespace Envoy
