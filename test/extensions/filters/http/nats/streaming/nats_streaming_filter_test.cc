#include "extensions/filters/http/nats/streaming/nats_streaming_filter.h"
#include "extensions/filters/http/nats/streaming/nats_streaming_filter_config.h"
#include "extensions/filters/http/nats/streaming/nats_streaming_filter_config_factory.h"
#include "extensions/filters/http/nats/streaming/subject_retriever.h"

#include "test/extensions/filters/http/nats/streaming/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/nats/streaming/mocks.h"
#include "test/mocks/server/mocks.h"
#include "test/mocks/upstream/mocks.h"

#include "./nats_streaming_filter.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::NiceMock;
using testing::Ref;

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

// TODO: move to common
class NothingMetadataAccessor : public Http::MetadataAccessor {
public:
  virtual absl::optional<const std::string *> getFunctionName() const {
    return {};
  }
  virtual absl::optional<const ProtobufWkt::Struct *> getFunctionSpec() const {
    return {};
  }
  virtual absl::optional<const ProtobufWkt::Struct *>
  getClusterMetadata() const {
    return {};
  }
  virtual absl::optional<const ProtobufWkt::Struct *> getRouteMetadata() const {
    return {};
  }

  virtual ~NothingMetadataAccessor() {}
};

class NatsStreamingFilterTest : public testing::Test {
public:
  NatsStreamingFilterTest() {}

protected:
  void SetUp() override {

    envoy::api::v2::filter::http::NatsStreaming proto_config;
    proto_config.mutable_op_timeout()->set_nanos(17 * 1000000);
    proto_config.set_max_connections(1);
    proto_config.set_cluster("cluster");

    config_.reset(new NatsStreamingFilterConfig(
        proto_config, factory_context_.clusterManager()));
    subject_retriever_.reset(new NiceMock<MockSubjectRetriever>);
    nats_streaming_client_.reset(
        new NiceMock<Envoy::Nats::Streaming::MockClient>);
    filter_.reset(new NatsStreamingFilter(config_, subject_retriever_,
                                          nats_streaming_client_));
    filter_->setDecoderFilterCallbacks(callbacks_);
  }

  bool retreivefunction() {
    return filter_->retrieveFunction(NothingMetadataAccessor());
  }

  NiceMock<Envoy::Server::Configuration::MockFactoryContext> factory_context_;
  NatsStreamingFilterConfigSharedPtr config_;
  std::shared_ptr<NiceMock<MockSubjectRetriever>> subject_retriever_;
  std::shared_ptr<NiceMock<Envoy::Nats::Streaming::MockClient>>
      nats_streaming_client_;
  std::unique_ptr<NatsStreamingFilter> filter_;
  NiceMock<Http::MockStreamDecoderFilterCallbacks> callbacks_;
};

TEST_F(NatsStreamingFilterTest, NoSubjectHeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should not be called.
  EXPECT_CALL(*nats_streaming_client_, makeRequest_(_, _, _, _, _)).Times(0);

  ASSERT_EQ(false, retreivefunction());
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should not be called.
  EXPECT_CALL(*nats_streaming_client_, makeRequest_(_, _, _, _, _)).Times(0);

  ASSERT_EQ(false, retreivefunction());
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should not be called.
  EXPECT_CALL(*nats_streaming_client_, makeRequest_(_, _, _, _, _)).Times(0);

  ASSERT_EQ(false, retreivefunction());
}

TEST_F(NatsStreamingFilterTest, HeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*nats_streaming_client_,
              makeRequest_("Subject1", "cluster_id", "discover_prefix1", _,
                           Ref(*filter_)))
      .Times(1);

  const std::string subject = "Subject1";
  const std::string cluster_id = "cluster_id";
  const std::string discover_prefix = "discover_prefix1";
  subject_retriever_->subject_ =
      absl::optional<Subject>(Subject{&subject, &cluster_id, &discover_prefix});

  ASSERT_EQ(true, retreivefunction());

  Http::TestHeaderMapImpl headers;
  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, true));

  EXPECT_TRUE(nats_streaming_client_->last_payload_.empty());
}

TEST_F(NatsStreamingFilterTest, RequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*nats_streaming_client_,
              makeRequest_("Subject1", "cluster_id", "discover_prefix1", _,
                           Ref(*filter_)))
      .Times(1);

  const std::string subject = "Subject1";
  const std::string cluster_id = "cluster_id";
  const std::string discover_prefix = "discover_prefix1";
  subject_retriever_->subject_ =
      absl::optional<Subject>(Subject{&subject, &cluster_id, &discover_prefix});

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  ASSERT_EQ(true, retreivefunction());

  Http::TestHeaderMapImpl headers;
  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data2, true));

  pb::Payload actual_payload;
  EXPECT_TRUE(
      actual_payload.ParseFromString(nats_streaming_client_->last_payload_));
  EXPECT_TRUE(actual_payload.headers().empty());
  EXPECT_EQ("hello world", actual_payload.body());
}

TEST_F(NatsStreamingFilterTest, RequestWithHeadersAndOneChunkOfData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*nats_streaming_client_,
              makeRequest_("Subject1", "cluster_id", "discover_prefix1", _,
                           Ref(*filter_)))
      .Times(1);

  const std::string subject = "Subject1";
  const std::string cluster_id = "cluster_id";
  const std::string discover_prefix = "discover_prefix1";
  subject_retriever_->subject_ =
      absl::optional<Subject>(Subject{&subject, &cluster_id, &discover_prefix});

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  ASSERT_EQ(true, retreivefunction());

  Http::TestHeaderMapImpl headers{{"some-header", "a"}, {"other-header", "b"}};
  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data("hello world");
  callbacks_.buffer_->add(data);
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data, true));

  pb::Payload actual_payload;
  EXPECT_TRUE(
      actual_payload.ParseFromString(nats_streaming_client_->last_payload_));
  EXPECT_EQ("a", actual_payload.headers().at("some-header"));
  EXPECT_EQ("b", actual_payload.headers().at("other-header"));
  EXPECT_EQ("hello world", actual_payload.body());
}

TEST_F(NatsStreamingFilterTest, RequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*nats_streaming_client_,
              makeRequest_("Subject1", "cluster_id", "discover_prefix1", _,
                           Ref(*filter_)))
      .Times(1);

  const std::string subject = "Subject1";
  const std::string cluster_id = "cluster_id";
  const std::string discover_prefix = "discover_prefix1";
  subject_retriever_->subject_ =
      absl::optional<Subject>(Subject{&subject, &cluster_id, &discover_prefix});

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  ASSERT_EQ(true, retreivefunction());

  Http::TestHeaderMapImpl headers;
  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(Http::FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data2, false));

  Http::TestHeaderMapImpl trailers;
  EXPECT_EQ(Envoy::Http::FilterTrailersStatus::StopIteration,
            filter_->decodeTrailers(trailers));

  pb::Payload actual_payload;
  EXPECT_TRUE(
      actual_payload.ParseFromString(nats_streaming_client_->last_payload_));
  EXPECT_TRUE(actual_payload.headers().empty());
  EXPECT_EQ("hello world", actual_payload.body());
}

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
