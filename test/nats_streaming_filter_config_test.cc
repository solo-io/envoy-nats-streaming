#include "test/test_common/utility.h"

#include "nats_streaming_filter_config.h"
#include "nats_streaming_filter_config_factory.h"

namespace Envoy {

using Http::NatsStreamingFilterConfig;
using Server::Configuration::NatsStreamingFilterConfigFactory;

namespace {

NatsStreamingFilterConfig
constructNatsStrteamingFilterConfigFromJson(const Json::Object &config) {
  auto proto_config =
      NatsStreamingFilterConfigFactory::translateNatsStreamingFilter(config);
  return NatsStreamingFilterConfig(proto_config);
}

} // namespace

TEST(NatsStreamingFilterConfigTest, NoPlaceholder) {
  std::string json = R"EOF(
    {
    }
    )EOF";

  Envoy::Json::ObjectSharedPtr json_config =
      Envoy::Json::Factory::loadFromString(json);

  EXPECT_THROW(NatsStreamingFilterConfigFactory::translateNatsStreamingFilter(
                   *json_config),
               Envoy::EnvoyException);
}

TEST(NatsStreamingFilterConfigTest, Placeholder) {
  std::string json = R"EOF(
    {
      "placeholder" : "a"
    }
    )EOF";

  Envoy::Json::ObjectSharedPtr json_config =
      Envoy::Json::Factory::loadFromString(json);
  auto config = constructNatsStrteamingFilterConfigFromJson(*json_config);

  EXPECT_EQ(config.placeholder(), "a");
}

} // namespace Envoy
