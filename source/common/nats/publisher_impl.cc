#include "common/nats/publisher_impl.h"

#include "common/common/assert.h"
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
  UNREFERENCED_PARAMETER(payload);

  subject_.value(subject);
  callbacks_.value(&callbacks);
  conn_pool_->setPoolCallbacks(*this);

  // Send a NATS CONNECT message.
  const std::string hash_key;
  const Message request = message_builder_.createConnectMessage();
  conn_pool_->makeRequest(hash_key, request);

  // TODO(talnordan)
  return nullptr;
}

void InstanceImpl::onResponse(Nats::MessagePtr &&value) {
  switch (state_) {
  case State::Initial:
    onInitialResponse(std::move(value));
    break;
  case State::Published:
    onPublishedResponse(std::move(value));
    break;
  case State::WaitingForPayload:
    onWaitingForPayloadResponse(std::move(value));
    break;
  }
}

void InstanceImpl::onClose() {
  // TODO(talnordan)
}

void InstanceImpl::onInitialResponse(Nats::MessagePtr &&value) {
  UNREFERENCED_PARAMETER(value);

  const std::string hash_key;

  // Send a NATS SUB message.
  const std::string sid = "sid1";
  const Message subMessage =
      message_builder_.createSubMessage(subject_.value(), sid);
  conn_pool_->makeRequest(hash_key, subMessage);

  // Send a NATS PUB message.
  const Message pubMessage =
      message_builder_.createPubMessage(subject_.value());
  conn_pool_->makeRequest(hash_key, pubMessage);

  state_ = State::Published;
}

void InstanceImpl::onPublishedResponse(Nats::MessagePtr &&value) {
  RELEASE_ASSERT(value->asString() == "MSG subject1 sid1 0");
  state_ = State::WaitingForPayload;
}

void InstanceImpl::onWaitingForPayloadResponse(Nats::MessagePtr &&value) {
  RELEASE_ASSERT(value->asString().empty());
  callbacks_.value()->onResponse();
}

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
