#pragma once

#include "envoy/nats/codec.h"
#include "envoy/tcp/codec.h"

#include "common/common/logger.h"

namespace Envoy {
namespace Nats {

using Tcp::Decoder;
using Tcp::DecoderCallbacks;
using Tcp::Encoder;

/**
 * Decoder implementation of
 * https://nats.io/documentation/internals/nats-protocol/
 *
 * This implementation buffers when needed and will always consume all bytes
 * passed for decoding.
 */
class DecoderImpl : public Decoder, Logger::Loggable<Logger::Id::tracing> {
public:
  DecoderImpl(DecoderCallbacks<Message> &callbacks) : callbacks_(callbacks) {}

  // Tcp::Decoder
  void decode(Buffer::Instance &data) override;

private:
  enum class State { ValueRootStart, SimpleString, LF, ValueComplete };

  void parseSlice(const Buffer::RawSlice &slice);

  DecoderCallbacks<Message> &callbacks_;
  State state_{State::ValueRootStart};
  MessagePtr pending_value_root_;
};

/**
 * Encoder implementation of
 * https://nats.io/documentation/internals/nats-protocol/
 */
class EncoderImpl : public Encoder<Message> {
public:
  // Tcp::Encoder
  void encode(const Message &value, Buffer::Instance &out) override;
};

} // namespace Nats
} // namespace Envoy
