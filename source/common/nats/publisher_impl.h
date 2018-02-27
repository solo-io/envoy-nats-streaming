#pragma once

#include "envoy/common/optional.h"
#include "envoy/nats/codec.h"
#include "envoy/nats/publisher.h"
#include "envoy/tcp/conn_pool.h"

#include "common/nats/message_builder.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

class InstanceImpl : public Instance,
                     public Tcp::ConnPool::PoolCallbacks<Message> {
public:
  InstanceImpl(Tcp::ConnPool::InstancePtr<Message> &&conn_pool);

  // Nats::Publisher::Instance
  PublishRequestPtr makeRequest(const std::string &cluster_name,
                                const std::string &subject,
                                const Buffer::Instance *payload,
                                PublishCallbacks &callbacks) override;

  // Tcp::ConnPool::PoolCallbacks
  void onResponse(Nats::MessagePtr &&value) override;
  void onClose() override;

private:
  enum class State {
    Initial,
    Published,
    WaitingForPayload,
  };

  inline void onInitialResponse(Nats::MessagePtr &&value);

  inline void onPublishedResponse(Nats::MessagePtr &&value);

  inline void onWaitingForPayloadResponse(Nats::MessagePtr &&value);

  Tcp::ConnPool::InstancePtr<Message> conn_pool_;
  MessageBuilder message_builder_;
  State state_{};
  Optional<std::string> subject_{};

  // TODO(talnordan): This should be a collection.
  Optional<PublishCallbacks *> callbacks_{};
};

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
