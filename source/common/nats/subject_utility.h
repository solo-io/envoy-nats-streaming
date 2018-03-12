#pragma once

#include <string>

#include "envoy/nats/token_generator.h"

#include "fmt/format.h"

namespace Envoy {
namespace Nats {

class SubjectUtility {
public:
  inline std::string join(const std::string &prefix,
                          const std::string &subject) const {
    return fmt::format("{}.{}", prefix, subject);
  }

  inline std::string randomChild(const std::string &parent,
                                 TokenGenerator &token_generator) const {
    return join(parent, token_generator.random());
  }

  inline std::string childWildcard(const std::string &parent) const {
    return join(parent, "*");
  }
};

} // namespace Nats
} // namespace Envoy
