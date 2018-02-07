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

  PublishRequestPtr makeRequest(PublishCallbacks &callbacks) override {
    return PublishRequestPtr{makeRequest_(callbacks)};
  }

  MOCK_METHOD1(makeRequest_, PublishRequest *(PublishCallbacks &callbacks));
};

} // namespace Publisher

} // namespace Nats
} // namespace Envoy
