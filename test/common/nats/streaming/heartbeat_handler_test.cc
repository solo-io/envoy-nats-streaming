#include "common/common/assert.h"
#include "common/nats/message_builder.h"
#include "common/nats/streaming/heartbeat_handler.h"

#include "test/mocks/nats/mocks.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingHeartbeatHandlerTest : public testing::Test {
public:
  NatsStreamingHeartbeatHandlerTest() {}

protected:
  class MockCallbacks : public HeartbeatHandler::Callbacks {
  public:
    MOCK_METHOD1(send, void(const Message &message));
    MOCK_METHOD1(onFailure, void(const std::string &error));
  };

  MockCallbacks callbacks_;
};

TEST_F(NatsStreamingHeartbeatHandlerTest, NoReplyTo) {
  Optional<std::string> reply_to{};
  const std::string payload{};

  EXPECT_CALL(callbacks_, onFailure("incoming heartbeat without reply subject"))
      .Times(1);
  HeartbeatHandler::onMessage(reply_to, payload, callbacks_);
}

TEST_F(NatsStreamingHeartbeatHandlerTest, NonEmptyPayload) {
  Optional<std::string> reply_to{"reply-to"};
  const std::string payload{"payload"};

  EXPECT_CALL(callbacks_,
              onFailure("incoming heartbeat with non-empty payload"))
      .Times(1);
  HeartbeatHandler::onMessage(reply_to, payload, callbacks_);
}

TEST_F(NatsStreamingHeartbeatHandlerTest, Reply) {
  Optional<std::string> reply_to{"reply-to"};
  const std::string payload{};
  Message expected_message = MessageBuilder::createPubMessage(reply_to.value());
  EXPECT_CALL(callbacks_, send(expected_message)).Times(1);
  HeartbeatHandler::onMessage(reply_to, payload, callbacks_);
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
