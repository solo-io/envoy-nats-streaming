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

  PublishRequestPtr makeRequest(const std::string &subject,
                                const std::string &cluster_id,
                                const std::string &discover_prefix,
                                std::string &&payload,
                                PublishCallbacks &callbacks) override;

  MOCK_METHOD5(makeRequest_,
               PublishRequestPtr(const std::string &, const std::string &,
                                 const std::string &, const std::string &,
                                 PublishCallbacks &callbacks));

  std::string last_payload_;
};

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
