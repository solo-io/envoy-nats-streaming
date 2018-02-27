#include "common/common/assert.h"
#include "common/nats/streaming/message_builder.h"

#include "test/test_common/utility.h"

#include "protocol.pb.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingMessageBuilderTest : public testing::Test {
public:
  NatsStreamingMessageBuilderTest() {}

protected:
  MessageBuilder message_builder_{};
};

TEST_F(NatsStreamingMessageBuilderTest, ConnectRequestMessage) {
  const auto message = message_builder_.createConnectRequestMessage(
      "client_id", "heartbeat_inbox");

  pb::ConnectRequest connect_request;
  connect_request.ParseFromString(message);

  ASSERT_EQ("client_id", connect_request.clientid());
  ASSERT_EQ("heartbeat_inbox", connect_request.heartbeatinbox());
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
