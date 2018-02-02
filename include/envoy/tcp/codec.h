#pragma once

#include <memory>
#include <string>

#include "envoy/buffer/buffer.h"
#include "envoy/common/exception.h"
#include "envoy/common/pure.h"

namespace Envoy {
namespace Tcp {

template <typename T> using MessagePtr = std::unique_ptr<T>;

/**
 * Callbacks that the decoder fires.
 */
template <typename T> class DecoderCallbacks {
public:
  virtual ~DecoderCallbacks() {}

  /**
   * Called when a new top level value has been decoded.
   * @param value supplies the decoded value that is now owned by the callee.
   */
  virtual void onValue(MessagePtr<T> &&value) PURE;
};

/**
 * A byte decoder.
 */
class Decoder {
public:
  virtual ~Decoder() {}

  /**
   * Decode protocol bytes.
   * @param data supplies the data to decode. All bytes will be consumed by the
   * decoder or a ProtocolError will be thrown.
   */
  virtual void decode(Buffer::Instance &data) PURE;
};

typedef std::unique_ptr<Decoder> DecoderPtr;

/**
 * A factory for a decoder.
 */
template <typename T> class DecoderFactory {
public:
  virtual ~DecoderFactory() {}

  /**
   * Create a decoder given a set of decoder callbacks.
   */
  virtual DecoderPtr create(DecoderCallbacks<T> &callbacks) PURE;
};

/**
 * A byte encoder.
 */
template <typename T> class Encoder {
public:
  virtual ~Encoder() {}

  /**
   * Encode a value to a buffer.
   * @param value supplies the value to encode.
   * @param out supplies the buffer to encode to.
   */
  virtual void encode(const T &value, Buffer::Instance &out) PURE;
};

template <typename T> using EncoderPtr = std::unique_ptr<Encoder<T>>;

/**
 * A protocol error.
 */
class ProtocolError : public EnvoyException {
public:
  ProtocolError(const std::string &error) : EnvoyException(error) {}
};

} // namespace Tcp
} // namespace Envoy
