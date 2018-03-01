#pragma once

#include "envoy/tcp/conn_pool.h"

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

namespace ConnPool {

class MockInstance : public Tcp::ConnPool::Instance<Message> {
public:
  MockInstance();
  ~MockInstance();

  MOCK_METHOD1(setPoolCallbacks,
               void(Tcp::ConnPool::PoolCallbacks<Message> &callbacks));

  MOCK_METHOD2(makeRequest,
               void(const std::string &hash_key, const Message &request));
};

class MockManager : public Tcp::ConnPool::Manager<Message> {
public:
  MockManager();
  ~MockManager();

  MOCK_METHOD2(getInstance,
               Tcp::ConnPool::Instance<Message> &(
                   const std::string &cluster_name,
                   Tcp::ConnPool::PoolCallbacks<Message> &callbacks));
};

} // namespace ConnPool

} // namespace Nats
} // namespace Envoy
