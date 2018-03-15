#include "common/common/assert.h"
#include "common/nats/streaming/connect_response_handler.h"
#include "common/nats/streaming/message_utility.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

class NatsStreamingConnectResponseHandlerTest : public testing::Test {
public:
  NatsStreamingConnectResponseHandlerTest() {}

protected:
  struct MockCallbacks : ConnectResponseHandler::Callbacks {
    MOCK_METHOD1(onConnected, void(const std::string &pub_prefix));
    MOCK_METHOD1(onFailure, void(const std::string &error));
  };

  MockCallbacks callbacks_;
};

TEST_F(NatsStreamingConnectResponseHandlerTest, NonEmptyReplyTo) {
  Optional<std::string> reply_to{"reply-to"};
  const std::string payload{};

  EXPECT_CALL(
      callbacks_,
      onFailure("incoming ConnectResponse with non-empty reply subject"))
      .Times(1);
  ConnectResponseHandler::onMessage(reply_to, payload, callbacks_);
}

TEST_F(NatsStreamingConnectResponseHandlerTest, NoPayload) {
  Optional<std::string> reply_to{};
  const std::string payload{};

  EXPECT_CALL(callbacks_, onFailure("incoming ConnectResponse without payload"))
      .Times(1);
  ConnectResponseHandler::onMessage(reply_to, payload, callbacks_);
}

TEST_F(NatsStreamingConnectResponseHandlerTest, OnConnected) {
  Optional<std::string> reply_to{};
  const std::string payload{
      MessageUtility::createConnectResponseMessage("pub_prefix_1", "", "", "")};

  EXPECT_CALL(callbacks_, onConnected("pub_prefix_1")).Times(1);
  ConnectResponseHandler::onMessage(reply_to, payload, callbacks_);
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
