#include "extensions/filters/http/nats/streaming/nats_streaming_filter_config_factory.h"

#include "envoy/registry/registry.h"

#include "common/nats/codec_impl.h"
#include "common/nats/streaming/client_pool.h"
#include "common/tcp/conn_pool_impl.h"

#include "extensions/filters/http/nats/streaming/metadata_subject_retriever.h"
#include "extensions/filters/http/nats/streaming/nats_streaming_filter.h"
#include "extensions/filters/http/nats/streaming/nats_streaming_filter_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

typedef Http::FunctionalFilterMixin<NatsStreamingFilter>
    MixedNatsStreamingFilter;

Http::FilterFactoryCb
NatsStreamingFilterConfigFactory::createFilterFactoryFromProtoTyped(
    const envoy::api::v2::filter::http::NatsStreaming &proto_config,
    const std::string &, Server::Configuration::FactoryContext &context) {

  NatsStreamingFilterConfigSharedPtr config =
      std::make_shared<NatsStreamingFilterConfig>(
          NatsStreamingFilterConfig(proto_config, context.clusterManager()));

  SubjectRetrieverSharedPtr subjectRetriever =
      std::make_shared<MetadataSubjectRetriever>();

  Tcp::ConnPoolNats::ClientFactory<Envoy::Nats::Message> &client_factory =
      Tcp::ConnPoolNats::ClientFactoryImpl<Envoy::Nats::Message,
                                           Envoy::Nats::EncoderImpl,
                                           Envoy::Nats::DecoderImpl>::instance_;

  Envoy::Nats::Streaming::ClientPtr nats_streaming_client =
      std::make_shared<Envoy::Nats::Streaming::ClientPool>(
          config->cluster(), context.clusterManager(), client_factory,
          context.threadLocal(), context.random(), config->opTimeout());

  return [&context, config, subjectRetriever, nats_streaming_client](
             Envoy::Http::FilterChainFactoryCallbacks &callbacks) -> void {
    auto filter = new MixedNatsStreamingFilter(
        context, Config::NatsStreamingMetadataFilters::get().NATS_STREAMING,
        config, subjectRetriever, nats_streaming_client);
    callbacks.addStreamDecoderFilter(
        Http::StreamDecoderFilterSharedPtr{filter});
  };
}

/**
 * Static registration for the Nats Streaming filter. @see RegisterFactory.
 */
static Envoy::Registry::RegisterFactory<
    NatsStreamingFilterConfigFactory,
    Envoy::Server::Configuration::NamedHttpFilterConfigFactory>
    register_;

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
