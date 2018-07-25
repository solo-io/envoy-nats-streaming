#pragma once

#include <map>

#include "envoy/event/timer.h"
#include "envoy/nats/codec.h"
#include "envoy/nats/streaming/client.h"
#include "envoy/runtime/runtime.h"
#include "envoy/tcp/conn_pool_nats.h"

#include "common/common/logger.h"
#include "common/nats/streaming/connect_response_handler.h"
#include "common/nats/streaming/heartbeat_handler.h"
#include "common/nats/streaming/message_utility.h"
#include "common/nats/streaming/pub_request_handler.h"
#include "common/nats/subject_utility.h"
#include "common/nats/token_generator_impl.h"

#include "absl/types/optional.h"

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
                   public ConnectResponseHandler::Callbacks,
                   public HeartbeatHandler::Callbacks,
                   public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
public:
  ClientImpl(Tcp::ConnPool::InstancePtr<Message> &&conn_pool,
             Runtime::RandomGenerator &random, Event::Dispatcher &dispatcher,
             const std::chrono::milliseconds &op_timeout);

  // Nats::Streaming::Client
  PublishRequestPtr makeRequest(const std::string &subject,
                                const std::string &cluster_id,
                                const std::string &discover_prefix,
                                Buffer::Instance &payload,
                                PublishCallbacks &callbacks) override;

  // Tcp::ConnPool::PoolCallbacks
  void onResponse(Nats::MessagePtr &&value) override;
  void onClose() override;

  // Nats::Streaming::InboxCallbacks
  void onFailure(const std::string &error) override;

  // Nats::Streaming::ConnectResponseHandler::Callbacks
  void onConnected(const std::string &pub_prefix) override;

  // Nats::Streaming::HeartbeatHandler::Callbacks
  void send(const Message &message) override;

  void cancel(const std::string &pub_ack_inbox);

private:
  enum class State { NotConnected, Connecting, Connected };

  struct PendingRequest {
    std::string subject;
    std::string payload;
    PublishCallbacks *callbacks;
  };

  class PublishRequestCanceler : public PublishRequest {
  public:
    PublishRequestCanceler(ClientImpl &parent, std::string &&pub_ack_inbox);

    // Nats::Streaming::PublishRequest
    void cancel();

  private:
    ClientImpl &parent_;
    const std::string pub_ack_inbox_;
  };

  inline void onOperation(Nats::MessagePtr &&value);

  inline void onPayload(Nats::MessagePtr &&value);

  inline void onInfo(Nats::MessagePtr &&value);

  inline void onMsg(std::vector<absl::string_view> &&tokens);

  inline void onPing();

  inline void onTimeout(const std::string &pub_ack_inbox);

  inline void subInbox(const std::string &subject);

  inline void subChildWildcardInbox(const std::string &parent_subject);

  inline void subHeartbeatInbox();

  inline void subReplyInbox();

  inline void subPubAckInbox();

  inline void pubConnectRequest();

  inline void enqueuePendingRequest(const std::string &subject,
                                    const std::string &payload,
                                    PublishCallbacks &callbacks,
                                    const std::string &pub_ack_inbox);

  inline void pubPubMsg(const std::string &subject, const std::string &payload,
                        PublishCallbacks &callbacks,
                        const std::string &pub_ack_inbox);

  inline void pong();

  inline void sendNatsMessage(const Message &message);

  // TODO(talnordan): Consider introducing `Nats::streaming::Message` instead of
  // using `std::string`.
  inline void pubNatsStreamingMessage(const std::string &subject,
                                      const std::string &reply_to,
                                      const std::string &message);

  inline void waitForPayload(std::string subject,
                             absl::optional<std::string> reply_to) {
    subect_and_reply_to_waiting_for_payload_.emplace(
        make_pair(subject, reply_to));
  }

  inline bool isWaitingForPayload() const {
    return subect_and_reply_to_waiting_for_payload_.has_value();
  }

  inline std::string &getSubjectWaitingForPayload() {
    return subect_and_reply_to_waiting_for_payload_.value().first;
  }

  inline absl::optional<std::string> &getReplyToWaitingForPayload() {
    return subect_and_reply_to_waiting_for_payload_.value().second;
  }

  inline void doneWaitingForPayload() {
    subect_and_reply_to_waiting_for_payload_ =
        absl::optional<std::pair<std::string, absl::optional<std::string>>>{};
  }

  Tcp::ConnPool::InstancePtr<Message> conn_pool_;
  TokenGeneratorImpl token_generator_;
  Event::Dispatcher &dispatcher_;
  const std::chrono::milliseconds op_timeout_;
  State state_{};
  const std::string heartbeat_inbox_;
  const std::string root_inbox_;
  const std::string root_pub_ack_inbox_;
  const std::string connect_response_inbox_;
  const std::string client_id_;
  std::map<std::string, PendingRequest> pending_request_per_inbox_;
  std::map<std::string, PubRequest> pub_request_per_inbox_;
  uint64_t sid_;
  absl::optional<std::string> cluster_id_{};
  absl::optional<std::string> discover_prefix_{};
  absl::optional<std::pair<std::string, absl::optional<std::string>>>
      subect_and_reply_to_waiting_for_payload_{};
  absl::optional<std::string> pub_prefix_{};

  static const std::string INBOX_PREFIX;
  static const std::string PUB_ACK_PREFIX;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
