#include "test/mocks/http/filter/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/nats/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "nats_streaming_filter.h"
#include "nats_streaming_filter.pb.h"
#include "nats_streaming_filter_config.h"
#include "subject_retriever.h"
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
        subject_retriever_{new NiceMock<MockSubjectRetriever>},
        filter_(config_, subject_retriever_, cm_, publisher_) {
    filter_.setDecoderFilterCallbacks(callbacks_);
  }

protected:
  envoy::api::v2::filter::http::NatsStreaming proto_config_;
  NatsStreamingFilterConfigSharedPtr config_;
  std::shared_ptr<NiceMock<MockSubjectRetriever>> subject_retriever_;
  NiceMock<Envoy::Upstream::MockClusterManager> cm_;
  std::shared_ptr<NiceMock<Nats::Publisher::MockInstance>> publisher_;
  NatsStreamingFilter filter_;
  NiceMock<MockStreamDecoderFilterCallbacks> callbacks_;
};

TEST_F(NatsStreamingFilterTest, NoSubjectHeaderOnlyRequest) {
  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_.decodeHeaders(headers, true));
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithData) {
  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue,
            filter_.decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data2, true));
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithTrailers) {
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
  subject_retriever_->subject_ = Optional<Subject>("Subject1");

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_.decodeHeaders(headers, true));
}

TEST_F(NatsStreamingFilterTest, RequestWithData) {
  subject_retriever_->subject_ = Optional<Subject>("Subject1");

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
  subject_retriever_->subject_ = Optional<Subject>("Subject1");

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
