#include "common/nats/publisher_impl.h"

#include "common/common/macros.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

InstanceImpl::InstanceImpl() {}

PublishRequestPtr InstanceImpl::makeRequest(PublishCallbacks &callbacks) {
  UNREFERENCED_PARAMETER(callbacks);

  // TODO(talnordan)
  return nullptr;
}

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
