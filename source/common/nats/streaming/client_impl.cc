#include "common/nats/streaming/client_impl.h"

#include "common/common/assert.h"
#include "common/common/macros.h"
#include "common/common/utility.h"

#include "fmt/format.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

ClientImpl::ClientImpl(Tcp::ConnPool::InstancePtr<Message> &&conn_pool_)
    : conn_pool_(std::move(conn_pool_)) {}

PublishRequestPtr ClientImpl::makeRequest(const std::string &subject,
                                          const std::string &cluster_id,
                                          const std::string &discover_prefix,
                                          Buffer::Instance &payload,
                                          PublishCallbacks &callbacks) {
  cluster_id_.value(cluster_id);
  discover_prefix_.value(discover_prefix);
  outbound_request_.value({subject, drainBufferToString(payload), &callbacks});

  conn_pool_->setPoolCallbacks(*this);

  // Send a NATS CONNECT message.
  sendNatsMessage(nats_message_builder_.createConnectMessage());

  // TODO(talnordan)
  return nullptr;
}

void ClientImpl::onResponse(Nats::MessagePtr &&value) {
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

void ClientImpl::onClose() {
  // TODO(talnordan)
}

void ClientImpl::onOperation(Nats::MessagePtr &&value) {
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
    // TODO(talnordan): Error handling.
    // TODO(talnordan): Increment error stats.
    throw ProtocolError("invalid message");
  }
}

void ClientImpl::onPayload(Nats::MessagePtr &&value) {
  // Mark that the payload has been received.
  waiting_for_payload_ = false;

  if (heartbeat_reply_to_.valid()) {
    sendNatsMessage(
        nats_message_builder_.createPubMessage(heartbeat_reply_to_.value()));
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

void ClientImpl::onInfo(Nats::MessagePtr &&value) {
  // TODO(talnordan): Process `INFO` options.
  UNREFERENCED_PARAMETER(value);

  // TODO(talnordan): The following behavior is part of the PoC implementation.
  subHeartbeatInbox();
  subReplyInbox();
  pubConnectRequest();
}

void ClientImpl::onMsg(std::vector<absl::string_view> &&tokens) {
  auto num_tokens = tokens.size();
  if (!(num_tokens == 4 || num_tokens == 5)) {
    // TODO(talnordan): Error handling.
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

void ClientImpl::onPing() { pong(); }

void ClientImpl::onIncomingHeartbeat(std::vector<absl::string_view> &&tokens) {
  if (tokens.size() != 5) {
    // TODO(talnordan): Error handling.
    throw ProtocolError("invalid incoming heartbeat");
  }

  // TODO(talnordan): Remove assertion.
  RELEASE_ASSERT(!heartbeat_reply_to_.valid());
  heartbeat_reply_to_.value(std::string(tokens[3]));
}

void ClientImpl::onInitialResponse(std::vector<absl::string_view> &&tokens) {
  UNREFERENCED_PARAMETER(tokens);
}

void ClientImpl::onConnectResponsePayload(Nats::MessagePtr &&value) {
  const std::string &payload = value->asString();
  pub_prefix_.value(nats_streaming_message_utility_.getPubPrefix(payload));

  // TODO(talnordan): Remove assertion.
  RELEASE_ASSERT(
      StringUtil::startsWith(pub_prefix_.value().c_str(), "_STAN.pub.", true));

  pubPubMsg();
  state_ = State::SentPubMsg;
}

void ClientImpl::onSentPubMsgResponse(std::vector<absl::string_view> &&tokens) {
  // TODO(talnordan): Remove assertion.
  std::vector<absl::string_view> expected_tokens{"MSG", "reply-to.2", "2", "7"};
  RELEASE_ASSERT(tokens == expected_tokens);
}

void ClientImpl::onPubAckPayload(Nats::MessagePtr &&value) {
  const std::string &payload = value->asString();
  auto &&pub_ack = nats_streaming_message_utility_.parsePubAckMessage(payload);
  auto &&callbacks = outbound_request_.value().callbacks;

  if (pub_ack.error().empty()) {
    callbacks->onResponse();
  } else {
    callbacks->onFailure();
  }

  state_ = State::Done;
}

void ClientImpl::subInbox(const std::string &subject, const std::string &sid) {
  sendNatsMessage(nats_message_builder_.createSubMessage(subject, sid));
}

void ClientImpl::subHeartbeatInbox() {
  // TODO(talnordan): Avoid using hard-coded string literals.
  subInbox("heartbeat-inbox", "1");
}

void ClientImpl::subReplyInbox() {
  // TODO(talnordan): Avoid using hard-coded string literals.
  subInbox("reply-to.*", "2");
}

void ClientImpl::pubConnectRequest() {
  const std::string hash_key;

  // TODO(talnordan): Extract a helper function for prepending a prefix to a
  // subject.
  const std::string subject =
      fmt::format("{}.{}", discover_prefix_.value(), cluster_id_.value());

  // TODO(talnordan): Avoid using hard-coded string literals.
  const std::string connect_request_message =
      nats_streaming_message_utility_.createConnectRequestMessage(
          "client1", "heartbeat-inbox");
  const Message pubMessage = nats_message_builder_.createPubMessage(
      subject, "reply-to.1", connect_request_message);

  sendNatsMessage(pubMessage);
}

void ClientImpl::pubPubMsg() {
  auto &&subject = outbound_request_.value().subject;
  auto &&payload = outbound_request_.value().payload;

  // TODO(talnordan): Avoid using hard-coded string literals.
  const std::string pub_msg_message =
      nats_streaming_message_utility_.createPubMsgMessage("client1", "guid1",
                                                          subject, payload);
  const Message pubMessage = nats_message_builder_.createPubMessage(
      pub_prefix_.value() + "." + subject, "reply-to.2", pub_msg_message);

  sendNatsMessage(pubMessage);
}

void ClientImpl::pong() {
  sendNatsMessage(nats_message_builder_.createPongMessage());
}

inline void ClientImpl::sendNatsMessage(const Message &message) {
  // TODO(talnordan): Manage hash key computation.
  const std::string hash_key;

  conn_pool_->makeRequest(hash_key, message);
}

// TODO(talnordan): Consider introducing `BufferUtility` and extracting this
// member function into it.
std::string ClientImpl::drainBufferToString(Buffer::Instance &buffer) const {
  std::string output = bufferToString(buffer);
  buffer.drain(buffer.length());
  return output;
}

// TODO(talnordan): This is duplicated from `TestUtility::bufferToString()`.
// Consider moving the code to a shared utility class.
// TODO(talnordan): Consider leveraging the fact that `max_payload` is given in
// the NATS `INFO` message and reuse the same pre-allocated `std:string`.
std::string ClientImpl::bufferToString(const Buffer::Instance &buffer) const {
  std::string output;
  uint64_t num_slices = buffer.getRawSlices(nullptr, 0);
  Buffer::RawSlice slices[num_slices];
  buffer.getRawSlices(slices, num_slices);
  for (Buffer::RawSlice &slice : slices) {
    output.append(static_cast<const char *>(slice.mem_), slice.len_);
  }

  return output;
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
