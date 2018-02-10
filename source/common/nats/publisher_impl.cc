#include "common/nats/publisher_impl.h"

#include "common/common/macros.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

InstanceImpl::InstanceImpl(
    Tcp::ConnPool::ManagerPtr<std::string> conn_pool_manager)
    : conn_pool_manager_(std::move(conn_pool_manager)) {}

PublishRequestPtr InstanceImpl::makeRequest(const std::string &cluster_name,
                                            const std::string &subject,
                                            const Buffer::Instance *payload,
                                            PublishCallbacks &callbacks) {
  UNREFERENCED_PARAMETER(cluster_name);
  UNREFERENCED_PARAMETER(subject);
  UNREFERENCED_PARAMETER(payload);
  UNREFERENCED_PARAMETER(callbacks);

  // TODO(talnordan)
  callbacks.onResponse();
  return nullptr;
}

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
