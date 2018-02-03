#pragma once

#include <string>

#include "envoy/tcp/codec.h"
#include "envoy/tcp/conn_pool.h"

#include "common/tcp/codec_impl.h"

#include "gmock/gmock.h"

namespace Envoy {
namespace Tcp {

using T = std::string;
using TPtr = MessagePtr<T>;

class MockEncoder : public Encoder<T> {
public:
  MockEncoder();
  ~MockEncoder();

  MOCK_METHOD2(encode, void(const T &value, Buffer::Instance &out));

private:
  void encodeSimpleString(const T &string, Buffer::Instance &out);
};

class MockDecoder : public Decoder {
public:
  MockDecoder();
  MockDecoder(DecoderCallbacks<T> &callbacks);
  ~MockDecoder();

  MOCK_METHOD1(decode, void(Buffer::Instance &data));
};

namespace ConnPool {

class MockClient : public Client<T> {
public:
  MockClient();
  ~MockClient();

  void raiseEvent(Network::ConnectionEvent event) {
    for (Network::ConnectionCallbacks *callbacks : callbacks_) {
      callbacks->onEvent(event);
    }
  }

  MOCK_METHOD1(addConnectionCallbacks,
               void(Network::ConnectionCallbacks &callbacks));
  MOCK_METHOD0(close, void());
  MOCK_METHOD2(makeRequest,
               PoolRequest *(const T &request, PoolCallbacks<T> &callbacks));

  std::list<Network::ConnectionCallbacks *> callbacks_;
};

class MockPoolRequest : public PoolRequest {
public:
  MockPoolRequest();
  ~MockPoolRequest();

  MOCK_METHOD0(cancel, void());
};

class MockPoolCallbacks : public PoolCallbacks<T> {
public:
  MockPoolCallbacks();
  ~MockPoolCallbacks();

  void onResponse(TPtr &&value) override { onResponse_(value); }

  MOCK_METHOD1(onResponse_, void(TPtr &value));
  MOCK_METHOD0(onFailure, void());
};

} // namespace ConnPool

} // namespace Tcp
} // namespace Envoy
