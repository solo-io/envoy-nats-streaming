#include <algorithm>
#include <cctype>

#include "common/common/assert.h"
#include "common/nats/token_generator_impl.h"

#include "test/mocks/runtime/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::NiceMock;
using testing::Return;

namespace Envoy {
namespace Nats {

class NatsTokenGeneratorImplTest : public testing::Test {
public:
  NatsTokenGeneratorImplTest() {}
};

TEST_F(NatsTokenGeneratorImplTest, Random) {
  NiceMock<Runtime::MockRandomGenerator> random_generator;

  for (auto i = 0; i < 1000; ++i) {
    TokenGeneratorImpl token_generator{random_generator};
    for (auto j = 0; j < 10; ++j) {
      auto token = token_generator.random();
      EXPECT_FALSE(token.empty());
      constexpr auto isalnum = [](const char c) { return std::isalnum(c); };
      EXPECT_TRUE(std::all_of(token.begin(), token.end(), isalnum));
    }
  }
}

} // namespace Nats
} // namespace Envoy
