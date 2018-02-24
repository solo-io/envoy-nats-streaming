#pragma once

#include <string>

#include "envoy/buffer/buffer.h"
#include "envoy/common/exception.h"

namespace Envoy {
namespace Nats {

class Message {
public:
  Message() {}

  explicit Message(const std::string &string) : string_(string) {}

  ~Message() {}

  /**
   * Convert a message to a string for debugging purposes.
   */
  std::string toString() const;

  /**
   * The following are a getter and a setter for the internal value.
   */
  std::string &asString();
  const std::string &asString() const;

private:
  std::string string_;
};

typedef std::unique_ptr<Message> MessagePtr;

/**
 * A NATS protocol error.
 */
class ProtocolError : public EnvoyException {
public:
  ProtocolError(const std::string &error) : EnvoyException(error) {}
};

} // namespace Nats
} // namespace Envoy
