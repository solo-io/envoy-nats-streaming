#pragma once

#include "envoy/common/optional.h"
#include "envoy/nats/codec.h"
#include "envoy/nats/streaming/client.h"
#include "envoy/runtime/runtime.h"
#include "envoy/tcp/conn_pool.h"

#include "common/common/logger.h"
#include "common/nats/streaming/heartbeat_handler.h"
#include "common/nats/streaming/message_utility.h"
#include "common/nats/subject_utility.h"
#include "common/nats/token_generator_impl.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

// TODO(talnordan): Maintaining the state of multiple requests and multiple
// inboxes in a single object is becoming cumbersome, error-prone and hard to
// unit-test. Consider refactoring this code into an object hierarchy parallel
// to the inbox hierarchy. After the refactoring, each object is going to be
// responsible for changing its internal state upon incoming messages from a
// particular inbox. Such design would be similar to an actor system.
class ClientImpl : public Client,
                   public Tcp::ConnPool::PoolCallbacks<Message>,
                   public HeartbeatHandler::Callbacks,
                   public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
public:
  ClientImpl(Tcp::ConnPool::InstancePtr<Message> &&conn_pool,
             Runtime::RandomGenerator &random);

  // Nats::Streaming::Client
  PublishRequestPtr makeRequest(const std::string &subject,
                                const std::string &cluster_id,
                                const std::string &discover_prefix,
                                Buffer::Instance &payload,
                                PublishCallbacks &callbacks) override;

  // Tcp::ConnPool::PoolCallbacks
  void onResponse(Nats::MessagePtr &&value) override;
  void onClose() override;

  // Nats::Streaming::HeartbeatHandler::Callbacks
  void send(const Message &message) override;
  void onFailure(const std::string &error) override;

private:
  struct OutboundRequest {
    std::string subject;
    std::string payload;
    PublishCallbacks *callbacks;
  };

  inline void onOperation(Nats::MessagePtr &&value);

  inline void onPayload(Nats::MessagePtr &&value);

  inline void onInfo(Nats::MessagePtr &&value);

  inline void onMsg(std::vector<absl::string_view> &&tokens);

  inline void onPing();

  inline void onConnectResponsePayload(Optional<std::string> &reply_to,
                                       std::string &payload);

  inline void onPubAckPayload(Optional<std::string> &reply_to,
                              std::string &payload);

  inline void subInbox(const std::string &subject);

  inline void subHeartbeatInbox();

  inline void subReplyInbox();

  inline void pubConnectRequest();

  inline void pubPubMsg();

  inline void pong();

  inline void sendNatsMessage(const Message &message);

  // TODO(talnordan): Consider introducing `Nats::streaming::Message` instead of
  // using `std::string`.
  inline void pubNatsStreamingMessage(const std::string &subject,
                                      const std::string &reply_to,
                                      const std::string &message);

  inline void waitForPayload(std::string subject,
                             Optional<std::string> reply_to) {
    subect_and_reply_to_waiting_for_payload_.value(
        make_pair(subject, reply_to));
  }

  inline bool isWaitingForPayload() const {
    return subect_and_reply_to_waiting_for_payload_.valid();
  }

  inline std::string &getSubjectWaitingForPayload() {
    return subect_and_reply_to_waiting_for_payload_.value().first;
  }

  inline Optional<std::string> &getReplyToWaitingForPayload() {
    return subect_and_reply_to_waiting_for_payload_.value().second;
  }

  inline void doneWaitingForPayload() {
    subect_and_reply_to_waiting_for_payload_ =
        Optional<std::pair<std::string, Optional<std::string>>>{};
  }

  inline std::string drainBufferToString(Buffer::Instance &buffer) const;

  inline std::string bufferToString(const Buffer::Instance &buffer) const;

  Tcp::ConnPool::InstancePtr<Message> conn_pool_;
  TokenGeneratorImpl token_generator_;
  const std::string heartbeat_inbox_;
  const std::string root_inbox_;
  const std::string connect_response_inbox_;
  const std::string pub_ack_inbox_;
  uint64_t sid_;
  Optional<std::string> cluster_id_{};
  Optional<std::string> discover_prefix_{};
  Optional<std::pair<std::string, Optional<std::string>>>
      subect_and_reply_to_waiting_for_payload_{};
  Optional<OutboundRequest> outbound_request_{};
  Optional<std::string> pub_prefix_{};

  static const std::string INBOX_PREFIX;
  static const std::string PUB_ACK_PREFIX;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
