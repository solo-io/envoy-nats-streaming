#include <memory>
#include <string>

#include "envoy/tcp/codec.h"

#include "common/network/utility.h"
#include "common/tcp/conn_pool_impl.h"
#include "common/upstream/upstream_impl.h"

#include "test/mocks/network/mocks.h"
#include "test/mocks/tcp/mocks.h"
#include "test/mocks/thread_local/mocks.h"
#include "test/mocks/upstream/mocks.h"
#include "test/test_common/printers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Eq;
using testing::InSequence;
using testing::Invoke;
using testing::Ref;
using testing::Return;
using testing::ReturnRef;
using testing::SaveArg;
using testing::_;

namespace Envoy {
namespace Tcp {
namespace ConnPool {

using T = std::string;
using TPtr = MessagePtr<T>;

std::chrono::milliseconds createConnPoolSettings() {
  return std::chrono::milliseconds(20);
}

class TcpClientImplTest : public testing::Test, public DecoderFactory<T> {
public:
  // Tcp::DecoderFactory
  DecoderPtr create(DecoderCallbacks<T> &callbacks) override {
    callbacks_ = &callbacks;
    return DecoderPtr{decoder_};
  }

  ~TcpClientImplTest() {
    client_.reset();

    // Make sure all gauges are 0.
    for (const Stats::GaugeSharedPtr &gauge :
         host_->cluster_.stats_store_.gauges()) {
      EXPECT_EQ(0U, gauge->value());
    }
    for (const Stats::GaugeSharedPtr &gauge : host_->stats_store_.gauges()) {
      EXPECT_EQ(0U, gauge->value());
    }
  }

  void setup() {
    config_.reset(new ConfigImpl(createConnPoolSettings()));
    finishSetup();
  }

  void setup(std::unique_ptr<Config> &&config) {
    config_ = std::move(config);
    finishSetup();
  }

  void finishSetup() {
    upstream_connection_ = new NiceMock<Network::MockClientConnection>();
    Upstream::MockHost::MockCreateConnectionData conn_info;
    conn_info.connection_ = upstream_connection_;
    EXPECT_CALL(*connect_or_op_timer_, enableTimer(_));
    EXPECT_CALL(*host_, createConnection_(_)).WillOnce(Return(conn_info));
    EXPECT_CALL(*upstream_connection_, addReadFilter(_))
        .WillOnce(SaveArg<0>(&upstream_read_filter_));
    EXPECT_CALL(*upstream_connection_, connect());
    EXPECT_CALL(*upstream_connection_, noDelay(true));

    client_ = ClientImpl<T>::create(host_, dispatcher_, EncoderPtr<T>{encoder_},
                                    *this, *config_);
    EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_cx_total_.value());
    EXPECT_EQ(1UL, host_->stats_.cx_total_.value());
  }

  void onConnected() {
    EXPECT_CALL(*connect_or_op_timer_, enableTimer(_));
    upstream_connection_->raiseEvent(Network::ConnectionEvent::Connected);
  }

