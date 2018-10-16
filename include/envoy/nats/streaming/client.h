#pragma once

#include <memory>

#include "envoy/common/pure.h"

#include "common/buffer/buffer_impl.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

/**
 * A handle to a publish request.
 */
class PublishRequest {
public:
  virtual ~PublishRequest() {}

  /**
   * Cancel the request. No further request callbacks will be called.
   */
  virtual void cancel() PURE;
};

typedef std::unique_ptr<PublishRequest> PublishRequestPtr;

/**
 * Publish request callbacks.
 */
class PublishCallbacks {
public:
  virtual ~PublishCallbacks() {}

  /**
   * Called when the response is ready.
   */
  virtual void onResponse() PURE;

  /**
   * Called when a network/protocol error occurs and there is no response.
   */
  virtual void onFailure() PURE;

  /**
   * Called when a timeout occurs and there is no response.
   */
  virtual void onTimeout() PURE;
};

/**
 * A NATS streaming client that takes requests routed to NATS streaming and
 * publishes them using a backend connection pool.
 */
class Client {
public:
  virtual ~Client() {}

  // TODO(talnordan): Add `ack_prefix`.
  /**
   * Makes a request.
   * @param subject supplies the subject.
   * @param cluster_id supplies the cluster-id with which the NATS Streaming
   * Server was started.
   * @param discover_prefix supplies the prefix subject used to connect to the
   * NATS Streaming server.
   * @param payload supplies the fully buffered payload as buffered by this
   * filter or previous ones in the filter chain.
   * @param callbacks supplies the request completion callbacks.
   * @return PublishRequestPtr a handle to the active request or nullptr if the
   * request could not be made for some reason.
   */
  virtual PublishRequestPtr makeRequest(const std::string &subject,
                                        const std::string &cluster_id,
                                        const std::string &discover_prefix,
                                        std::string &&payload,
                                        PublishCallbacks &callbacks) PURE;
};

typedef std::shared_ptr<Client> ClientPtr;

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
