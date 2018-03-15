#include "common/common/assert.h"
#include "common/nats/streaming/message_utility.h"
#include "common/nats/streaming/pub_ack_handler.h"

#include "test/mocks/nats/streaming/mocks.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingPubAckHandlerTest : public testing::Test {
public:
  NatsStreamingPubAckHandlerTest() {}

protected:
  class MockInboxCallbacks : public InboxCallbacks {
  public:
    MOCK_METHOD1(onFailure, void(const std::string &error));
  };

  MockInboxCallbacks inbox_callbacks_;
  MockPublishCallbacks publish_callbacks_;
};

TEST_F(NatsStreamingPubAckHandlerTest, NonEmptyReplyTo) {
  Optional<std::string> reply_to{"reply-to"};
  const std::string payload{};

  EXPECT_CALL(inbox_callbacks_,
              onFailure("incoming PubAck with non-empty reply subject"))
      .Times(1);
  PubAckHandler::onMessage(reply_to, payload, inbox_callbacks_,
                           publish_callbacks_);
}

TEST_F(NatsStreamingPubAckHandlerTest, NoPayload) {
  Optional<std::string> reply_to{};
  const std::string payload{};

  EXPECT_CALL(inbox_callbacks_, onFailure("incoming PubAck without payload"))
      .Times(1);
  PubAckHandler::onMessage(reply_to, payload, inbox_callbacks_,
                           publish_callbacks_);
}

TEST_F(NatsStreamingPubAckHandlerTest, Error) {
  Optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{"error1"};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};

  EXPECT_CALL(publish_callbacks_, onFailure()).Times(1);
  PubAckHandler::onMessage(reply_to, payload, inbox_callbacks_,
                           publish_callbacks_);
}

TEST_F(NatsStreamingPubAckHandlerTest, NoError) {
  Optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};

  EXPECT_CALL(publish_callbacks_, onResponse()).Times(1);
  PubAckHandler::onMessage(reply_to, payload, inbox_callbacks_,
                           publish_callbacks_);
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
