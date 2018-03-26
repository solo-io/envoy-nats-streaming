#include "common/nats/token_generator_impl.h"

#include <algorithm>

namespace Envoy {
namespace Nats {

TokenGeneratorImpl::TokenGeneratorImpl(
    Runtime::RandomGenerator &random_generator)
    : nuid_(random_generator) {}

std::string TokenGeneratorImpl::random() { return nuid_.next(); }

} // namespace Nats
} // namespace Envoy
