#include "common/nats/publisher_impl.h"

#include "common/common/macros.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

InstanceImpl::InstanceImpl(Tcp::ConnPool::InstancePtr<Message> &&conn_pool_)
    : conn_pool_(std::move(conn_pool_)) {}

PublishRequestPtr InstanceImpl::makeRequest(const std::string &cluster_name,
                                            const std::string &subject,
                                            const Buffer::Instance *payload,
                                            PublishCallbacks &callbacks) {
  UNREFERENCED_PARAMETER(cluster_name);
  UNREFERENCED_PARAMETER(subject);
  UNREFERENCED_PARAMETER(payload);

  callbacks_ = &callbacks;
  conn_pool_->setPoolCallbacks(*this);

  // Send a NATS CONNECT message.
  const std::string hash_key;
  const Message request = message_builder_.createNatsConnectRequest();
  conn_pool_->makeRequest(hash_key, request);

  // TODO(talnordan)
  return nullptr;
}

void InstanceImpl::onResponse(Nats::MessagePtr &&value) {
  // TODO(talnordan)
  UNREFERENCED_PARAMETER(value);
  callbacks_->onResponse();
}

void InstanceImpl::onClose() {
  // TODO(talnordan)
}

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
