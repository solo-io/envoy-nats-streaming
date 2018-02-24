#include "server/config/http/nats_streaming_filter_config_factory.h"

#include <string>

#include "envoy/registry/registry.h"

#include "common/common/macros.h"
#include "common/config/json_utility.h"
#include "common/config/nats_streaming_well_known_names.h"
#include "common/http/filter/metadata_subject_retriever.h"
#include "common/http/filter/nats_streaming_filter.h"
#include "common/http/filter/nats_streaming_filter_config.h"
#include "common/nats/codec_impl.h"
#include "common/nats/publisher_impl.h"
#include "common/protobuf/utility.h"
#include "common/tcp/conn_pool_impl.h"

#include "nats_streaming_filter.pb.h"

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
  JSON_UTIL_SET_DURATION(json_config, proto_config, op_timeout);
  return proto_config;
}

HttpFilterFactoryCb NatsStreamingFilterConfigFactory::createFilter(
    const envoy::api::v2::filter::http::NatsStreaming &proto_config,
    FactoryContext &context) {

  Http::NatsStreamingFilterConfigSharedPtr config =
      std::make_shared<Http::NatsStreamingFilterConfig>(
          Http::NatsStreamingFilterConfig(proto_config));

  Http::SubjectRetrieverSharedPtr subjectRetriever =
      std::make_shared<Http::MetadataSubjectRetriever>();

  Tcp::ConnPool::ClientFactory<Nats::Message> &client_factory =
      Tcp::ConnPool::ClientFactoryImpl<Nats::Message, Nats::EncoderImpl,
                                       Nats::DecoderImpl>::instance_;

  // TODO(talnordan):
  //   Tcp::ConnPool::ManagerPtr<Nats::Message> conn_pool_manager =
  //   std::make_shared<
  //       Tcp::ConnPool::ManagerImpl<Nats::Message, Nats::DecoderImpl>>(
  //       context.clusterManager(), client_factory, context.threadLocal());

  Tcp::ConnPool::InstancePtr<Nats::Message> conn_pool(
      new Tcp::ConnPool::InstanceImpl<Nats::Message, Nats::DecoderImpl>(
          "cluster_0", context.clusterManager(), client_factory,
          context.threadLocal()));

  Nats::Publisher::InstancePtr publisher =
      std::make_shared<Nats::Publisher::InstanceImpl>(std::move(conn_pool));

  return [&context, config, subjectRetriever, publisher](
             Envoy::Http::FilterChainFactoryCallbacks &callbacks) -> void {
    auto filter = new Http::NatsStreamingFilter(
        context, Config::SoloMetadataFilters::get().NATS_STREAMING, config,
        subjectRetriever, publisher);
    callbacks.addStreamDecoderFilter(
        Http::StreamDecoderFilterSharedPtr{filter});
  };
}

const std::string
    NatsStreamingFilterConfigFactory::NATS_STREAMING_HTTP_FILTER_SCHEMA(R"EOF(
  {
    "$schema": "http://json-schema.org/schema#",
    "type" : "object",
    "properties":{
      "op_timeout_ms" : {
        "type" : "integer",
        "minimum" : 0,
        "exclusiveMinimum" : true
      }
    },
    "required": ["op_timeout_ms"],
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
