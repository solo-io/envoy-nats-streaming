#pragma once

#include <string>

#include "envoy/server/filter_config.h"

#include "nats_streaming_filter.pb.h"

namespace Envoy {
namespace Server {
namespace Configuration {

class NatsStreamingFilterConfigFactory : public NamedHttpFilterConfigFactory {
public:
  HttpFilterFactoryCb createFilterFactory(const Json::Object &config,
                                          const std::string &stat_prefix,
                                          FactoryContext &context) override;

  HttpFilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message &config,
                               const std::string &stat_prefix,
                               FactoryContext &context) override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override;

  std::string name() override;

private:
  HttpFilterFactoryCb
  createFilter(const envoy::api::v2::filter::http::NatsStreaming &proto_config,
               FactoryContext &context);

  static const std::string NATS_STREAMING_HTTP_FILTER_SCHEMA;
};

} // namespace Configuration
} // namespace Server
} // namespace Envoy