  const std::string cluster_name_{"foo"};
  std::shared_ptr<Upstream::MockHost> host_{new NiceMock<Upstream::MockHost>()};
  Event::MockDispatcher dispatcher_;
  Event::MockTimer *connect_or_op_timer_{new Event::MockTimer(&dispatcher_)};
  MockEncoder *encoder_{new MockEncoder()};
  MockDecoder *decoder_{new MockDecoder()};
  DecoderCallbacks<T> *callbacks_{};
  NiceMock<Network::MockClientConnection> *upstream_connection_{};
  Network::ReadFilterSharedPtr upstream_read_filter_;
  std::unique_ptr<Config> config_;
  ClientPtr<T> client_;
};

TEST_F(TcpClientImplTest, Basic) {
  InSequence s;

  setup();

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  onConnected();

  T request2;
  MockPoolCallbacks callbacks2;
  EXPECT_CALL(*encoder_, encode(Ref(request2), _));
  PoolRequest *handle2 = client_->makeRequest(request2, callbacks2);
  EXPECT_NE(nullptr, handle2);

  EXPECT_EQ(2UL, host_->cluster_.stats_.upstream_rq_total_.value());
  EXPECT_EQ(2UL, host_->cluster_.stats_.upstream_rq_active_.value());
  EXPECT_EQ(2UL, host_->stats_.rq_total_.value());
  EXPECT_EQ(2UL, host_->stats_.rq_active_.value());

  Buffer::OwnedImpl fake_data;
  EXPECT_CALL(*decoder_, decode(Ref(fake_data)))
      .WillOnce(Invoke([&](Buffer::Instance &) -> void {
        InSequence s;
        TPtr response1(new T());
        EXPECT_CALL(callbacks1, onResponse_(Ref(response1)));
        EXPECT_CALL(*connect_or_op_timer_, enableTimer(_));
        EXPECT_CALL(host_->outlier_detector_,
                    putResult(Upstream::Outlier::Result::SUCCESS));
        callbacks_->onValue(std::move(response1));

        TPtr response2(new T());
        EXPECT_CALL(callbacks2, onResponse_(Ref(response2)));
        EXPECT_CALL(*connect_or_op_timer_, disableTimer());
        EXPECT_CALL(host_->outlier_detector_,
                    putResult(Upstream::Outlier::Result::SUCCESS));
        callbacks_->onValue(std::move(response2));
      }));
  upstream_read_filter_->onData(fake_data);

  EXPECT_CALL(*upstream_connection_,
              close(Network::ConnectionCloseType::NoFlush));
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  client_->close();
}

TEST_F(TcpClientImplTest, Cancel) {
  InSequence s;

  setup();

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  onConnected();

  T request2;
  MockPoolCallbacks callbacks2;
  EXPECT_CALL(*encoder_, encode(Ref(request2), _));
  PoolRequest *handle2 = client_->makeRequest(request2, callbacks2);
  EXPECT_NE(nullptr, handle2);

  handle1->cancel();

  Buffer::OwnedImpl fake_data;
  EXPECT_CALL(*decoder_, decode(Ref(fake_data)))
      .WillOnce(Invoke([&](Buffer::Instance &) -> void {
        InSequence s;

        TPtr response1(new T());
        EXPECT_CALL(callbacks1, onResponse_(_)).Times(0);
        EXPECT_CALL(*connect_or_op_timer_, enableTimer(_));
        EXPECT_CALL(host_->outlier_detector_,
                    putResult(Upstream::Outlier::Result::SUCCESS));
        callbacks_->onValue(std::move(response1));

        TPtr response2(new T());
        EXPECT_CALL(callbacks2, onResponse_(Ref(response2)));
        EXPECT_CALL(*connect_or_op_timer_, disableTimer());
        EXPECT_CALL(host_->outlier_detector_,
                    putResult(Upstream::Outlier::Result::SUCCESS));
        callbacks_->onValue(std::move(response2));
      }));
  upstream_read_filter_->onData(fake_data);

  EXPECT_CALL(*upstream_connection_,
              close(Network::ConnectionCloseType::NoFlush));
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  client_->close();

  EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_rq_cancelled_.value());
}

TEST_F(TcpClientImplTest, FailAll) {
  InSequence s;

  setup();

  NiceMock<Network::MockConnectionCallbacks> connection_callbacks;
  client_->addConnectionCallbacks(connection_callbacks);

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  onConnected();

  EXPECT_CALL(host_->outlier_detector_,
              putResult(Upstream::Outlier::Result::SERVER_FAILURE));
  EXPECT_CALL(callbacks1, onFailure());
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  EXPECT_CALL(connection_callbacks,
              onEvent(Network::ConnectionEvent::RemoteClose));
  upstream_connection_->raiseEvent(Network::ConnectionEvent::RemoteClose);

  EXPECT_EQ(1UL,
            host_->cluster_.stats_.upstream_cx_destroy_with_active_rq_.value());
  EXPECT_EQ(1UL, host_->cluster_.stats_
                     .upstream_cx_destroy_remote_with_active_rq_.value());
}

TEST_F(TcpClientImplTest, FailAllWithCancel) {
  InSequence s;

  setup();

  NiceMock<Network::MockConnectionCallbacks> connection_callbacks;
  client_->addConnectionCallbacks(connection_callbacks);

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  onConnected();
  handle1->cancel();

  EXPECT_CALL(callbacks1, onFailure()).Times(0);
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  EXPECT_CALL(connection_callbacks,
              onEvent(Network::ConnectionEvent::LocalClose));
  upstream_connection_->raiseEvent(Network::ConnectionEvent::LocalClose);

  EXPECT_EQ(1UL,
            host_->cluster_.stats_.upstream_cx_destroy_with_active_rq_.value());
  EXPECT_EQ(
      1UL,
      host_->cluster_.stats_.upstream_cx_destroy_local_with_active_rq_.value());
  EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_rq_cancelled_.value());
}

