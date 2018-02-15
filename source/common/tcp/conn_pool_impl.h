#pragma once

#include <type_traits>

#include "envoy/event/dispatcher.h"
#include "envoy/network/connection.h"
#include "envoy/tcp/conn_pool.h"
#include "envoy/thread_local/thread_local.h"
#include "envoy/upstream/cluster_manager.h"
#include "envoy/upstream/upstream.h"

#include "common/buffer/buffer_impl.h"
#include "common/common/assert.h"
#include "common/network/filter_impl.h"
#include "common/tcp/codec_impl.h"

namespace Envoy {
namespace Tcp {
namespace ConnPool {

class ConfigImpl : public Config {
public:
  ConfigImpl(const std::chrono::milliseconds &op_timeout);

  bool disableOutlierEvents() const override { return false; }
  std::chrono::milliseconds opTimeout() const override { return op_timeout_; }

private:
  const std::chrono::milliseconds op_timeout_;
};

template <typename T>
class ClientImpl : public Client<T>,
                   public DecoderCallbacks<T>,
                   public Network::ConnectionCallbacks {
public:
  static ClientPtr<T> create(Upstream::HostConstSharedPtr host,
                             Event::Dispatcher &dispatcher,
                             EncoderPtr<T> &&encoder,
                             DecoderFactory<T> &decoder_factory,
                             const Config &config) {
    std::unique_ptr<ClientImpl> client(new ClientImpl(
        host, dispatcher, std::move(encoder), decoder_factory, config));
    client->connection_ = host->createConnection(dispatcher).connection_;
    client->connection_->addConnectionCallbacks(*client);
    client->connection_->addReadFilter(
        Network::ReadFilterSharedPtr{new UpstreamReadFilter(*client)});
    client->connection_->connect();
    client->connection_->noDelay(true);
    return std::move(client);
  }

  ~ClientImpl() {
    ASSERT(pending_requests_.empty());
    ASSERT(connection_->state() == Network::Connection::State::Closed);
    host_->cluster().stats().upstream_cx_active_.dec();
    host_->stats().cx_active_.dec();
  }

  // Tcp::ConnPool::Client
  void
  addConnectionCallbacks(Network::ConnectionCallbacks &callbacks) override {
    connection_->addConnectionCallbacks(callbacks);
  }
  void close() override {
    connection_->close(Network::ConnectionCloseType::NoFlush);
  }
  PoolRequest *makeRequest(const T &request,
                           PoolCallbacks<T> &callbacks) override {
    ASSERT(connection_->state() == Network::Connection::State::Open);

    pending_requests_.emplace_back(*this, callbacks);
    encoder_->encode(request, encoder_buffer_);
    connection_->write(encoder_buffer_);

    // Only boost the op timeout if:
    // - We are not already connected. Otherwise, we are governed by the connect
    // timeout and the timer
    //   will be reset when/if connection occurs. This allows a relatively long
    //   connection spin up time for example if TLS is being used.
    // - This is the first request on the pipeline. Otherwise the timeout would
    // effectively start on
    //   the last operation.
    if (connected_ && pending_requests_.size() == 1) {
      connect_or_op_timer_->enableTimer(config_.opTimeout());
    }

    return &pending_requests_.back();
  }

private:
  struct UpstreamReadFilter : public Network::ReadFilterBaseImpl {
    UpstreamReadFilter(ClientImpl<T> &parent) : parent_(parent) {}

    // Network::ReadFilter
    Network::FilterStatus onData(Buffer::Instance &data) override {
      parent_.onData(data);
      return Network::FilterStatus::Continue;
    }

    ClientImpl &parent_;
  };

  struct PendingRequest : public PoolRequest {
    PendingRequest(ClientImpl &parent, PoolCallbacks<T> &callbacks)
        : parent_(parent), callbacks_(callbacks) {
      parent.host_->cluster().stats().upstream_rq_total_.inc();
      parent.host_->cluster().stats().upstream_rq_active_.inc();
      parent.host_->stats().rq_total_.inc();
      parent.host_->stats().rq_active_.inc();
    }
    ~PendingRequest() {
      parent_.host_->cluster().stats().upstream_rq_active_.dec();
      parent_.host_->stats().rq_active_.dec();
    }

    // Tcp::ConnPool::PoolRequest
    void cancel() override {
      // If we get a cancellation, we just mark the pending request as
      // cancelled, and then we drop the response as it comes through. There is
      // no reason to blow away the connection when the remote is already
      // responding as fast as possible.
      canceled_ = true;
    }

    ClientImpl &parent_;
    PoolCallbacks<T> &callbacks_;
    bool canceled_{};
  };

  ClientImpl(Upstream::HostConstSharedPtr host, Event::Dispatcher &dispatcher,
             EncoderPtr<T> &&encoder, DecoderFactory<T> &decoder_factory,
             const Config &config)
      : host_(host), encoder_(std::move(encoder)),
        decoder_(decoder_factory.create(*this)), config_(config),
        connect_or_op_timer_(dispatcher.createTimer(
            [this]() -> void { onConnectOrOpTimeout(); })) {
    host->cluster().stats().upstream_cx_total_.inc();
    host->cluster().stats().upstream_cx_active_.inc();
    host->stats().cx_total_.inc();
    host->stats().cx_active_.inc();
    connect_or_op_timer_->enableTimer(host->cluster().connectTimeout());
  }
  void onConnectOrOpTimeout() {
    putOutlierEvent(Upstream::Outlier::Result::TIMEOUT);
    if (connected_) {
      host_->cluster().stats().upstream_rq_timeout_.inc();
    } else {
      host_->cluster().stats().upstream_cx_connect_timeout_.inc();
    }

    connection_->close(Network::ConnectionCloseType::NoFlush);
  }
  void onData(Buffer::Instance &data) {
    try {
      decoder_->decode(data);
    } catch (ProtocolError &) {
      putOutlierEvent(Upstream::Outlier::Result::REQUEST_FAILED);
      host_->cluster().stats().upstream_cx_protocol_error_.inc();
      connection_->close(Network::ConnectionCloseType::NoFlush);
    }
  }
  void putOutlierEvent(Upstream::Outlier::Result result) {
    if (!config_.disableOutlierEvents()) {
      host_->outlierDetector().putResult(result);
    }
  }

  // Tcp::DecoderCallbacks
  void onValue(MessagePtr<T> &&value) override {
    ASSERT(!pending_requests_.empty());
    PendingRequest &request = pending_requests_.front();
    if (!request.canceled_) {
      request.callbacks_.onResponse(std::move(value));
    } else {
      host_->cluster().stats().upstream_rq_cancelled_.inc();
    }
    pending_requests_.pop_front();

    // If there are no remaining ops in the pipeline we need to disable the
    // timer. Otherwise we boost the timer since we are receiving responses and
    // there are more to flush out.
    if (pending_requests_.empty()) {
      connect_or_op_timer_->disableTimer();
    } else {
      connect_or_op_timer_->enableTimer(config_.opTimeout());
    }

    putOutlierEvent(Upstream::Outlier::Result::SUCCESS);
  }

  // Network::ConnectionCallbacks
  void onEvent(Network::ConnectionEvent event) override {
    if (event == Network::ConnectionEvent::RemoteClose ||
        event == Network::ConnectionEvent::LocalClose) {
      if (!pending_requests_.empty()) {
        host_->cluster().stats().upstream_cx_destroy_with_active_rq_.inc();
        if (event == Network::ConnectionEvent::RemoteClose) {
          putOutlierEvent(Upstream::Outlier::Result::SERVER_FAILURE);
          host_->cluster()
              .stats()
              .upstream_cx_destroy_remote_with_active_rq_.inc();
        }
        if (event == Network::ConnectionEvent::LocalClose) {
          host_->cluster()
              .stats()
              .upstream_cx_destroy_local_with_active_rq_.inc();
        }
      }

      while (!pending_requests_.empty()) {
        PendingRequest &request = pending_requests_.front();
        if (!request.canceled_) {
          request.callbacks_.onFailure();
        } else {
          host_->cluster().stats().upstream_rq_cancelled_.inc();
        }
        pending_requests_.pop_front();
      }

      connect_or_op_timer_->disableTimer();
    } else if (event == Network::ConnectionEvent::Connected) {
      connected_ = true;
      ASSERT(!pending_requests_.empty());
      connect_or_op_timer_->enableTimer(config_.opTimeout());
    }

    if (event == Network::ConnectionEvent::RemoteClose && !connected_) {
      host_->cluster().stats().upstream_cx_connect_fail_.inc();
      host_->stats().cx_connect_fail_.inc();
    }
  }
  void onAboveWriteBufferHighWatermark() override {}
  void onBelowWriteBufferLowWatermark() override {}

  Upstream::HostConstSharedPtr host_;
  Network::ClientConnectionPtr connection_;
  EncoderPtr<T> encoder_;
  Buffer::OwnedImpl encoder_buffer_;
  DecoderPtr decoder_;
  const Config &config_;
  std::list<PendingRequest> pending_requests_;
  Event::TimerPtr connect_or_op_timer_;
  bool connected_{};
};

template <typename T, typename E, typename D>
class ClientFactoryImpl : public ClientFactory<T> {
  static_assert(std::is_base_of<Encoder<T>, E>::value,
                "Encoder<T> should be a base of E");
  static_assert(std::is_base_of<Decoder, D>::value,
                "Decoder should be a base of D");

public:
  // Tcp::ConnPool::ClientFactoryImpl
  ClientPtr<T> create(Upstream::HostConstSharedPtr host,
                      Event::Dispatcher &dispatcher,
                      const Config &config) override {
    return ClientImpl<T>::create(host, dispatcher, EncoderPtr<T>{new E()},
                                 decoder_factory_, config);
  }

