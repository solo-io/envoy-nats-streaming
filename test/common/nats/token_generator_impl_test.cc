#include "common/common/assert.h"
#include "common/nats/token_generator_impl.h"

#include "test/mocks/runtime/mocks.h"

#include "gtest/gtest.h"

using testing::Return;

namespace Envoy {
namespace Nats {

class NatsTokenGeneratorImplTest : public testing::Test {
public:
  NatsTokenGeneratorImplTest() {}
};

TEST_F(NatsTokenGeneratorImplTest, Random) {
  Runtime::MockRandomGenerator random_generator;
  TokenGeneratorImpl token_generator{random_generator};
  EXPECT_CALL(random_generator, uuid())
      .WillOnce(Return("a121e9e1-feae-4136-9e0e-6fac343d56c9"));
  std::string expected_token{"a121e9e1feae41369e0e6fac343d56c9"};
  auto actual_token = token_generator.random();
  EXPECT_EQ(expected_token, actual_token);
}

} // namespace Nats
} // namespace Envoy
