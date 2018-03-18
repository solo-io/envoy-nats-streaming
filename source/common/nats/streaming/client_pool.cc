#include "common/nats/streaming/client_pool.h"

#include "common/nats/codec_impl.h"
#include "common/nats/streaming/client_impl.h"
#include "common/tcp/conn_pool_impl.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

ClientPool::ClientPool(const std::string &cluster_name,
                       Upstream::ClusterManager &cm,
                       Tcp::ConnPool::ClientFactory<Message> &client_factory,
                       ThreadLocal::SlotAllocator &tls,
                       Runtime::RandomGenerator &random)
    : cm_(cm), client_factory_(client_factory), slot_(tls.allocateSlot()),
      random_(random) {
  slot_->set([this, cluster_name](Event::Dispatcher &dispatcher)
                 -> ThreadLocal::ThreadLocalObjectSharedPtr {
    Tcp::ConnPool::InstancePtr<Message> conn_pool(
        new Tcp::ConnPool::InstanceImpl<Message, DecoderImpl>(
            cluster_name, cm_, client_factory_, dispatcher));
    return std::make_shared<ThreadLocalPool>(std::move(conn_pool), random_);
  });
}

PublishRequestPtr ClientPool::makeRequest(const std::string &subject,
                                          const std::string &cluster_id,
                                          const std::string &discover_prefix,
                                          Buffer::Instance &payload,
                                          PublishCallbacks &callbacks) {
  return slot_->getTyped<ThreadLocalPool>().getClient().makeRequest(
      subject, cluster_id, discover_prefix, payload, callbacks);
}

ClientPool::ThreadLocalPool::ThreadLocalPool(
    Tcp::ConnPool::InstancePtr<Message> &&conn_pool,
    Runtime::RandomGenerator &random)
    : client_(std::move(conn_pool), random) {}

Client &ClientPool::ThreadLocalPool::getClient() { return client_; }

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
