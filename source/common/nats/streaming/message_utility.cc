#include "common/nats/streaming/message_utility.h"

#include "protocol.pb.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

std::string MessageUtility::createConnectRequestMessage(
    const std::string &client_id, const std::string &heartbeat_inbox) const {
  pb::ConnectRequest connect_request;
  connect_request.set_clientid(client_id);
  connect_request.set_heartbeatinbox(heartbeat_inbox);

  return serializeToString(connect_request);
}

std::string MessageUtility::createConnectResponseMessage(
    const std::string &pub_prefix, const std::string &sub_requests,
    const std::string &unsub_requests,
    const std::string &close_requests) const {
  pb::ConnectResponse connect_response;
  connect_response.set_pubprefix(pub_prefix);
  connect_response.set_subrequests(sub_requests);
  connect_response.set_unsubrequests(unsub_requests);
  connect_response.set_closerequests(close_requests);

  return serializeToString(connect_response);
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
