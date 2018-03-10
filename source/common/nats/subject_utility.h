#pragma once

#include <string>

#include "fmt/format.h"

namespace Envoy {
namespace Nats {

class SubjectUtility {
public:
  inline std::string join(const std::string &prefix,
                          const std::string &subject) const {
    return fmt::format("{}.{}", prefix, subject);
  }
};

} // namespace Nats
} // namespace Envoy
