#include "common/nats/publisher_impl.h"

#include "common/common/assert.h"
#include "common/common/macros.h"
#include "common/common/utility.h"

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
  const Message request = nats_message_builder_.createConnectMessage();
  conn_pool_->makeRequest(hash_key, request);

  // TODO(talnordan)
  return nullptr;
}

void InstanceImpl::onResponse(Nats::MessagePtr &&value) {
  ENVOY_LOG(trace, "on response: value is\n[{}]", value->asString());
  switch (state_) {
  case State::Initial:
    onInitialResponse(std::move(value));
    break;
  case State::SentConnectRequest:
    onSentConnectRequestResponse(std::move(value));
    break;
  case State::WaitingForPayload:
    onWaitingForPayloadResponse(std::move(value));
    break;
  case State::Done:
    break;
  }
}

void InstanceImpl::onClose() {
  // TODO(talnordan)
}

void InstanceImpl::onInitialResponse(Nats::MessagePtr &&value) {
  UNREFERENCED_PARAMETER(value);

  subHeartbeatInbox();
  subReplyInbox();
  pubConnectRequest();

  state_ = State::SentConnectRequest;
}

void InstanceImpl::onSentConnectRequestResponse(Nats::MessagePtr &&value) {
  UNREFERENCED_PARAMETER(value);
  state_ = State::WaitingForPayload;
}

void InstanceImpl::onWaitingForPayloadResponse(Nats::MessagePtr &&value) {
  const std::string &payload = value->asString();
  const std::string pub_prefix =
      nats_streaming_message_utility_.getPubPrefix(payload);

  // TODO(talnordan)
  RELEASE_ASSERT(
      StringUtil::startsWith(pub_prefix.c_str(), "_STAN.pub.", true));
  state_ = State::Done;
  callbacks_.value()->onResponse();
}

void InstanceImpl::subHeartbeatInbox() {
  const std::string hash_key;

  // TODO(talnordan): Avoid using hard-coded string literals.
  const Message subMessage =
      nats_message_builder_.createSubMessage("heartbeat-inbox", "1");

  conn_pool_->makeRequest(hash_key, subMessage);
}

void InstanceImpl::subReplyInbox() {
  const std::string hash_key;

  // TODO(talnordan): Avoid using hard-coded string literals.
  const Message subMessage =
      nats_message_builder_.createSubMessage("reply-to.*", "2");

  conn_pool_->makeRequest(hash_key, subMessage);
}

void InstanceImpl::pubConnectRequest() {
  const std::string hash_key;

  // TODO(talnordan): Avoid using hard-coded string literals.
  const std::string connect_request_message =
      nats_streaming_message_utility_.createConnectRequestMessage(
          "client1", "heartbeat-inbox");
  const Message pubMessage = nats_message_builder_.createPubMessage(
      "_STAN.discover.test-cluster", "reply-to.1", connect_request_message);

  conn_pool_->makeRequest(hash_key, pubMessage);
}

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
