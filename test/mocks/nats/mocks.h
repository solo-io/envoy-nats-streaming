#pragma once

#include "envoy/tcp/conn_pool_nats.h"

#include "common/nats/codec_impl.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {

bool operator==(const Message &lhs, const Message &rhs);

namespace ConnPoolNats {

class MockInstance : public Tcp::ConnPoolNats::Instance<Message> {
public:
  MockInstance();
  ~MockInstance();

  MOCK_METHOD1(setPoolCallbacks,
               void(Tcp::ConnPoolNats::PoolCallbacks<Message> &callbacks));

  MOCK_METHOD2(makeRequest,
               void(const std::string &hash_key, const Message &request));
};

} // namespace ConnPoolNats

} // namespace Nats
} // namespace Envoy
