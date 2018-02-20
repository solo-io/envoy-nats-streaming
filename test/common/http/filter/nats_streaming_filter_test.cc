#include "common/http/filter/nats_streaming_filter.h"
#include "common/http/filter/nats_streaming_filter_config.h"
#include "common/http/filter/subject_retriever.h"

#include "server/config/http/nats_streaming_filter_config_factory.h"

#include "test/mocks/http/filter/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/nats/mocks.h"
#include "test/mocks/server/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "nats_streaming_filter.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::NiceMock;
using testing::Ref;
using testing::_;

namespace Envoy {
namespace Http {

class NatsStreamingFilterTest : public testing::Test {
public:
  NatsStreamingFilterTest() {}

protected:
  void SetUp() override {
    std::string json = R"EOF(
    {
      "op_timeout_ms" : 17
    }
    )EOF";

    Envoy::Json::ObjectSharedPtr json_config =
        Envoy::Json::Factory::loadFromString(json);
    auto proto_config =
        Server::Configuration::NatsStreamingFilterConfigFactory::
            translateNatsStreamingFilter(*json_config);
    config_.reset(new NatsStreamingFilterConfig(proto_config));
    subject_retriever_.reset(new NiceMock<MockSubjectRetriever>);
    publisher_.reset(new NiceMock<Nats::Publisher::MockInstance>);
    filter_.reset(new NatsStreamingFilter(factory_context_, "doesn't matter",
                                          config_, subject_retriever_,
                                          publisher_));
    filter_->setDecoderFilterCallbacks(callbacks_);
  }

  NiceMock<Envoy::Server::Configuration::MockFactoryContext> factory_context_;
  NatsStreamingFilterConfigSharedPtr config_;
  std::shared_ptr<NiceMock<MockSubjectRetriever>> subject_retriever_;
  std::shared_ptr<NiceMock<Nats::Publisher::MockInstance>> publisher_;
  std::unique_ptr<NatsStreamingFilter> filter_;
  NiceMock<MockStreamDecoderFilterCallbacks> callbacks_;
};

TEST_F(NatsStreamingFilterTest, NoSubjectHeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `publisher_->makeRequest()` should not be called.
  EXPECT_CALL(*publisher_, makeRequest(_, _, _, _)).Times(0);

  ASSERT_EQ(false, filter_->retrieveFunction(*filter_));
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `publisher_->makeRequest()` should not be called.
  EXPECT_CALL(*publisher_, makeRequest(_, _, _, _)).Times(0);

  ASSERT_EQ(false, filter_->retrieveFunction(*filter_));
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `publisher_->makeRequest()` should not be called.
  EXPECT_CALL(*publisher_, makeRequest(_, _, _, _)).Times(0);

  ASSERT_EQ(false, filter_->retrieveFunction(*filter_));
}

TEST_F(NatsStreamingFilterTest, HeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `publisher_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*publisher_,
              makeRequest("fake_cluster", "Subject1", nullptr, Ref(*filter_)))
      .Times(1);

  subject_retriever_->subject_ = Optional<Subject>("Subject1");

  ASSERT_EQ(true, filter_->retrieveFunction(*filter_));

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->functionDecodeHeaders(headers, true));

  ASSERT(!publisher_->last_payload_);
}

TEST_F(NatsStreamingFilterTest, RequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `publisher_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*publisher_,
              makeRequest("fake_cluster", "Subject1", _, Ref(*filter_)))
      .Times(1);

  subject_retriever_->subject_ = Optional<Subject>("Subject1");

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  ASSERT_EQ(true, filter_->retrieveFunction(*filter_));

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->functionDecodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->functionDecodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->functionDecodeData(data2, true));

  const Buffer::OwnedImpl expectedPayload("hello world");

  // TODO(talnordan): Compare buffer content too.
  ASSERT(publisher_->last_payload_);
  ASSERT_EQ(expectedPayload.length(), publisher_->last_payload_->length());
}

TEST_F(NatsStreamingFilterTest, RequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `publisher_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*publisher_,
              makeRequest("fake_cluster", "Subject1", _, Ref(*filter_)))
      .Times(1);

  subject_retriever_->subject_ = Optional<Subject>("Subject1");

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  ASSERT_EQ(true, filter_->retrieveFunction(*filter_));

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->functionDecodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->functionDecodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->functionDecodeData(data2, false));

  TestHeaderMapImpl trailers;
  EXPECT_EQ(Envoy::Http::FilterTrailersStatus::StopIteration,
            filter_->functionDecodeTrailers(trailers));

  const Buffer::OwnedImpl expectedPayload("hello world");

  // TODO(talnordan): Compare buffer content too.
  ASSERT(publisher_->last_payload_);
  ASSERT_EQ(expectedPayload.length(), publisher_->last_payload_->length());
}

} // namespace Http
} // namespace Envoy
