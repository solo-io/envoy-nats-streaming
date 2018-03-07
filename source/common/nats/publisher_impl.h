#pragma once

#include "envoy/common/optional.h"
#include "envoy/nats/codec.h"
#include "envoy/nats/publisher.h"
#include "envoy/tcp/conn_pool.h"

#include "common/common/logger.h"
#include "common/nats/message_builder.h"
#include "common/nats/streaming/message_utility.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

class InstanceImpl : public Instance,
                     public Tcp::ConnPool::PoolCallbacks<Message>,
                     public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
public:
  InstanceImpl(Tcp::ConnPool::InstancePtr<Message> &&conn_pool);

  // Nats::Publisher::Instance
  PublishRequestPtr makeRequest(const std::string &cluster_name,
                                const std::string &subject,
                                Buffer::Instance &payload,
                                PublishCallbacks &callbacks) override;

  // Tcp::ConnPool::PoolCallbacks
  void onResponse(Nats::MessagePtr &&value) override;
  void onClose() override;

private:
  enum class State {
    Initial,
    SentPubMsg,
    Done,
  };

  inline void onOperation(Nats::MessagePtr &&value);

  inline void onPayload(Nats::MessagePtr &&value);

  inline void onInfo(Nats::MessagePtr &&value);

  inline void onMsg(Nats::MessagePtr &&value);

  inline void onPing();

  inline void onInitialResponse(Nats::MessagePtr &&value);

  inline void onConnectResponsePayload(Nats::MessagePtr &&value);

  inline void onSentPubMsgResponse(Nats::MessagePtr &&value);

  inline void onPubAckPayload(Nats::MessagePtr &&value);

  inline void subHeartbeatInbox();

  inline void subReplyInbox();

  inline void pubConnectRequest();

  inline void pubPubMsg();

  inline void pong();

  inline std::string drainBufferToString(Buffer::Instance &buffer) const;

  inline std::string bufferToString(const Buffer::Instance &buffer) const;

  Tcp::ConnPool::InstancePtr<Message> conn_pool_;
  Nats::MessageBuilder nats_message_builder_;
  Nats::Streaming::MessageUtility nats_streaming_message_utility_;
  State state_{};
  bool waiting_for_payload_{};
  Optional<std::string> subject_{};
  Optional<std::string> payload_{};
  Optional<std::string> pub_prefix_{};

  // TODO(talnordan): This should be a collection.
  Optional<PublishCallbacks *> callbacks_{};
};

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
