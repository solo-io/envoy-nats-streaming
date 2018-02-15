#pragma once

#include "common/nats/codec_impl.h"
#include "common/nats/publisher_impl.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs);

namespace Publisher {

class MockPublishCallbacks : public PublishCallbacks {
public:
  MockPublishCallbacks();
  ~MockPublishCallbacks();

  MOCK_METHOD0(onResponse, void());
};

class MockInstance : public Instance {
public:
  MockInstance();
  ~MockInstance();

  MOCK_METHOD4(makeRequest,
               PublishRequestPtr(const std::string &, const std::string &,
                                 const Buffer::Instance *,
                                 PublishCallbacks &callbacks));

  const Buffer::Instance *last_payload_;
};

} // namespace Publisher

namespace ConnPool {

class MockManager : public Tcp::ConnPool::Manager<Message> {
public:
  MockManager();
  ~MockManager();

  MOCK_METHOD1(getInstance, Tcp::ConnPool::Instance<Message> &(
                                const std::string &cluster_name));
};

} // namespace ConnPool

} // namespace Nats
} // namespace Envoy
