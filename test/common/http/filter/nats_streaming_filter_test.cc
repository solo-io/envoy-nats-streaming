#include "test/mocks/http/filter/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "nats_streaming_filter.h"
#include "nats_streaming_filter.pb.h"
#include "nats_streaming_filter_config.h"
#include "topic_retriever.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::InSequence;
using testing::NiceMock;

namespace Envoy {
namespace Http {

class NatsStreamingFilterTest : public testing::Test {
public:
  NatsStreamingFilterTest()
      : config_{new NatsStreamingFilterConfig(proto_config_)},
        topic_retriever_{new NiceMock<MockTopicRetriever>},
        filter_(config_, topic_retriever_, cm_) {
    filter_.setDecoderFilterCallbacks(callbacks_);
  }

protected:
  envoy::api::v2::filter::http::NatsStreaming proto_config_;
  NatsStreamingFilterConfigSharedPtr config_;
  std::shared_ptr<NiceMock<MockTopicRetriever>> topic_retriever_;
  NiceMock<Envoy::Upstream::MockClusterManager> cm_;
  NatsStreamingFilter filter_;
  NiceMock<MockStreamDecoderFilterCallbacks> callbacks_;
};

TEST_F(NatsStreamingFilterTest, NoTopicHeaderOnlyRequest) {
  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_.decodeHeaders(headers, true));
}

TEST_F(NatsStreamingFilterTest, NoTopicRequestWithData) {
  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_.decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data2, true));
}

TEST_F(NatsStreamingFilterTest, NoTopicRequestWithTrailers) {
  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_.decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data2, false));

  TestHeaderMapImpl trailers;
  EXPECT_EQ(Envoy::Http::FilterTrailersStatus::Continue,
            filter_.decodeTrailers(trailers));
}

TEST_F(NatsStreamingFilterTest, HeaderOnlyRequest) {
  topic_retriever_->topic_ = Optional<Topic>("Topic1");

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_.decodeHeaders(headers, true));
}

TEST_F(NatsStreamingFilterTest, RequestWithData) {
  topic_retriever_->topic_ = Optional<Topic>("Topic1");

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_.decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_.decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::StopIterationNoBuffer,
            filter_.decodeData(data2, true));
}

TEST_F(NatsStreamingFilterTest, RequestWithTrailers) {
  topic_retriever_->topic_ = Optional<Topic>("Topic1");

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_.decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_.decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::StopIterationAndBuffer,
            filter_.decodeData(data2, false));

  TestHeaderMapImpl trailers;
  EXPECT_EQ(Envoy::Http::FilterTrailersStatus::StopIteration,
            filter_.decodeTrailers(trailers));
}

} // namespace Http
} // namespace Envoy
