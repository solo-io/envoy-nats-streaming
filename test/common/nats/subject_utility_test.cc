#include "common/common/assert.h"
#include "common/nats/subject_utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Nats {

class NatsSubjectUtilityTest : public testing::Test {
public:
  NatsSubjectUtilityTest() {}

protected:
  SubjectUtility subject_utility_{};
};

TEST_F(NatsSubjectUtilityTest, Join) {
  std::string expected_subject{"_STAN.acks.DL0Gdhfh3RvpPyDOLQuLNI"};
  auto actual_subject =
      subject_utility_.join("_STAN.acks", "DL0Gdhfh3RvpPyDOLQuLNI");
  EXPECT_EQ(expected_subject, actual_subject);
}

} // namespace Nats
} // namespace Envoy
