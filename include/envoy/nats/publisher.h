#pragma once

#include <memory>

#include "envoy/common/pure.h"

#include "common/buffer/buffer_impl.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

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
};

/**
 * A NATS publisher that takes incoming NATS messages and publishes them using a
 * backend connection pool.
 */
class Instance {
public:
  virtual ~Instance() {}

  /**
   * Makes a request.
   * @param cluster_name supplies the the cluster name.
   * @param subject supplies the subject.
   * @param payload supplies the fully buffered payload as buffered by this
   * filter or previous ones in the filter chain. May be nullptr if nothing has
   * been buffered.
   * @param callbacks supplies the request completion callbacks.
   * @return PublishRequestPtr a handle to the active request or nullptr if the
   * request could not be made for some reason.
   */
  virtual PublishRequestPtr makeRequest(const std::string &cluster_name,
                                        const std::string &subject,
                                        const Buffer::Instance *payload,
                                        PublishCallbacks &callbacks) PURE;
};

typedef std::shared_ptr<Instance> InstancePtr;

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
