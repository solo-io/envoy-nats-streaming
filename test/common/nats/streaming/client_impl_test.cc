#include "common/nats/streaming/client_impl.h"

#include "test/mocks/nats/mocks.h"
#include "test/mocks/nats/streaming/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingClientImplTest : public testing::Test {
public:
  Nats::ConnPool::MockInstance *conn_pool_{new Nats::ConnPool::MockInstance()};
  ClientImpl Client_{Tcp::ConnPool::InstancePtr<Message>{conn_pool_}};
  MockPublishCallbacks callbacks_;
  PublishRequestPtr handle_;
};

TEST_F(NatsStreamingClientImplTest, Empty) {
  // TODO(talnordan): This is a dummy test.
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
