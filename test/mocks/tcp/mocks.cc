#include "mocks.h"

#include "common/common/assert.h"
#include "common/common/macros.h"

#include "test/test_common/printers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Invoke;
using testing::_;

namespace Envoy {
namespace Tcp {

MockEncoder::MockEncoder() {
  ON_CALL(*this, encode(_, _))
      .WillByDefault(Invoke(
          [this](const std::string &value, Buffer::Instance &out) -> void {
            encodeSimpleString(value, out);
          }));
}

MockEncoder::~MockEncoder() {}

void MockEncoder::encodeSimpleString(const std::string &string,
                                     Buffer::Instance &out) {
  out.add("+", 1);
  out.add(string);
  out.add("\r\n", 2);
}

MockDecoder::MockDecoder() {}

MockDecoder::MockDecoder(DecoderCallbacks<T> &callbacks) {
  UNREFERENCED_PARAMETER(callbacks);
}

MockDecoder::~MockDecoder() {}

namespace ConnPool {

MockClient::MockClient() {
  ON_CALL(*this, addConnectionCallbacks(_))
      .WillByDefault(
          Invoke([this](Network::ConnectionCallbacks &callbacks) -> void {
            callbacks_.push_back(&callbacks);
          }));
  ON_CALL(*this, close()).WillByDefault(Invoke([this]() -> void {
    raiseEvent(Network::ConnectionEvent::LocalClose);
  }));
}

MockClient::~MockClient() {}

MockPoolRequest::MockPoolRequest() {}
MockPoolRequest::~MockPoolRequest() {}

MockPoolCallbacks::MockPoolCallbacks() {}
MockPoolCallbacks::~MockPoolCallbacks() {}

} // namespace ConnPool

} // namespace Tcp
} // namespace Envoy
