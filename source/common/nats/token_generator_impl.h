#pragma once

#include <string>

#include "envoy/nats/token_generator.h"
#include "envoy/runtime/runtime.h"

namespace Envoy {
namespace Nats {

class TokenGeneratorImpl : public TokenGenerator {
public:
  explicit TokenGeneratorImpl(Runtime::RandomGenerator &random_generator);

  // Nats::TokenGenerator
  std::string random() override;

private:
  Runtime::RandomGenerator &random_generator_;
};

} // namespace Nats
} // namespace Envoy