TEST_F(TcpClientImplTest, ProtocolError) {
  InSequence s;

  setup();

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  onConnected();

  Buffer::OwnedImpl fake_data;
  EXPECT_CALL(*decoder_, decode(Ref(fake_data)))
      .WillOnce(Invoke(
          [&](Buffer::Instance &) -> void { throw ProtocolError("error"); }));
  EXPECT_CALL(host_->outlier_detector_,
              putResult(Upstream::Outlier::Result::REQUEST_FAILED));
  EXPECT_CALL(*upstream_connection_,
              close(Network::ConnectionCloseType::NoFlush));
  EXPECT_CALL(callbacks1, onFailure());
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  upstream_read_filter_->onData(fake_data);

  EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_cx_protocol_error_.value());
}

TEST_F(TcpClientImplTest, ConnectFail) {
  InSequence s;

  setup();

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  EXPECT_CALL(host_->outlier_detector_,
              putResult(Upstream::Outlier::Result::SERVER_FAILURE));
  EXPECT_CALL(callbacks1, onFailure());
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  upstream_connection_->raiseEvent(Network::ConnectionEvent::RemoteClose);

  EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_cx_connect_fail_.value());
  EXPECT_EQ(1UL, host_->stats_.cx_connect_fail_.value());
}

class ConfigOutlierDisabled : public Config {
  bool disableOutlierEvents() const override { return true; }
  std::chrono::milliseconds opTimeout() const override {
    return std::chrono::milliseconds(25);
  }
};

TEST_F(TcpClientImplTest, OutlierDisabled) {
  InSequence s;

  setup(std::make_unique<ConfigOutlierDisabled>());

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  EXPECT_CALL(host_->outlier_detector_, putResult(_)).Times(0);
  EXPECT_CALL(callbacks1, onFailure());
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  upstream_connection_->raiseEvent(Network::ConnectionEvent::RemoteClose);

  EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_cx_connect_fail_.value());
  EXPECT_EQ(1UL, host_->stats_.cx_connect_fail_.value());
}

TEST_F(TcpClientImplTest, ConnectTimeout) {
  InSequence s;

  setup();

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  EXPECT_CALL(host_->outlier_detector_,
              putResult(Upstream::Outlier::Result::TIMEOUT));
  EXPECT_CALL(*upstream_connection_,
              close(Network::ConnectionCloseType::NoFlush));
  EXPECT_CALL(callbacks1, onFailure());
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  connect_or_op_timer_->callback_();

  EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_cx_connect_timeout_.value());
}

TEST_F(TcpClientImplTest, OpTimeout) {
  InSequence s;

  setup();

  T request1;
  MockPoolCallbacks callbacks1;
  EXPECT_CALL(*encoder_, encode(Ref(request1), _));
  PoolRequest *handle1 = client_->makeRequest(request1, callbacks1);
  EXPECT_NE(nullptr, handle1);

  onConnected();

  EXPECT_CALL(host_->outlier_detector_,
              putResult(Upstream::Outlier::Result::TIMEOUT));
  EXPECT_CALL(*upstream_connection_,
              close(Network::ConnectionCloseType::NoFlush));
  EXPECT_CALL(callbacks1, onFailure());
  EXPECT_CALL(*connect_or_op_timer_, disableTimer());
  connect_or_op_timer_->callback_();

  EXPECT_EQ(1UL, host_->cluster_.stats_.upstream_rq_timeout_.value());
}

