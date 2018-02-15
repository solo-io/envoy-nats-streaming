#include "test/mocks/http/filter/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/nats/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "nats_streaming_filter.h"
#include "nats_streaming_filter.pb.h"
#include "nats_streaming_filter_config.h"
#include "nats_streaming_filter_config_factory.h"
#include "subject_retriever.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::NiceMock;
using testing::Ref;
using testing::_;

namespace Envoy {
namespace Http {

class NatsStreamingFilterTest : public testing::Test {
public:
  NatsStreamingFilterTest() {
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
    filter_.reset(
        new NatsStreamingFilter(config_, subject_retriever_, cm_, publisher_));
    filter_->setDecoderFilterCallbacks(callbacks_);
  }

protected:
  envoy::api::v2::filter::http::NatsStreaming proto_config_;
  NatsStreamingFilterConfigSharedPtr config_;
  std::shared_ptr<NiceMock<MockSubjectRetriever>> subject_retriever_;
  NiceMock<Envoy::Upstream::MockClusterManager> cm_;
  std::shared_ptr<NiceMock<Nats::Publisher::MockInstance>> publisher_;
  std::unique_ptr<NatsStreamingFilter> filter_;
  NiceMock<MockStreamDecoderFilterCallbacks> callbacks_;
};

TEST_F(NatsStreamingFilterTest, NoSubjectHeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_, _));

  // `publisher_->makeRequest()` should not be called.
  EXPECT_CALL(*publisher_, makeRequest(_, _, _, _)).Times(0);

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_->decodeHeaders(headers, true));
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_, _));

  // `publisher_->makeRequest()` should not be called.
  EXPECT_CALL(*publisher_, makeRequest(_, _, _, _)).Times(0);

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data2, true));
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_, _));

  // `publisher_->makeRequest()` should not be called.
  EXPECT_CALL(*publisher_, makeRequest(_, _, _, _)).Times(0);

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::Continue, filter_->decodeData(data2, false));

  TestHeaderMapImpl trailers;
  EXPECT_EQ(Envoy::Http::FilterTrailersStatus::Continue,
            filter_->decodeTrailers(trailers));
}

TEST_F(NatsStreamingFilterTest, HeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_, _));

  // `publisher_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*publisher_,
              makeRequest("fake_cluster", "Subject1", nullptr, Ref(*filter_)))
      .Times(1);

  subject_retriever_->subject_ = Optional<Subject>("Subject1");

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, true));

  ASSERT(!publisher_->last_payload_);
}

TEST_F(NatsStreamingFilterTest, RequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_, _));

  // `publisher_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*publisher_,
              makeRequest("fake_cluster", "Subject1", _, Ref(*filter_)))
      .Times(1);

  subject_retriever_->subject_ = Optional<Subject>("Subject1");

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->decodeData(data2, true));

  const Buffer::OwnedImpl expectedPayload("hello world");

  // TODO(talnordan): Compare buffer content too.
  ASSERT(publisher_->last_payload_);
  ASSERT_EQ(expectedPayload.length(), publisher_->last_payload_->length());
}

TEST_F(NatsStreamingFilterTest, RequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_, _));

  // `publisher_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*publisher_,
              makeRequest("fake_cluster", "Subject1", _, Ref(*filter_)))
      .Times(1);

  subject_retriever_->subject_ = Optional<Subject>("Subject1");

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_->decodeData(data2, false));

  TestHeaderMapImpl trailers;
  EXPECT_EQ(Envoy::Http::FilterTrailersStatus::StopIteration,
            filter_->decodeTrailers(trailers));

  const Buffer::OwnedImpl expectedPayload("hello world");

  // TODO(talnordan): Compare buffer content too.
  ASSERT(publisher_->last_payload_);
  ASSERT_EQ(expectedPayload.length(), publisher_->last_payload_->length());
}

} // namespace Http
} // namespace Envoy
