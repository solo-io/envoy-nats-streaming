#pragma once

#include "envoy/nats/codec.h"
#include "envoy/nats/publisher.h"
#include "envoy/tcp/conn_pool.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

class InstanceImpl : public Instance {
public:
  InstanceImpl(Tcp::ConnPool::ManagerPtr<std::string> conn_pool_manager);

  // Nats::Publisher::Instance
  PublishRequestPtr makeRequest(const std::string &cluster_name,
                                const std::string &subject,
                                const Buffer::Instance *payload,
                                PublishCallbacks &callbacks) override;

private:
  // TODO(talnordan): Use `Tcp::ConnPool::ManagerPtr<Message>`.
  Tcp::ConnPool::ManagerPtr<std::string> conn_pool_manager_;
};

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
