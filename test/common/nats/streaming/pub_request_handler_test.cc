#include "common/common/assert.h"
#include "common/nats/streaming/message_utility.h"
#include "common/nats/streaming/pub_request_handler.h"

#include "test/mocks/event/mocks.h"
#include "test/mocks/nats/streaming/mocks.h"

#include "gmock/gmock.h"

using testing::NiceMock;

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
  const absl::optional<std::string> reply_to{"reply-to"};
  const std::string payload{};

  EXPECT_CALL(inbox_callbacks_,
              onFailure("incoming PubAck with non-empty reply subject"))
      .Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, NoPayload) {
  const absl::optional<std::string> reply_to{};
  const std::string payload{};

  EXPECT_CALL(inbox_callbacks_, onFailure("incoming PubAck without payload"))
      .Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, Error) {
  const absl::optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{"error1"};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};

  EXPECT_CALL(publish_callbacks_, onFailure()).Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, NoError) {
  const absl::optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};

  EXPECT_CALL(publish_callbacks_, onResponse()).Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, InvalidPayload) {
  const absl::optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{"This is not a PubAck message."};

  EXPECT_CALL(publish_callbacks_, onFailure()).Times(1);
  PubRequestHandler::onMessage(reply_to, payload, inbox_callbacks_,
                               publish_callbacks_);
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapNoPayload) {
  const std::string inbox{"inbox1"};
  const absl::optional<std::string> reply_to{};
  const std::string payload{};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(inbox_callbacks_, onFailure("incoming PubAck without payload"))
      .Times(1);
  PubRequestHandler::onMessage(inbox, reply_to, payload, inbox_callbacks_,
                               request_per_inbox);

  EXPECT_EQ(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapError) {
  const std::string inbox{"inbox1"};
  const absl::optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{"error1"};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(publish_callbacks_, onFailure()).Times(1);
  PubRequestHandler::onMessage(inbox, reply_to, payload, inbox_callbacks_,
                               request_per_inbox);

  EXPECT_EQ(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapInvalidPayload) {
  const std::string inbox{"inbox1"};
  const absl::optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{"This is not a PubAck message."};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(publish_callbacks_, onFailure()).Times(1);
  PubRequestHandler::onMessage(inbox, reply_to, payload, inbox_callbacks_,
                               request_per_inbox);

  EXPECT_EQ(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapNoError) {
  const std::string inbox{"inbox1"};
  const absl::optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(publish_callbacks_, onResponse()).Times(1);
  PubRequestHandler::onMessage(inbox, reply_to, payload, inbox_callbacks_,
                               request_per_inbox);

  EXPECT_EQ(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, MapMissingInbox) {
  const std::string inbox{"inbox1"};
  const absl::optional<std::string> reply_to{};
  const std::string guid{"guid1"};
  const std::string error{};
  const std::string payload{MessageUtility::createPubAckMessage(guid, error)};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  PubRequestHandler::onMessage("inbox2", reply_to, payload, inbox_callbacks_,
                               request_per_inbox);

  EXPECT_NE(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, OnTimeout) {
  const std::string inbox{"inbox1"};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(publish_callbacks_, onTimeout()).Times(1);
  PubRequestHandler::onTimeout(inbox, request_per_inbox);

  EXPECT_EQ(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, OnTimeoutMissingInbox) {
  const std::string inbox{"inbox1"};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(publish_callbacks_, onTimeout()).Times(0);
  PubRequestHandler::onTimeout("inbox2", request_per_inbox);

  EXPECT_NE(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, OnCancel) {
  const std::string inbox{"inbox1"};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(publish_callbacks_, onResponse()).Times(0);
  EXPECT_CALL(publish_callbacks_, onFailure()).Times(0);
  EXPECT_CALL(publish_callbacks_, onTimeout()).Times(0);
  PubRequestHandler::onCancel(inbox, request_per_inbox);

  EXPECT_EQ(request_per_inbox.end(), request_per_inbox.find(inbox));
}

TEST_F(NatsStreamingPubRequestHandlerTest, OnCancelMissingInbox) {
  const std::string inbox{"inbox1"};
  auto timeout_timer = Event::TimerPtr(new NiceMock<Event::MockTimer>);
  std::map<std::string, PubRequest> request_per_inbox;
  PubRequest pub_request{&publish_callbacks_, std::move(timeout_timer)};
  request_per_inbox.emplace(inbox, std::move(pub_request));

  EXPECT_CALL(publish_callbacks_, onResponse()).Times(0);
  EXPECT_CALL(publish_callbacks_, onFailure()).Times(0);
  EXPECT_CALL(publish_callbacks_, onTimeout()).Times(0);
  PubRequestHandler::onCancel("inbox2", request_per_inbox);

  EXPECT_NE(request_per_inbox.end(), request_per_inbox.find(inbox));
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
