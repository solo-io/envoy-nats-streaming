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
  MOCK_METHOD1(makeRequest, void(const T &request));
  MOCK_METHOD0(cancel, void());

  std::list<Network::ConnectionCallbacks *> callbacks_;
};

class MockClientFactory : public ClientFactory<T> {
public:
  MockClientFactory();
  ~MockClientFactory();

  // Tcp::ConnPool::ClientFactory
  ClientPtr<T> create(Upstream::HostConstSharedPtr host, Event::Dispatcher &,
                      PoolCallbacks<T> &, const Config &) override {
    return ClientPtr<T>{create_(host)};
  }

  MOCK_METHOD1(create_, Client<T> *(Upstream::HostConstSharedPtr host));
};

class MockPoolCallbacks : public PoolCallbacks<T> {
public:
  MockPoolCallbacks();
  ~MockPoolCallbacks();

  void onResponse(TPtr &&value) override { onResponse_(value); }

  MOCK_METHOD1(onResponse_, void(TPtr &value));
  MOCK_METHOD0(onClose, void());
};

class MockInstance : public Instance<T> {
public:
  MockInstance();
  ~MockInstance();

  MOCK_METHOD2(makeRequest,
               void(const std::string &hash_key, const T &request));
};

} // namespace ConnPool

} // namespace Tcp
} // namespace Envoy
