#include "server/config/http/nats_streaming_filter_config_factory.h"

#include "envoy/registry/registry.h"

#include "common/http/filter/metadata_subject_retriever.h"
#include "common/http/filter/nats_streaming_filter.h"
#include "common/http/filter/nats_streaming_filter_config.h"
#include "common/nats/codec_impl.h"
#include "common/nats/streaming/client_pool.h"
#include "common/tcp/conn_pool_impl.h"

namespace Envoy {
namespace Server {
namespace Configuration {

typedef Http::FunctionalFilterMixin<Http::NatsStreamingFilter>
    MixedNatsStreamingFilter;

Http::FilterFactoryCb
NatsStreamingFilterConfigFactory::createFilterFactoryFromProtoTyped(
    const envoy::api::v2::filter::http::NatsStreaming &proto_config,
    const std::string &, FactoryContext &context) {

  Http::NatsStreamingFilterConfigSharedPtr config =
      std::make_shared<Http::NatsStreamingFilterConfig>(
          Http::NatsStreamingFilterConfig(proto_config,
                                          context.clusterManager()));

  Http::SubjectRetrieverSharedPtr subjectRetriever =
      std::make_shared<Http::MetadataSubjectRetriever>();

  Tcp::ConnPoolNats::ClientFactory<Nats::Message> &client_factory =
      Tcp::ConnPoolNats::ClientFactoryImpl<Nats::Message, Nats::EncoderImpl,
                                       Nats::DecoderImpl>::instance_;

  Nats::Streaming::ClientPtr nats_streaming_client =
      std::make_shared<Nats::Streaming::ClientPool>(
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

} // namespace Configuration
} // namespace Server
} // namespace Envoy
