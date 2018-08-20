#pragma once

#include <string>

#include "common/config/nats_streaming_well_known_names.h"

#include "extensions/filters/http/common/factory_base.h"

#include "nats_streaming_filter.pb.validate.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Nats {
namespace Streaming {

using Extensions::HttpFilters::Common::FactoryBase;

/**
 * Config registration for the NATS Streaming filter.
 */
class NatsStreamingFilterConfigFactory
    : public FactoryBase<envoy::api::v2::filter::http::NatsStreaming> {
public:
  NatsStreamingFilterConfigFactory()
      : FactoryBase(
            Config::NatsStreamingHttpFilterNames::get().NATS_STREAMING) {}

private:
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoy::api::v2::filter::http::NatsStreaming &proto_config,
      const std::string &stats_prefix,
      Server::Configuration::FactoryContext &context) override;
};

} // namespace Streaming
} // namespace Nats
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
