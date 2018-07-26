#include "mocks.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs) {
  return lhs.asString() == rhs.asString();
}

namespace ConnPoolNats {

MockInstance::MockInstance() {}
MockInstance::~MockInstance() {}

} // namespace ConnPoolNats

} // namespace Nats
} // namespace Envoy
