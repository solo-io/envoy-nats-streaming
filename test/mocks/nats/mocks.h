#pragma once

#include "envoy/tcp/conn_pool_nats.h"

#include "common/nats/codec_impl.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs);

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

} // namespace ConnPool

} // namespace Nats
} // namespace Envoy
