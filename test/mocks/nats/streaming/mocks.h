#pragma once

#include "common/nats/streaming/publisher_impl.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {
namespace Publisher {

class MockPublishCallbacks : public PublishCallbacks {
public:
  MockPublishCallbacks();
  ~MockPublishCallbacks();

  MOCK_METHOD0(onResponse, void());
  MOCK_METHOD0(onFailure, void());
};

class MockInstance : public Instance {
public:
  MockInstance();
  ~MockInstance();

  MOCK_METHOD4(makeRequest,
               PublishRequestPtr(const std::string &, const std::string &,
                                 Buffer::Instance &,
                                 PublishCallbacks &callbacks));

  Buffer::OwnedImpl last_payload_;
};

} // namespace Publisher
} // namespace Nats
} // namespace Envoy
