#pragma once

#include <string>

#include "envoy/common/pure.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class InboxCallbacks {
public:
  virtual ~InboxCallbacks() {}
  virtual void onFailure(const std::string &error) PURE;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
