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
};

TEST_F(NatsStreamingMessageUtilityTest, ConnectRequestMessage) {
  const auto message = MessageUtility::createConnectRequestMessage(
      "client_id", "heartbeat_inbox");

  pb::ConnectRequest connect_request;
  connect_request.ParseFromString(message);

  EXPECT_EQ("client_id", connect_request.clientid());
  EXPECT_EQ("heartbeat_inbox", connect_request.heartbeatinbox());
}

TEST_F(NatsStreamingMessageUtilityTest, ConnectResponseMessage) {
  const auto message = MessageUtility::createConnectResponseMessage(
      "pub_prefix", "sub_requests", "unsub_requests", "close_requests");

  pb::ConnectResponse connect_response;
  connect_response.ParseFromString(message);

  EXPECT_EQ("pub_prefix", connect_response.pubprefix());
  EXPECT_EQ("sub_requests", connect_response.subrequests());
  EXPECT_EQ("unsub_requests", connect_response.unsubrequests());
  EXPECT_EQ("close_requests", connect_response.closerequests());
}

TEST_F(NatsStreamingMessageUtilityTest, PubMsgMessage) {
  const std::string client_id{"client1"};
  const std::string uuid{"13581321-dead-beef-b77c-24f6818b6043"};
  const std::string subject{"subject1"};
  const std::string data{"\"d\ra\0t\t \na\v"};
  const auto message =
      MessageUtility::createPubMsgMessage(client_id, uuid, subject, data);

  pb::PubMsg pub_msg;
  pub_msg.ParseFromString(message);

  EXPECT_EQ(client_id, pub_msg.clientid());
  EXPECT_EQ(uuid, pub_msg.guid());
  EXPECT_EQ(subject, pub_msg.subject());
  EXPECT_EQ(data, pub_msg.data());
}

TEST_F(NatsStreamingMessageUtilityTest, PubAckMessage) {
  const std::string uuid{"13581321-dead-beef-b77c-24f6818b6043"};
  const std::string error{"E\"R\rR\0O\t \nR\v"};
  const auto message = MessageUtility::createPubAckMessage(uuid, error);

  pb::PubAck pub_ack;
  pub_ack.ParseFromString(message);

  EXPECT_EQ(uuid, pub_ack.guid());
  EXPECT_EQ(error, pub_ack.error());
}

TEST_F(NatsStreamingMessageUtilityTest, GetPubPrefix) {
  const auto message = MessageUtility::createConnectResponseMessage(
      "pub_prefix", "sub_requests", "unsub_requests", "close_requests");

  const auto pub_prefix = MessageUtility::getPubPrefix(message);

  EXPECT_EQ("pub_prefix", pub_prefix);
}

TEST_F(NatsStreamingMessageUtilityTest, ParsePubAckMessage) {
  const std::string uuid{"13581321-dead-beef-b77c-24f6818b6043"};

  // No error.
  const std::string error{""};
  const auto message = MessageUtility::createPubAckMessage(uuid, error);

  auto &&maybe_result = MessageUtility::parsePubAckMessage(message);

  EXPECT_TRUE(maybe_result.valid());
  EXPECT_EQ(uuid, maybe_result.value().guid());
  EXPECT_TRUE(maybe_result.value().error().empty());
}

TEST_F(NatsStreamingMessageUtilityTest, ParsePubAckMessageWithError) {
  const std::string uuid{"13581321-dead-beef-b77c-24f6818b6043"};
  const std::string error{"Error!"};
  const auto message = MessageUtility::createPubAckMessage(uuid, error);

  auto &&maybe_result = MessageUtility::parsePubAckMessage(message);

  EXPECT_TRUE(maybe_result.valid());
  EXPECT_EQ(uuid, maybe_result.value().guid());
  EXPECT_EQ(error, maybe_result.value().error());
}

TEST_F(NatsStreamingMessageUtilityTest, ParseInvalidPubAckMessage) {
  const std::string invalid_message{"This is not a PubAck message."};
  auto &&maybe_result{MessageUtility::parsePubAckMessage(invalid_message)};
  EXPECT_FALSE(maybe_result.valid());
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
