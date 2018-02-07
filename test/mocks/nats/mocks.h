#pragma once

#include "common/nats/codec_impl.h"
#include "common/nats/publisher_impl.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs);

namespace Publisher {

class MockPublishRequest : public PublishRequest {
public:
  MockPublishRequest();
  ~MockPublishRequest();

  MOCK_METHOD0(cancel, void());
};

class MockPublishCallbacks : public PublishCallbacks {
public:
  MockPublishCallbacks();
  ~MockPublishCallbacks();

  void onResponse() override { onResponse_(); }

  MOCK_METHOD0(onResponse_, void());
};

class MockInstance : public Instance {
public:
  MockInstance();
  ~MockInstance();

  PublishRequestPtr makeRequest(const std::string &cluster_name,
                                const std::string &subject,
                                const Buffer::Instance *payload,
                                PublishCallbacks &callbacks) override {
    return PublishRequestPtr{
        makeRequest_(cluster_name, subject, payload, callbacks)};
  }

  MOCK_METHOD4(makeRequest_,
               PublishRequest *(const std::string &, const std::string &,
                                const Buffer::Instance *,
                                PublishCallbacks &callbacks));
};

} // namespace Publisher

} // namespace Nats
} // namespace Envoy
