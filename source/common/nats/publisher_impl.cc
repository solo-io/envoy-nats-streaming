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
                                            Buffer::Instance &payload,
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

  // Check whether a payload is expected prior to NATS operation extraction.
  // TODO(talnordan): Eventually, we might want `onResponse()` to be passed a
  // single decoded message consisting of both the `MSG` arguments and the
  // payload.
  if (waiting_for_payload_) {
    onPayload(std::move(value));
  } else {
    onOperation(std::move(value));
  }
}

void InstanceImpl::onClose() {
  // TODO(talnordan)
}

void InstanceImpl::onOperation(Nats::MessagePtr &&value) {
  // TODO(talnordan): This implementation is provided as a proof of concept. In
  // a production-ready implementation, the decoder should use zero allocation
  // byte parsing, and this code should switch over an `enum class` representing
  // the message type. See:
  // https://github.com/nats-io/go-nats/blob/master/parser.go
  // https://youtu.be/ylRKac5kSOk?t=10m46s

  auto delimiters = " \t";
  auto keep_empty_string = false;
  auto tokens =
      StringUtil::splitToken(value->asString(), delimiters, keep_empty_string);

  auto &&op = tokens[0];
  if (StringUtil::caseCompare(op, "INFO")) {
    onInfo(std::move(value));
  } else if (StringUtil::caseCompare(op, "MSG")) {
    onMsg(std::move(tokens));
  } else if (StringUtil::caseCompare(op, "PING")) {
    onPing();
  } else {
    throw ProtocolError("invalid message");
  }
}

void InstanceImpl::onPayload(Nats::MessagePtr &&value) {
  // Mark that the payload has been received.
  waiting_for_payload_ = false;

  if (heartbeat_reply_to_.valid()) {
    const std::string hash_key;
    const Message hearbeatResponseMessage =
        nats_message_builder_.createPubMessage(heartbeat_reply_to_.value());
    conn_pool_->makeRequest(hash_key, hearbeatResponseMessage);

    heartbeat_reply_to_ = Optional<std::string>{};
  } else {
    switch (state_) {
    case State::Initial:
      onConnectResponsePayload(std::move(value));
      break;
    case State::SentPubMsg:
      onPubAckPayload(std::move(value));
      break;
    case State::Done:
      break;
    }
  }
}

void InstanceImpl::onInfo(Nats::MessagePtr &&value) {
  // TODO(talnordan): Process `INFO` options.
  UNREFERENCED_PARAMETER(value);

  // TODO(talnordan): The following behavior is part of the PoC implementation.
  subHeartbeatInbox();
  subReplyInbox();
  pubConnectRequest();
}

void InstanceImpl::onMsg(std::vector<absl::string_view> &&tokens) {
  auto num_tokens = tokens.size();
  if (!(num_tokens == 4 || num_tokens == 5)) {
    throw ProtocolError("invalid MSG");
  }

  waiting_for_payload_ = true;

  // TODO(talnordan): Avoid using hard-coded string literals.
  const auto &subject = tokens[1];
  if (subject == "heartbeat-inbox") {
    onIncomingHeartbeat(std::move(tokens));
    return;
  }

  switch (state_) {
  case State::Initial:
    onInitialResponse(std::move(tokens));
    break;
  case State::SentPubMsg:
    onSentPubMsgResponse(std::move(tokens));
    break;
  case State::Done:
    break;
  }
}

void InstanceImpl::onPing() { pong(); }

void InstanceImpl::onIncomingHeartbeat(
    std::vector<absl::string_view> &&tokens) {
  if (tokens.size() != 5) {
    throw ProtocolError("invalid incoming heartbeat");
  }

  RELEASE_ASSERT(!heartbeat_reply_to_.valid());
  heartbeat_reply_to_.value(std::string(tokens[3]));
}

void InstanceImpl::onInitialResponse(std::vector<absl::string_view> &&tokens) {
  UNREFERENCED_PARAMETER(tokens);
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

void InstanceImpl::onSentPubMsgResponse(
    std::vector<absl::string_view> &&tokens) {
  // TODO(talnordan): Remove assertion.
  std::vector<absl::string_view> expected_tokens{"MSG", "reply-to.2", "2", "7"};
  RELEASE_ASSERT(tokens == expected_tokens);
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

void InstanceImpl::pong() {
  const std::string hash_key;
  const Message pongMessage = nats_message_builder_.createPongMessage();
  conn_pool_->makeRequest(hash_key, pongMessage);
}

// TODO(talnordan): Consider introducing `BufferUtility` and extracting this
// member function into it.
std::string InstanceImpl::drainBufferToString(Buffer::Instance &buffer) const {
  std::string output = bufferToString(buffer);
  buffer.drain(buffer.length());
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
