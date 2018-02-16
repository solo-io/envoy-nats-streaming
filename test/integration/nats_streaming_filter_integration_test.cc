#include "common/config/metadata.h"
#include "common/config/solo_well_known_names.h"

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

    config_helper_.addConfigModifier(
        [](envoy::config::filter::network::http_connection_manager::v2::
               HttpConnectionManager &hcm) {
          auto *metadata = hcm.mutable_route_config()
                               ->mutable_virtual_hosts(0)
                               ->mutable_routes(0)
                               ->mutable_metadata();

          Config::Metadata::mutableMetadataValue(
              *metadata, Config::SoloMetadataFilters::get().NATS_STREAMING,
              Config::MetadataNatsStreamingKeys::get().SUBJECT)
              .set_string_value("Subject1");
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

TEST_P(NatsStreamingFilterIntegrationTest, Test1) {
  Envoy::Http::TestHeaderMapImpl headers{
      {":method", "POST"}, {":authority", "www.solo.io"}, {":path", "/"}};

  Envoy::IntegrationStreamDecoderPtr response(
      new Envoy::IntegrationStreamDecoder(*dispatcher_));
  Envoy::FakeStreamPtr request_stream;

  Envoy::Http::StreamEncoder &stream =
      codec_client_->startRequest(headers, *response);
  Envoy::Buffer::OwnedImpl data;
  data.add(std::string("{\"a\":123}"));
  codec_client_->sendData(stream, data, true);

  Envoy::FakeHttpConnectionPtr fake_upstream_connection =
      fake_upstreams_[0]->waitForHttpConnection(*dispatcher_);
  request_stream = fake_upstream_connection->waitForNewStream(*dispatcher_);
  request_stream->waitForEndStream(*dispatcher_);
  response->waitForEndStream();

  // TODO(talnordan)

  codec_client_->close();
}
} // namespace Envoy
