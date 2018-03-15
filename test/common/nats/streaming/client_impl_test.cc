#include "common/nats/streaming/client_impl.h"

#include "test/mocks/nats/mocks.h"
#include "test/mocks/nats/streaming/mocks.h"
#include "test/mocks/runtime/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Return;

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingClientImplTest : public testing::Test {
public:
  Nats::ConnPool::MockInstance *conn_pool_{new Nats::ConnPool::MockInstance()};
  Runtime::MockRandomGenerator random_;
  MockPublishCallbacks callbacks_;
  PublishRequestPtr handle_;
};

TEST_F(NatsStreamingClientImplTest, Empty) {
  // TODO(talnordan): This is a dummy test.
  EXPECT_CALL(random_, uuid())
      .Times(4)
      .WillRepeatedly(Return("a121e9e1-feae-4136-9e0e-6fac343d56c9"));
  ClientImpl client{Tcp::ConnPool::InstancePtr<Message>{conn_pool_}, random_};
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
