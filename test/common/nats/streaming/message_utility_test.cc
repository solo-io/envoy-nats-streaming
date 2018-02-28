#include "common/common/assert.h"
#include "common/nats/streaming/message_utility.h"

#include "test/test_common/utility.h"

#include "protocol.pb.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingMessageUtilityTest : public testing::Test {
public:
  NatsStreamingMessageUtilityTest() {}

protected:
  MessageUtility message_utility_{};
};

TEST_F(NatsStreamingMessageUtilityTest, ConnectRequestMessage) {
  const auto message = message_utility_.createConnectRequestMessage(
      "client_id", "heartbeat_inbox");

  pb::ConnectRequest connect_request;
  connect_request.ParseFromString(message);

  ASSERT_EQ("client_id", connect_request.clientid());
  ASSERT_EQ("heartbeat_inbox", connect_request.heartbeatinbox());
}

TEST_F(NatsStreamingMessageUtilityTest, ConnectResponseMessage) {
  const auto message = message_utility_.createConnectResponseMessage(
      "pub_prefix", "sub_requests", "unsub_requests", "close_requests");

  pb::ConnectResponse connect_response;
  connect_response.ParseFromString(message);

  ASSERT_EQ("pub_prefix", connect_response.pubprefix());
  ASSERT_EQ("sub_requests", connect_response.subrequests());
  ASSERT_EQ("unsub_requests", connect_response.unsubrequests());
  ASSERT_EQ("close_requests", connect_response.closerequests());
}

TEST_F(NatsStreamingMessageUtilityTest, GetPubPrefix) {
  const auto message = message_utility_.createConnectResponseMessage(
      "pub_prefix", "sub_requests", "unsub_requests", "close_requests");

  const auto pub_prefix = message_utility_.getPubPrefix(message);

  ASSERT_EQ("pub_prefix", pub_prefix);
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
