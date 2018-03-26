#pragma once

#include <string>

#include "envoy/nats/token_generator.h"
#include "envoy/runtime/runtime.h"

#include "common/nats/nuid/nuid.h"

namespace Envoy {
namespace Nats {

class TokenGeneratorImpl : public TokenGenerator {
public:
  explicit TokenGeneratorImpl(Runtime::RandomGenerator &random_generator);

  // Nats::TokenGenerator
  std::string random() override;

private:
  Nuid::Nuid nuid_;
};

} // namespace Nats
} // namespace Envoy
