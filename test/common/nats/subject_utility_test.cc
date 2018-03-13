#include "common/common/assert.h"
#include "common/nats/subject_utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Nats {

class NatsSubjectUtilityTest : public testing::Test {
public:
  NatsSubjectUtilityTest() {}

protected:
  struct MockTokenGenerator : public TokenGenerator {
    explicit MockTokenGenerator(const std::string &token) : token_(token) {}

    std::string random() override { return token_; }
    std::string token_;
  };
};

TEST_F(NatsSubjectUtilityTest, Join) {
  std::string expected_subject{"_STAN.acks.DL0Gdhfh3RvpPyDOLQuLNI"};
  auto actual_subject =
      SubjectUtility::join("_STAN.acks", "DL0Gdhfh3RvpPyDOLQuLNI");
  EXPECT_EQ(expected_subject, actual_subject);
}

TEST_F(NatsSubjectUtilityTest, RandomChild) {
  MockTokenGenerator token_generator{"H39pAjTnENSgSH3HIHnEON"};
  std::string expected_subject{"_INBOX.H39pAjTnENSgSH3HIHnEON"};
  auto actual_subject = SubjectUtility::randomChild("_INBOX", token_generator);
  EXPECT_EQ(expected_subject, actual_subject);
}

TEST_F(NatsSubjectUtilityTest, RandomGrandchild) {
  MockTokenGenerator token_generator{"gSH3HIH1YJ70TA744uhFid"};
  std::string expected_subject{
      "_INBOX.M2kl72gBUTGH12kgXu5c9i.gSH3HIH1YJ70TA744uhFid"};
  auto actual_subject = SubjectUtility::randomChild(
      "_INBOX.M2kl72gBUTGH12kgXu5c9i", token_generator);
  EXPECT_EQ(expected_subject, actual_subject);
}

TEST_F(NatsSubjectUtilityTest, ChildWildcard) {
  std::string expected_subject{"_INBOX.M2kl72gBUTGH12kgXu5c9i.*"};
  auto actual_subject =
      SubjectUtility::childWildcard("_INBOX.M2kl72gBUTGH12kgXu5c9i");
  EXPECT_EQ(expected_subject, actual_subject);
}

} // namespace Nats
} // namespace Envoy
