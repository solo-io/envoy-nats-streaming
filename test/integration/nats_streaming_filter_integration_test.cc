#include "common/config/metadata.h"
#include "common/config/nats_streaming_well_known_names.h"

#include "test/integration/http_integration.h"
#include "test/integration/integration.h"
#include "test/integration/utility.h"

namespace Envoy {

const std::string DEFAULT_NATS_STREAMING_FILTER =
    R"EOF(
name: io.solo.nats_streaming
config:
  op_timeout:
    seconds: 0
    nanos: 20000000
  max_connections: 1
  cluster: cluster_0
)EOF";

class NatsStreamingFilterIntegrationTest
    : public Envoy::HttpIntegrationTest,
      public testing::TestWithParam<Envoy::Network::Address::IpVersion> {
public:
  NatsStreamingFilterIntegrationTest()
      : Envoy::HttpIntegrationTest(Envoy::Http::CodecClient::Type::HTTP1,
                                   GetParam()) {}

  /**
   * Initializer for an individual integration test.
   */
  void initialize() override {

    config_helper_.addFilter(DEFAULT_NATS_STREAMING_FILTER);

    // TODO(talnordan):
    // config_helper_.addConfigModifier(
    //     [](envoy::config::bootstrap::v2::Bootstrap &bootstrap) {
    //       auto &nats_streaming_cluster =
    //           (*bootstrap.mutable_static_resources()->mutable_clusters(0));

    //       auto *metadata = nats_streaming_cluster.mutable_metadata();

    //       Config::Metadata::mutableMetadataValue(
    //           *metadata, Config::SoloMetadataFilters::get().NATS_STREAMING,
    //           Config::MetadataNatsStreamingKeys::get().DISCOVER_PREFIX)
    //           .set_string_value("_STAN.discover");

    //       Config::Metadata::mutableMetadataValue(
    //           *metadata, Config::SoloMetadataFilters::get().NATS_STREAMING,
    //           Config::MetadataNatsStreamingKeys::get().CLUSTER_ID)
    //           .set_string_value("mycluster");
    //     });

    config_helper_.addConfigModifier(
        [](envoy::config::filter::network::http_connection_manager::v2::
               HttpConnectionManager &hcm) {
          auto *metadata = hcm.mutable_route_config()
                               ->mutable_virtual_hosts(0)
                               ->mutable_routes(0)
                               ->mutable_metadata();
          std::string functionalfilter = "io.solo.function_router";
          std::string functionKey = "function";
          std::string clustername =
              hcm.route_config().virtual_hosts(0).routes(0).route().cluster();

          ProtobufWkt::Struct *clusterstruct =
              Config::Metadata::mutableMetadataValue(
                  *metadata, functionalfilter, clustername)
                  .mutable_struct_value();

          (*clusterstruct->mutable_fields())[functionKey].set_string_value(
              "subject");
        });

    HttpIntegrationTest::initialize();

    codec_client_ =
        makeHttpConnection(makeClientConnection((lookupPort("http"))));
  }

  /**
   * Initialize before every test.
   */
  void SetUp() override { initialize(); }
};

INSTANTIATE_TEST_CASE_P(
    IpVersions, NatsStreamingFilterIntegrationTest,
    testing::ValuesIn(Envoy::TestEnvironment::getIpVersionsForTest()));

// TODO(talnordan):
// TEST_P(NatsStreamingFilterIntegrationTest, Test1) {
//   Envoy::Http::TestHeaderMapImpl request_headers{
//       {":method", "POST"}, {":authority", "www.solo.io"}, {":path", "/"}};

//   sendRequestAndWaitForResponse(request_headers, 10,
//   default_response_headers_,
//                                 10);

//   EXPECT_NE(0, upstream_request_->headers()
//                    .get(Envoy::Http::LowerCaseString("authorization"))
//                    ->value()
//                    .size());
// }

TEST_P(NatsStreamingFilterIntegrationTest, Test1) {
  Envoy::Http::TestHeaderMapImpl headers{
      {":method", "POST"}, {":authority", "www.solo.io"}, {":path", "/"}};

  Envoy::FakeStreamPtr request_stream;

  auto encoder_decoder = codec_client_->startRequest(headers);
  Http::StreamEncoder &encoder = encoder_decoder.first;
  Envoy::Buffer::OwnedImpl data;
  data.add(std::string("{\"a\":123}"));
  codec_client_->sendData(encoder, data, true);

  Envoy::FakeHttpConnectionPtr fake_upstream_connection =
      fake_upstreams_[0]->waitForHttpConnection(*dispatcher_);
  request_stream = fake_upstream_connection->waitForNewStream(*dispatcher_);
  request_stream->waitForEndStream(*dispatcher_);
  auto response = std::move(encoder_decoder.second);
  response->waitForEndStream();

  // TODO(talnordan)

  codec_client_->close();
}

} // namespace Envoy
