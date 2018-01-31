#include "nats_streaming_filter_config_factory.h"

#include <string>

#include "envoy/registry/registry.h"

#include "common/common/macros.h"
#include "common/config/json_utility.h"
#include "common/config/solo_well_known_names.h"
#include "common/protobuf/utility.h"

#include "metadata_topic_retriever.h"
#include "nats_streaming_filter.h"
#include "nats_streaming_filter.pb.h"
#include "nats_streaming_filter_config.h"

namespace Envoy {
namespace Server {
namespace Configuration {

HttpFilterFactoryCb NatsStreamingFilterConfigFactory::createFilterFactory(
    const Json::Object &config, const std::string &stat_prefix,
    FactoryContext &context) {
  UNREFERENCED_PARAMETER(stat_prefix);

  return createFilter(translateNatsStreamingFilter(config), context);
}

HttpFilterFactoryCb
NatsStreamingFilterConfigFactory::createFilterFactoryFromProto(
    const Protobuf::Message &config, const std::string &stat_prefix,
    FactoryContext &context) {
  UNREFERENCED_PARAMETER(stat_prefix);

  /**
   * TODO:
   * The corresponding `.pb.validate.h` for the message is required by
   * Envoy::MessageUtil.
   * @see https://github.com/envoyproxy/envoy/pull/2194
   *
   * #include "nats_streaming_filter.pb.validate.h"
   *
   * return createFilter(
   *    Envoy::MessageUtil::downcastAndValidate<const
   * envoy::api::v2::filter::http::NatsStreaming&>(proto_config), context);
   * */

  return createFilter(
      dynamic_cast<const envoy::api::v2::filter::http::NatsStreaming &>(config),
      context);
}

ProtobufTypes::MessagePtr
NatsStreamingFilterConfigFactory::createEmptyConfigProto() {
  return ProtobufTypes::MessagePtr{
      new envoy::api::v2::filter::http::NatsStreaming()};
}

std::string NatsStreamingFilterConfigFactory::name() {
  return Config::SoloHttpFilterNames::get().NATS_STREAMING;
}

const envoy::api::v2::filter::http::NatsStreaming
NatsStreamingFilterConfigFactory::translateNatsStreamingFilter(
    const Json::Object &json_config) {
  json_config.validateSchema(NATS_STREAMING_HTTP_FILTER_SCHEMA);

  envoy::api::v2::filter::http::NatsStreaming proto_config;
  JSON_UTIL_SET_STRING(json_config, proto_config, placeholder);
  return proto_config;
}

HttpFilterFactoryCb NatsStreamingFilterConfigFactory::createFilter(
    const envoy::api::v2::filter::http::NatsStreaming &proto_config,
    FactoryContext &context) {

  Http::NatsStreamingFilterConfigSharedPtr config =
      std::make_shared<Http::NatsStreamingFilterConfig>(
          Http::NatsStreamingFilterConfig(proto_config));

  Http::TopicRetrieverSharedPtr topicRetriever =
      std::make_shared<Http::MetadataTopicRetriever>(
          Config::SoloMetadataFilters::get().NATS_STREAMING,
          Config::MetadataNatsStreamingKeys::get().TOPIC);

  return [&context, config, topicRetriever](
             Envoy::Http::FilterChainFactoryCallbacks &callbacks) -> void {
    auto filter = new Http::NatsStreamingFilter(config, topicRetriever,
                                                context.clusterManager());
    callbacks.addStreamDecoderFilter(
        Http::StreamDecoderFilterSharedPtr{filter});
  };
}

const std::string
    NatsStreamingFilterConfigFactory::NATS_STREAMING_HTTP_FILTER_SCHEMA(R"EOF(
  {
    "$schema": "http://json-schema.org/schema#",
    "type" : "object",
    "properties" : {
      "placeholder": {
        "type" : "string"
      }
    },
    "required": ["placeholder"],
    "additionalProperties" : false
  }
  )EOF");

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static Envoy::Registry::RegisterFactory<
    NatsStreamingFilterConfigFactory,
    Envoy::Server::Configuration::NamedHttpFilterConfigFactory>
    register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
