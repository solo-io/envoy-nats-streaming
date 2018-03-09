#pragma once

#include "envoy/common/optional.h"
#include "envoy/nats/codec.h"
#include "envoy/nats/streaming/client.h"
#include "envoy/tcp/conn_pool.h"

#include "common/common/logger.h"
#include "common/nats/message_builder.h"
#include "common/nats/streaming/message_utility.h"

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
                   public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
public:
  ClientImpl(Tcp::ConnPool::InstancePtr<Message> &&conn_pool);

  // Nats::Streaming::Client
  PublishRequestPtr makeRequest(const std::string &subject,
                                const std::string &cluster_id,
                                const std::string &discover_prefix,
                                Buffer::Instance &payload,
                                PublishCallbacks &callbacks) override;

  // Tcp::ConnPool::PoolCallbacks
  void onResponse(Nats::MessagePtr &&value) override;
  void onClose() override;

private:
  struct OutboundRequest {
    std::string subject;
    std::string payload;
    PublishCallbacks *callbacks;
  };

  enum class State {
    Initial,
    SentPubMsg,
    Done,
  };

  inline void onOperation(Nats::MessagePtr &&value);

  inline void onPayload(Nats::MessagePtr &&value);

  inline void onInfo(Nats::MessagePtr &&value);

  inline void onMsg(std::vector<absl::string_view> &&tokens);

  inline void onPing();

  inline void onIncomingHeartbeat(std::vector<absl::string_view> &&tokens);

  inline void onConnectResponsePayload(Nats::MessagePtr &&value);

  inline void onPubAckPayload(Nats::MessagePtr &&value);

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

  inline std::string drainBufferToString(Buffer::Instance &buffer) const;

  inline std::string bufferToString(const Buffer::Instance &buffer) const;

  Tcp::ConnPool::InstancePtr<Message> conn_pool_;
  Nats::MessageBuilder nats_message_builder_;
  Nats::Streaming::MessageUtility nats_streaming_message_utility_;
  State state_{};
  uint64_t sid_;
  bool waiting_for_payload_{};
  Optional<std::string> heartbeat_reply_to_{};
  Optional<std::string> cluster_id_{};
  Optional<std::string> discover_prefix_{};
  Optional<OutboundRequest> outbound_request_{};
  Optional<std::string> pub_prefix_{};

  static const std::string HEARTBEAT_INBOX;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
