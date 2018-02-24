#pragma once

#include <memory>

#include "envoy/common/pure.h"
#include "envoy/event/deferred_deletable.h"
#include "envoy/network/connection.h"
#include "envoy/tcp/codec.h"
#include "envoy/upstream/upstream.h"

namespace Envoy {
namespace Tcp {
namespace ConnPool {

/**
 * Outbound request callbacks.
 */
template <typename T> class PoolCallbacks {
public:
  virtual ~PoolCallbacks() {}

  /**
   * Called when a pipelined response is received.
   * @param value supplies the response which is now owned by the callee.
   */
  virtual void onResponse(MessagePtr<T> &&value) PURE;

  /**
   * Called when close event occurs on a connection.
   */
  virtual void onClose() PURE;
};

/**
 * A single client connection.
 */
template <typename T> class Client : public Event::DeferredDeletable {
public:
  virtual ~Client() {}

  /**
   * Adds network connection callbacks to the underlying network connection.
   */
  virtual void
  addConnectionCallbacks(Network::ConnectionCallbacks &callbacks) PURE;

  /**
   * Closes the underlying network connection.
   */
  virtual void close() PURE;

  /**
   * Make a pipelined request to the remote server.
   * @param request supplies the request to make.
   */
  virtual void makeRequest(const T &request) PURE;

  /**
   * Cancel all requests. No further request callbacks will be called.
   */
  virtual void cancel() PURE;
};

template <typename T> using ClientPtr = std::unique_ptr<Client<T>>;

/**
 * Configuration for a connection pool.
 */
class Config {
public:
  virtual ~Config() {}

  /**
   * @return bool disable outlier events even if the cluster has it enabled.
   * This is used by the healthchecker's connection pool to avoid double
   * counting active healthcheck operations as passive healthcheck operations.
   */
  virtual bool disableOutlierEvents() const PURE;
};

/**
 * A factory for individual client connections.
 */
template <typename T> class ClientFactory {
public:
  virtual ~ClientFactory() {}

  /**
   * Create a client given an upstream host.
   * @param host supplies the upstream host.
   * @param dispatcher supplies the owning thread's dispatcher.
   * @param callbacks supplies the pool callbacks.
   * @param config supplies the connection pool configuration.
   * @return ClientPtr a new connection pool client.
   */
  virtual ClientPtr<T> create(Upstream::HostConstSharedPtr host,
                              Event::Dispatcher &dispatcher,
                              PoolCallbacks<T> &callbacks,
                              const Config &config) PURE;
};

/**
 * A connection pool. Wraps M connections to N upstream hosts, consistent
 * hashing, pipelining, failure handling, etc.
 */
template <typename T> class Instance {
public:
  virtual ~Instance() {}

  /**
   * Sets the pool callbacks.
   */
  virtual void setPoolCallbacks(PoolCallbacks<T> &callbacks) PURE;

  /**
   * Makes a request.
   * @param hash_key supplies the key to use for consistent hashing.
   * @param request supplies the request to make.
   */
  virtual void makeRequest(const std::string &hash_key, const T &request) PURE;
};

template <typename T> using InstancePtr = std::unique_ptr<Instance<T>>;

template <typename T> class Manager {
public:
  virtual ~Manager() {}

  virtual Instance<T> &getInstance(const std::string &cluster_name,
                                   PoolCallbacks<T> &callbacks) PURE;
};

template <typename T> using ManagerPtr = std::shared_ptr<Manager<T>>;

} // namespace ConnPool
} // namespace Tcp
} // namespace Envoy