  static ClientFactoryImpl<T, E, D> instance_;

private:
  DecoderFactoryImpl<T, D> decoder_factory_;
};

template <typename T, typename E, typename D>
ClientFactoryImpl<T, E, D> ClientFactoryImpl<T, E, D>::instance_;

template <typename T, typename D> class InstanceImpl : public Instance<T> {
public:
  InstanceImpl(const std::string &cluster_name, Upstream::ClusterManager &cm,
               ClientFactory<T> &client_factory,
               ThreadLocal::SlotAllocator &tls,
               const std::chrono::milliseconds &op_timeout)
      : cm_(cm), client_factory_(client_factory), tls_(tls.allocateSlot()),
        config_(op_timeout) {
    tls_->set([this, cluster_name](Event::Dispatcher &dispatcher)
                  -> ThreadLocal::ThreadLocalObjectSharedPtr {
      return std::make_shared<ThreadLocalPool>(*this, dispatcher, cluster_name);
    });
  }

  // Tcp::ConnPool::Instance
  PoolRequest *makeRequest(const std::string &hash_key, const T &request,
                           PoolCallbacks<T> &callbacks) override {
    return tls_->getTyped<ThreadLocalPool>().makeRequest(hash_key, request,
                                                         callbacks);
  }

private:
  struct ThreadLocalPool;

  struct ThreadLocalActiveClient : public Network::ConnectionCallbacks {
    ThreadLocalActiveClient(ThreadLocalPool &parent) : parent_(parent) {}

    // Network::ConnectionCallbacks
    void onEvent(Network::ConnectionEvent event) override {
      if (event == Network::ConnectionEvent::RemoteClose ||
          event == Network::ConnectionEvent::LocalClose) {
        auto client_to_delete = parent_.client_map_.find(host_);
        ASSERT(client_to_delete != parent_.client_map_.end());
        parent_.dispatcher_.deferredDelete(
            std::move(client_to_delete->second->client_));
        parent_.client_map_.erase(client_to_delete);
      }
    }
    void onAboveWriteBufferHighWatermark() override {}
    void onBelowWriteBufferLowWatermark() override {}

    ThreadLocalPool &parent_;
    Upstream::HostConstSharedPtr host_;
    ClientPtr<T> client_;
  };

  typedef std::unique_ptr<ThreadLocalActiveClient> ThreadLocalActiveClientPtr;

  struct ThreadLocalPool : public ThreadLocal::ThreadLocalObject {
    ThreadLocalPool(InstanceImpl &parent, Event::Dispatcher &dispatcher,
                    const std::string &cluster_name)
        : parent_(parent), dispatcher_(dispatcher),
          cluster_(parent_.cm_.get(cluster_name)) {

      // TODO(mattklein123): Redis is not currently safe for use with CDS. In
      // order to make this work
      //                     we will need to add thread local cluster removal
      //                     callbacks so that we can safely clean things up and
      //                     fail requests.
      ASSERT(!cluster_->info()->addedViaApi());
      local_host_set_member_update_cb_handle_ =
          cluster_->prioritySet().addMemberUpdateCb(
              [this](uint32_t, const std::vector<Upstream::HostSharedPtr> &,
                     const std::vector<Upstream::HostSharedPtr> &hosts_removed)
                  -> void { onHostsRemoved(hosts_removed); });
    }
    ~ThreadLocalPool() {
      local_host_set_member_update_cb_handle_->remove();
      while (!client_map_.empty()) {
        client_map_.begin()->second->client_->close();
      }
    }
    PoolRequest *makeRequest(const std::string &hash_key, const T &request,
                             PoolCallbacks<T> &callbacks) {
      LbContextImpl lb_context(hash_key);
      Upstream::HostConstSharedPtr host =
          cluster_->loadBalancer().chooseHost(&lb_context);
      if (!host) {
        return nullptr;
      }

      ThreadLocalActiveClientPtr &client = client_map_[host];
      if (!client) {
        client.reset(new ThreadLocalActiveClient(*this));
        client->host_ = host;
        client->client_ =
            parent_.client_factory_.create(host, dispatcher_, parent_.config_);
        client->client_->addConnectionCallbacks(*client);
      }

      return client->client_->makeRequest(request, callbacks);
    }
    void
    onHostsRemoved(const std::vector<Upstream::HostSharedPtr> &hosts_removed) {
      for (const auto &host : hosts_removed) {
        auto it = client_map_.find(host);
        if (it != client_map_.end()) {
          // We don't currently support any type of draining for
          // connections. If a host is gone, we just close the connection. This
          // will fail any pending requests.
          it->second->client_->close();
        }
      }
    }

    InstanceImpl &parent_;
    Event::Dispatcher &dispatcher_;
    Upstream::ThreadLocalCluster *cluster_;
    std::unordered_map<Upstream::HostConstSharedPtr, ThreadLocalActiveClientPtr>
        client_map_;
    Common::CallbackHandle *local_host_set_member_update_cb_handle_;
  };

  struct LbContextImpl : public Upstream::LoadBalancerContext {
    LbContextImpl(const std::string &hash_key)
        : hash_key_(std::hash<std::string>()(hash_key)) {}
    // TODO(danielhochman): convert to HashUtil::xxHash64 when we have a
    // migration strategy. Upstream::LoadBalancerContext
    Optional<uint64_t> computeHashKey() override { return hash_key_; }
    const Router::MetadataMatchCriteria *
    metadataMatchCriteria() const override {
      return nullptr;
    }
    const Network::Connection *downstreamConnection() const override {
      return nullptr;
    }

    const Optional<uint64_t> hash_key_;
  };

  Upstream::ClusterManager &cm_;
  ClientFactory<T> &client_factory_;
  ThreadLocal::SlotPtr tls_;
  ConfigImpl config_;
};

template <typename T, typename D> class ManagerImpl : public Manager<T> {
public:
  ManagerImpl(Upstream::ClusterManager &cm, ClientFactory<T> &client_factory,
              ThreadLocal::SlotAllocator &tls,
              const std::chrono::milliseconds &op_timeout)
      : cm_(cm), client_factory_(client_factory), tls_(tls),
        slot_(tls_.allocateSlot()), op_timeout_(op_timeout) {
    slot_->set([this](Event::Dispatcher &dispatcher)
                   -> ThreadLocal::ThreadLocalObjectSharedPtr {
      return std::make_shared<ThreadLocalPoolManager>(*this, dispatcher);
    });
  }

  // Nats::ConnPool::Manager
  Instance<T> &getInstance(const std::string &cluster_name) override {
    return slot_->getTyped<ThreadLocalPoolManager>().getInstance(cluster_name);
  }

private:
  struct ThreadLocalPoolManager : public ThreadLocal::ThreadLocalObject {
    ThreadLocalPoolManager(ManagerImpl &parent, Event::Dispatcher &dispatcher)
        : parent_(parent), dispatcher_(dispatcher) {}

    Instance<T> &getInstance(const std::string &cluster_name) {
      InstancePtr<T> &instance = instance_map_[cluster_name];
      if (!instance) {
        instance.reset(new InstanceImpl<T, D>(
            cluster_name, parent_.cm_, parent_.client_factory_, parent_.tls_,
            parent_.op_timeout_));
      }

      return *instance;
    }

    ManagerImpl &parent_;
    Event::Dispatcher &dispatcher_;
    std::unordered_map<std::string, InstancePtr<T>> instance_map_;
  };

  Upstream::ClusterManager &cm_;
  ClientFactory<T> &client_factory_;
  ThreadLocal::SlotAllocator &tls_;
  ThreadLocal::SlotPtr slot_;
  const std::chrono::milliseconds &op_timeout_;
};

} // namespace ConnPool
} // namespace Tcp
} // namespace Envoy
