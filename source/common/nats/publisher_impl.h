#pragma once

#include "envoy/nats/publisher.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

class InstanceImpl : public Instance {
public:
  InstanceImpl();

  // Nats::Publisher::Instance
  PublishRequestPtr makeRequest(const std::string &cluster_name,
                                const std::string &subject,
                                const Buffer::Instance *payload,
                                PublishCallbacks &callbacks) override;
};

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
