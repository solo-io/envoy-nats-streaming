#include "common/http/filter/nats_streaming_filter.h"
#include "common/http/filter/nats_streaming_filter_config.h"
#include "common/http/filter/subject_retriever.h"

#include "server/config/http/nats_streaming_filter_config_factory.h"

#include "test/mocks/http/filter/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/nats/streaming/mocks.h"
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

// TODO: move to common
class NothingMetadataAccessor : public MetadataAccessor {
public:
  virtual Optional<const std::string *> getFunctionName() const { return {}; }
  virtual Optional<const ProtobufWkt::Struct *> getFunctionSpec() const {
    return {};
  }
  virtual Optional<const ProtobufWkt::Struct *> getClusterMetadata() const {
    return {};
  }
  virtual Optional<const ProtobufWkt::Struct *> getRouteMetadata() const {
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
    nats_streaming_client_.reset(new NiceMock<Nats::Streaming::MockClient>);
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
  std::shared_ptr<NiceMock<Nats::Streaming::MockClient>> nats_streaming_client_;
  std::unique_ptr<NatsStreamingFilter> filter_;
  NiceMock<MockStreamDecoderFilterCallbacks> callbacks_;
};

TEST_F(NatsStreamingFilterTest, NoSubjectHeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should not be called.
  EXPECT_CALL(*nats_streaming_client_, makeRequest(_, _, _, _)).Times(0);

  ASSERT_EQ(false, retreivefunction());
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should not be called.
  EXPECT_CALL(*nats_streaming_client_, makeRequest(_, _, _, _)).Times(0);

  ASSERT_EQ(false, retreivefunction());
}

TEST_F(NatsStreamingFilterTest, NoSubjectRequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should not be called.
  EXPECT_CALL(*nats_streaming_client_, makeRequest(_, _, _, _)).Times(0);

  ASSERT_EQ(false, retreivefunction());
}

TEST_F(NatsStreamingFilterTest, HeaderOnlyRequest) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*nats_streaming_client_,
              makeRequest("fake_cluster", "Subject1", _, Ref(*filter_)))
      .Times(1);

  const std::string subject = "Subject1";
  const std::string cluster_id = "cluster_id";
  const std::string discover_prefix = "discover_prefix1";
  subject_retriever_->subject_ =
      Optional<Subject>(Subject{&subject, &cluster_id, &discover_prefix});

  ASSERT_EQ(true, retreivefunction());

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, true));

  EXPECT_EQ(0, nats_streaming_client_->last_payload_.length());
}

TEST_F(NatsStreamingFilterTest, RequestWithData) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*nats_streaming_client_,
              makeRequest("fake_cluster", "Subject1", _, Ref(*filter_)))
      .Times(1);

  const std::string subject = "Subject1";
  const std::string cluster_id = "cluster_id";
  const std::string discover_prefix = "discover_prefix1";
  subject_retriever_->subject_ =
      Optional<Subject>(Subject{&subject, &cluster_id, &discover_prefix});

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  ASSERT_EQ(true, retreivefunction());

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data2, true));

  const Buffer::OwnedImpl expectedPayload("hello world");

  // TODO(talnordan): Compare buffer content too.
  EXPECT_EQ(expectedPayload.length(),
            nats_streaming_client_->last_payload_.length());
}

TEST_F(NatsStreamingFilterTest, RequestWithTrailers) {
  // `subject_retriever_->getSubject()` should be called.
  EXPECT_CALL(*subject_retriever_, getSubject(_)).Times(1);

  // `nats_streaming_client_->makeRequest()` should be called exactly once.
  EXPECT_CALL(*nats_streaming_client_,
              makeRequest("fake_cluster", "Subject1", _, Ref(*filter_)))
      .Times(1);

  const std::string subject = "Subject1";
  const std::string cluster_id = "cluster_id";
  const std::string discover_prefix = "discover_prefix1";
  subject_retriever_->subject_ =
      Optional<Subject>(Subject{&subject, &cluster_id, &discover_prefix});

  callbacks_.buffer_.reset(new Buffer::OwnedImpl);

  ASSERT_EQ(true, retreivefunction());

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::StopIteration,
            filter_->decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  callbacks_.buffer_->add(data1);
  EXPECT_EQ(FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  callbacks_.buffer_->add(data2);
  EXPECT_EQ(FilterDataStatus::StopIterationNoBuffer,
            filter_->decodeData(data2, false));

  TestHeaderMapImpl trailers;
  EXPECT_EQ(Envoy::Http::FilterTrailersStatus::StopIteration,
            filter_->decodeTrailers(trailers));

  const Buffer::OwnedImpl expectedPayload("hello world");

  // TODO(talnordan): Compare buffer content too.
  EXPECT_EQ(expectedPayload.length(),
            nats_streaming_client_->last_payload_.length());
}

} // namespace Http
} // namespace Envoy
