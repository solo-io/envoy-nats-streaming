#pragma once

#include "envoy/tcp/codec.h"

namespace Envoy {
namespace Tcp {

/**
 * A factory implementation that returns a real decoder.
 */
template <typename T, typename D>
class DecoderFactoryImpl : public DecoderFactory<T> {
public:
  // Tcp::DecoderFactory
  DecoderPtr create(DecoderCallbacks<T> &callbacks) override {
    return DecoderPtr{new D(callbacks)};
  }
};

} // namespace Tcp
} // namespace Envoy