TEST(TcpClientFactoryImplTest, Basic) {
  ClientFactoryImpl<T, MockEncoder, MockDecoder> factory;
  Upstream::MockHost::MockCreateConnectionData conn_info;
  conn_info.connection_ = new NiceMock<Network::MockClientConnection>();
  std::shared_ptr<Upstream::MockHost> host(new NiceMock<Upstream::MockHost>());
  EXPECT_CALL(*host, createConnection_(_)).WillOnce(Return(conn_info));
  NiceMock<Event::MockDispatcher> dispatcher;
  ConfigImpl config(createConnPoolSettings());
  ClientPtr<T> client = factory.create(host, dispatcher, config);
  client->close();
}

class TcpConnPoolImplTest : public testing::Test, public ClientFactory<T> {
public:
  TcpConnPoolImplTest() {
    conn_pool_.reset(new InstanceImpl<T, MockDecoder>(
        cluster_name_, cm_, *this, tls_, createConnPoolSettings()));
  }

  // Tcp::ConnPool::ClientFactory
  // TODO(talnordan): Use `MockClientFactory` instead of having this class
  // implemnting `ClientFactory<T>.
  ClientPtr<T> create(Upstream::HostConstSharedPtr host, Event::Dispatcher &,
                      const Config &) override {
    return ClientPtr<T>{create_(host)};
  }

  MOCK_METHOD1(create_, Client<T> *(Upstream::HostConstSharedPtr host));

  const std::string cluster_name_{"foo"};
  NiceMock<Upstream::MockClusterManager> cm_;
  NiceMock<ThreadLocal::MockInstance> tls_;
  InstancePtr<T> conn_pool_;
};

TEST_F(TcpConnPoolImplTest, Basic) {
  InSequence s;

  T value;
  MockPoolRequest active_request;
  MockPoolCallbacks callbacks;
  MockClient *client = new NiceMock<MockClient>();

  EXPECT_CALL(cm_.thread_local_cluster_.lb_, chooseHost(_))
      .WillOnce(Invoke([&](Upstream::LoadBalancerContext *context)
                           -> Upstream::HostConstSharedPtr {
        EXPECT_EQ(context->computeHashKey().value(),
                  std::hash<std::string>()("foo"));
        return cm_.thread_local_cluster_.lb_.host_;
      }));
  EXPECT_CALL(*this, create_(_)).WillOnce(Return(client));
  EXPECT_CALL(*client, makeRequest(Ref(value), Ref(callbacks)))
      .WillOnce(Return(&active_request));
  PoolRequest *request = conn_pool_->makeRequest("foo", value, callbacks);
  EXPECT_EQ(&active_request, request);

  EXPECT_CALL(*client, close());
  tls_.shutdownThread();
};

TEST_F(TcpConnPoolImplTest, HostRemove) {
  InSequence s;
  MockPoolCallbacks callbacks;

  T value;
  std::shared_ptr<Upstream::Host> host1(new Upstream::MockHost());
  std::shared_ptr<Upstream::Host> host2(new Upstream::MockHost());
  MockClient *client1 = new NiceMock<MockClient>();
  MockClient *client2 = new NiceMock<MockClient>();

  EXPECT_CALL(cm_.thread_local_cluster_.lb_, chooseHost(_))
      .WillOnce(Return(host1));
  EXPECT_CALL(*this, create_(Eq(host1))).WillOnce(Return(client1));

  MockPoolRequest active_request1;
  EXPECT_CALL(*client1, makeRequest(Ref(value), Ref(callbacks)))
      .WillOnce(Return(&active_request1));
  PoolRequest *request1 = conn_pool_->makeRequest("foo", value, callbacks);
  EXPECT_EQ(&active_request1, request1);

  EXPECT_CALL(cm_.thread_local_cluster_.lb_, chooseHost(_))
      .WillOnce(Return(host2));
  EXPECT_CALL(*this, create_(Eq(host2))).WillOnce(Return(client2));

  MockPoolRequest active_request2;
  EXPECT_CALL(*client2, makeRequest(Ref(value), Ref(callbacks)))
      .WillOnce(Return(&active_request2));
  PoolRequest *request2 = conn_pool_->makeRequest("bar", value, callbacks);
  EXPECT_EQ(&active_request2, request2);

  EXPECT_CALL(*client2, close());
  cm_.thread_local_cluster_.cluster_.prioritySet()
      .getMockHostSet(0)
      ->runCallbacks({}, {host2});

  EXPECT_CALL(*client1, close());
  tls_.shutdownThread();
}

