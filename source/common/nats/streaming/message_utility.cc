#include "common/nats/streaming/message_builder.h"

#include "protocol.pb.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

std::string MessageBuilder::createConnectRequestMessage(
    const std::string &client_id, const std::string &heartbeat_inbox) const {
  pb::ConnectRequest connect_request;
  connect_request.set_clientid(client_id);
  connect_request.set_heartbeatinbox(heartbeat_inbox);

  std::string connect_request_str;
  connect_request.SerializeToString(&connect_request_str);

  return connect_request_str;
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
