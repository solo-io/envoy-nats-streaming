#pragma once

#include <string>

#include "envoy/nats/token_generator.h"

#include "fmt/format.h"

namespace Envoy {
namespace Nats {

class SubjectUtility {
public:
  static inline std::string join(const std::string &prefix,
                                 const std::string &subject) {
    // Using `operator+=()` seems to be the most performant approach.
    std::string result{prefix};
    result += '.';
    result += subject;
    return result;
  }

  static inline std::string randomChild(const std::string &parent,
                                        TokenGenerator &token_generator) {
    return join(parent, token_generator.random());
  }

  static inline std::string childWildcard(const std::string &parent) {
    return join(parent, "*");
  }
};

} // namespace Nats
} // namespace Envoy
