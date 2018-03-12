#pragma once

#include <string>

#include "envoy/common/pure.h"

namespace Envoy {
namespace Nats {

/**
 * Random token generator.
 */
class TokenGenerator {
public:
  virtual ~TokenGenerator() {}

  /**
   * @return std::string a new random token.
   */
  virtual std::string random() PURE;
};

} // namespace Nats
} // namespace Envoy
