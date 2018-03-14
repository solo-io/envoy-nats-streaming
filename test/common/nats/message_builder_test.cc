#include "common/common/assert.h"
#include "common/nats/message_builder.h"

#include "test/mocks/nats/mocks.h"
#include "test/test_common/printers.h"
#include "test/test_common/utility.h"

namespace Envoy {
namespace Nats {

class NatsMessageBuilderTest : public testing::Test {
public:
  NatsMessageBuilderTest() {}
};

TEST_F(NatsMessageBuilderTest, ConnectMessage) {
  Message expected_message{
      R"(CONNECT {"verbose":false,"pedantic":false,"tls_required":false,"name":"","lang":"cpp","version":"1.2.2","protocol":1})"};
  auto actual_message = MessageBuilder::createConnectMessage();
  ASSERT_EQ(expected_message, actual_message);
}

TEST_F(NatsMessageBuilderTest, PubMessage) {
  Message expected_message{"PUB subject1 0\r\n"};
  auto actual_message = MessageBuilder::createPubMessage("subject1");
  ASSERT_EQ(expected_message, actual_message);
}

TEST_F(NatsMessageBuilderTest, SubMessage) {
  Message expected_message{"SUB subject1 6"};
  auto actual_message = MessageBuilder::createSubMessage("subject1", 6);
  ASSERT_EQ(expected_message, actual_message);
}

} // namespace Nats
} // namespace Envoy