TEST_F(TcpConnPoolImplTest, DeleteFollowedByClusterUpdateCallback) {
  conn_pool_.reset();

  std::shared_ptr<Upstream::Host> host(new Upstream::MockHost());
  cm_.thread_local_cluster_.cluster_.prioritySet()
      .getMockHostSet(0)
      ->runCallbacks({}, {host});
}

TEST_F(TcpConnPoolImplTest, NoHost) {
  InSequence s;

  T value;
  MockPoolCallbacks callbacks;
  EXPECT_CALL(cm_.thread_local_cluster_.lb_, chooseHost(_))
      .WillOnce(Return(nullptr));
  PoolRequest *request = conn_pool_->makeRequest("foo", value, callbacks);
  EXPECT_EQ(nullptr, request);

  tls_.shutdownThread();
}

TEST_F(TcpConnPoolImplTest, RemoteClose) {
  InSequence s;

  T value;
  MockPoolRequest active_request;
  MockPoolCallbacks callbacks;
  MockClient *client = new NiceMock<MockClient>();

  EXPECT_CALL(cm_.thread_local_cluster_.lb_, chooseHost(_));
  EXPECT_CALL(*this, create_(_)).WillOnce(Return(client));
  EXPECT_CALL(*client, makeRequest(Ref(value), Ref(callbacks)))
      .WillOnce(Return(&active_request));
  conn_pool_->makeRequest("foo", value, callbacks);

  EXPECT_CALL(tls_.dispatcher_, deferredDelete_(_));
  client->raiseEvent(Network::ConnectionEvent::RemoteClose);

  tls_.shutdownThread();
}

class TcpConnPoolManagerImplTest : public testing::Test {
public:
  TcpConnPoolManagerImplTest() {
    conn_pool_mgr_.reset(new ManagerImpl<T, MockDecoder>(
        cm_, factory_, tls_, createConnPoolSettings()));
  }

  NiceMock<Upstream::MockClusterManager> cm_;
  NiceMock<MockClientFactory> factory_;
  NiceMock<ThreadLocal::MockInstance> tls_;
  ManagerPtr<T> conn_pool_mgr_;
};

TEST_F(TcpConnPoolManagerImplTest, SameCluster) {
  auto &instance1 = conn_pool_mgr_->getInstance("cluster1");
  auto &instance2 = conn_pool_mgr_->getInstance("cluster1");
  EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TcpConnPoolManagerImplTest, DifferentClusters) {
  auto &instance1 = conn_pool_mgr_->getInstance("cluster1");
  auto &instance2 = conn_pool_mgr_->getInstance("cluster2");
  EXPECT_NE(&instance1, &instance2);
}

TEST_F(TcpConnPoolManagerImplTest, InterleavedClusters) {
  auto &instance1 = conn_pool_mgr_->getInstance("cluster1");
  auto &instance2 = conn_pool_mgr_->getInstance("cluster2");
  auto &instance3 = conn_pool_mgr_->getInstance("cluster1");

  EXPECT_NE(&instance1, &instance2);
  EXPECT_EQ(&instance1, &instance3);
}

} // namespace ConnPool
} // namespace Tcp
} // namespace Envoy
