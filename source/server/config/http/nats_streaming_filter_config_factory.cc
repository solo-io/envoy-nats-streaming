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
#include "common/nats/streaming/client_impl.h"
#include "common/protobuf/utility.h"
#include "common/tcp/conn_pool_impl.h"

#include "nats_streaming_filter.pb.validate.h"

namespace Envoy {
namespace Server {
namespace Configuration {

typedef Http::FunctionalFilterMixin<Http::NatsStreamingFilter>
    MixedNatsStreamingFilter;

HttpFilterFactoryCb NatsStreamingFilterConfigFactory::createFilterFactory(
    const Json::Object &, const std::string &, FactoryContext &) {
  NOT_IMPLEMENTED;
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
      MessageUtil::downcastAndValidate<
          const envoy::api::v2::filter::http::NatsStreaming &>(config),
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

HttpFilterFactoryCb NatsStreamingFilterConfigFactory::createFilter(
    const envoy::api::v2::filter::http::NatsStreaming &proto_config,
    FactoryContext &context) {

  Http::NatsStreamingFilterConfigSharedPtr config =
      std::make_shared<Http::NatsStreamingFilterConfig>(
          Http::NatsStreamingFilterConfig(proto_config,
                                          context.clusterManager()));

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

  // TODO(talnordan): Avoid using hard-coded string literals.
  Tcp::ConnPool::InstancePtr<Nats::Message> conn_pool(
      new Tcp::ConnPool::InstanceImpl<Nats::Message, Nats::DecoderImpl>(
          "cluster_0", context.clusterManager(), client_factory,
          context.threadLocal()));

  Nats::Streaming::ClientPtr nats_streaming_client =
      std::make_shared<Nats::Streaming::ClientImpl>(std::move(conn_pool),
                                                    context.random());

  return [&context, config, subjectRetriever, nats_streaming_client](
             Envoy::Http::FilterChainFactoryCallbacks &callbacks) -> void {
    auto filter = new MixedNatsStreamingFilter(
        context, Config::SoloMetadataFilters::get().NATS_STREAMING, config,
        subjectRetriever, nats_streaming_client);
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
