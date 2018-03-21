#pragma once

#include "common/nats/streaming/client_impl.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class MockPublishCallbacks : public PublishCallbacks {
public:
  MockPublishCallbacks();
  ~MockPublishCallbacks();

  MOCK_METHOD0(onResponse, void());
  MOCK_METHOD0(onFailure, void());
  MOCK_METHOD0(onTimeout, void());
};

class MockClient : public Client {
public:
  MockClient();
  ~MockClient();

  MOCK_METHOD5(makeRequest,
               PublishRequestPtr(const std::string &, const std::string &,
                                 const std::string &, Buffer::Instance &,
                                 PublishCallbacks &callbacks));

  Buffer::OwnedImpl last_payload_;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
