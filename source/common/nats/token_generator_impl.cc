#include "common/nats/token_generator_impl.h"

#include <algorithm>

namespace Envoy {
namespace Nats {

TokenGeneratorImpl::TokenGeneratorImpl(
    Runtime::RandomGenerator &random_generator)
    : random_generator_(random_generator) {}

std::string TokenGeneratorImpl::random() {
  // TODO(talnordan): Introduce `RandomGeneratorImpl::nuid()` and use it instead
  // of `uuid()`. See https://github.com/nats-io/nuid.
  std::string str = random_generator_.uuid();

  // NATS tokens must be alphanumeric strings.
  str.erase(std::remove(str.begin(), str.end(), '-'), str.end());

  return str;
}

} // namespace Nats
} // namespace Envoy
