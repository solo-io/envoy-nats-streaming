#pragma once

#include "envoy/nats/codec.h"
#include "envoy/nats/streaming/client.h"
#include "envoy/tcp/conn_pool.h"
#include "envoy/thread_local/thread_local.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/nats/streaming/client_impl.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class ClientPool : public Client {
public:
  ClientPool(const std::string &cluster_name, Upstream::ClusterManager &cm,
             Tcp::ConnPool::ClientFactory<Message> &client_factory,
             ThreadLocal::SlotAllocator &tls, Runtime::RandomGenerator &random,
             const std::chrono::milliseconds &op_timeout);

  // Nats::Streaming::Client
  PublishRequestPtr makeRequest(const std::string &subject,
                                const std::string &cluster_id,
                                const std::string &discover_prefix,
                                Buffer::Instance &payload,
                                PublishCallbacks &callbacks) override;

private:
  struct ThreadLocalPool : public ThreadLocal::ThreadLocalObject {
    ThreadLocalPool(Tcp::ConnPool::InstancePtr<Message> &&conn_pool,
                    Runtime::RandomGenerator &random,
                    Event::Dispatcher &dispatcher,
                    const std::chrono::milliseconds &op_timeout);
    Client &getClient();

  private:
    ClientImpl client_;
  };

  Upstream::ClusterManager &cm_;
  Tcp::ConnPool::ClientFactory<Message> &client_factory_;
  ThreadLocal::SlotPtr slot_;
  Runtime::RandomGenerator &random_;
  const std::chrono::milliseconds op_timeout_;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
