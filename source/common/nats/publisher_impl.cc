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
                                            Buffer::Instance *payload,
                                            PublishCallbacks &callbacks) {
  UNREFERENCED_PARAMETER(cluster_name);

  subject_.value(subject);
  payload_.value(drainBufferToString(payload));
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
  case State::WaitingForConnectResponsePayload:
    onConnectResponsePayload(std::move(value));
    break;
  case State::SentPubMsg:
    onSentPubMsgResponse(std::move(value));
    break;
  case State::WaitingForPubAckPayload:
    onPubAckPayload(std::move(value));
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
  state_ = State::WaitingForConnectResponsePayload;
}

void InstanceImpl::onConnectResponsePayload(Nats::MessagePtr &&value) {
  const std::string &payload = value->asString();
  pub_prefix_.value(nats_streaming_message_utility_.getPubPrefix(payload));

  // TODO(talnordan): Remove assertion.
  RELEASE_ASSERT(
      StringUtil::startsWith(pub_prefix_.value().c_str(), "_STAN.pub.", true));

  pubPubMsg();
  state_ = State::SentPubMsg;
}

void InstanceImpl::onSentPubMsgResponse(Nats::MessagePtr &&value) {
  // TODO(talnordan): Remove assertion.
  RELEASE_ASSERT(value->asString() == "MSG reply-to.2 2 7");

  // TODO(talnordan): Read payload and parse it.
  state_ = State::WaitingForPubAckPayload;
}

void InstanceImpl::onPubAckPayload(Nats::MessagePtr &&value) {
  const std::string &payload = value->asString();
  auto &&pub_ack = nats_streaming_message_utility_.parsePubAckMessage(payload);

  if (pub_ack.error().empty()) {
    callbacks_.value()->onResponse();
  } else {
    callbacks_.value()->onFailure();
  }

  state_ = State::Done;
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

void InstanceImpl::pubPubMsg() {
  const std::string hash_key;

  // TODO(talnordan): Avoid using hard-coded string literals.
  const std::string pub_msg_message =
      nats_streaming_message_utility_.createPubMsgMessage(
          "client1", "guid1", subject_.value(), payload_.value());
  const Message pubMessage = nats_message_builder_.createPubMessage(
      pub_prefix_.value() + "." + subject_.value(), "reply-to.2",
      pub_msg_message);

  conn_pool_->makeRequest(hash_key, pubMessage);
}

// TODO(talnordan): Consider introducing `BufferUtility` and extracting this
// member function into it.
std::string InstanceImpl::drainBufferToString(Buffer::Instance *buffer) const {
  if (!buffer) {
    return "";
  }

  std::string output = bufferToString(*buffer);
  buffer->drain(buffer->length());
  return output;
}

// TODO(talnordan): This is duplicated from `TestUtility::bufferToString()`.
// Consider moving the code to a shared utility class.
// TODO(talnordan): Consider leveraging the fact that `max_payload` is given in
// the NATS `INFO` message and reuse the same pre-allocated `std:string`.
std::string InstanceImpl::bufferToString(const Buffer::Instance &buffer) const {
  std::string output;
  uint64_t num_slices = buffer.getRawSlices(nullptr, 0);
  Buffer::RawSlice slices[num_slices];
  buffer.getRawSlices(slices, num_slices);
  for (Buffer::RawSlice &slice : slices) {
    output.append(static_cast<const char *>(slice.mem_), slice.len_);
  }

  return output;
}

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
