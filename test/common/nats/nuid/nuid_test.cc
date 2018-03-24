#include <climits>

#include "common/nats/nuid/nuid.h"
#include "common/runtime/runtime_impl.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::NiceMock;

namespace Envoy {
namespace Nats {
namespace Nuid {

/**
 * See https://github.com/nats-io/nuid/blob/master/nuid_test.go
 */
class NuidTest : public testing::Test {
protected:
  Runtime::RandomGeneratorImpl random_generator_;
};

TEST_F(NuidTest, Digits) { EXPECT_EQ(Nuid::BASE, strlen(Nuid::DIGITS)); }

TEST_F(NuidTest, Rollover) {
  Nuid nuid(random_generator_, Nuid::MAX_SEQ - 1);
  std::string old_pre = nuid.pre();
  nuid.next();
  EXPECT_NE(old_pre, nuid.pre());
}

TEST_F(NuidTest, Length) {
  Nuid nuid(random_generator_);
  EXPECT_EQ(Nuid::TOTAL_LEN, nuid.next().length());
}

TEST_F(NuidTest, ProperPrefix) {
  auto min = CHAR_MAX;
  auto max = CHAR_MIN;
  for (auto i = 0; i < Nuid::BASE; ++i) {
    if (Nuid::DIGITS[i] < min) {
      min = Nuid::DIGITS[i];
    }
    if (Nuid::DIGITS[i] > max) {
      max = Nuid::DIGITS[i];
    }
  }
  EXPECT_EQ('0', min);
  EXPECT_EQ('z', max);
  auto total = 100000;
  for (auto i = 0; i < total; ++i) {
    Nuid n(random_generator_);
    auto pre = n.pre();
    for (auto j = 0; j < Nuid::PRE_LEN; ++j) {
      EXPECT_GE(pre[j], min);
      EXPECT_LE(pre[j], max);
    }
  }
}

} // namespace Nuid
} // namespace Nats
} // namespace Envoy
