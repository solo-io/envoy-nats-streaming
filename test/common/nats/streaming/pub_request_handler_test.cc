#include "common/common/assert.h"
#include "common/nats/streaming/message_utility.h"
#include "common/nats/streaming/pub_request_handler.h"

#include "test/mocks/nats/streaming/mocks.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingPubRequestHandlerTest : public testing::Test {
public:
  NatsStreamingPubRequestHandlerTest() {}

protected:
  class MockInboxCallbacks : public InboxCallbacks {
  public:
    MOCK_METHOD1(onFailure, void(const std::string &error));
  };

  MockInboxCallbacks inbox_callbacks_;
  MockPublishCallbacks publish_callbacks_;
};

TEST_F(NatsStreamingPubRequestHandlerTest, NonEmptyReplyTo) {
  const Optional<std::string> reply_to{"reply-to"};
  const std::string payload{};

  EXPECT_CALL(inbox_callbacks_,
              onFailure("incoming PubAck with non-empty reply subject"))
      .Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, NoPayload) {
  const Optional<std::string> reply_to{};
  const std::string payload{};

  EXPECT_CALL(inbox_callbacks_, onFailure("incoming PubAck without payload"))
      .Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, Error) {
  const Optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{"error1"};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};

  EXPECT_CALL(publish_callbacks_, onFailure()).Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, NoError) {
  const Optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};

  EXPECT_CALL(publish_callbacks_, onResponse()).Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapNoPayload) {
  const std::string inbox{"inbox1"};
  const Optional<std::string> reply_to{};
  const std::string payload{};
  std::map<std::string, PublishCallbacks *> publish_callbacks_per_inbox{
      {inbox, &publish_callbacks_}};

  EXPECT_CALL(inbox_callbacks_, onFailure("incoming PubAck without payload"))
      .Times(1);
  PubRequestHandler::onMessage(inbox, reply_to, payload, inbox_callbacks_,
                               publish_callbacks_per_inbox);

  EXPECT_EQ(publish_callbacks_per_inbox.end(),
            publish_callbacks_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapError) {
  const std::string inbox{"inbox1"};
  const Optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{"error1"};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};
  std::map<std::string, PublishCallbacks *> publish_callbacks_per_inbox{
      {inbox, &publish_callbacks_}};

  EXPECT_CALL(publish_callbacks_, onFailure()).Times(1);
  PubRequestHandler::onMessage(inbox, reply_to, payload, inbox_callbacks_,
                               publish_callbacks_per_inbox);

  EXPECT_EQ(publish_callbacks_per_inbox.end(),
            publish_callbacks_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapNoError) {
  const std::string inbox{"inbox1"};
  const Optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};
  std::map<std::string, PublishCallbacks *> publish_callbacks_per_inbox{
      {inbox, &publish_callbacks_}};

  EXPECT_CALL(publish_callbacks_, onResponse()).Times(1);
  PubRequestHandler::onMessage(inbox, reply_to, payload, inbox_callbacks_,
                               publish_callbacks_per_inbox);

  EXPECT_EQ(publish_callbacks_per_inbox.end(),
            publish_callbacks_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapMissingInbox) {
  const std::string inbox{"inbox1"};
  const Optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};
  std::map<std::string, PublishCallbacks *> publish_callbacks_per_inbox{
      {inbox, &publish_callbacks_}};

  PubRequestHandler::onMessage("inbox2", reply_to, payload, inbox_callbacks_,
                               publish_callbacks_per_inbox);

  EXPECT_NE(publish_callbacks_per_inbox.end(),
            publish_callbacks_per_inbox.find(inbox));
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
