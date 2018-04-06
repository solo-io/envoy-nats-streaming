#include "common/nats/streaming/message_utility.h"

namespace Envoy {
namespace Nats {
namespace Streaming {

std::string MessageUtility::createConnectRequestMessage(
    const std::string &client_id, const std::string &heartbeat_inbox) {
  pb::ConnectRequest connect_request;
  connect_request.set_clientid(client_id);
  connect_request.set_heartbeatinbox(heartbeat_inbox);

  return serializeToString(connect_request);
}

std::string MessageUtility::createConnectResponseMessage(
    const std::string &pub_prefix, const std::string &sub_requests,
    const std::string &unsub_requests, const std::string &close_requests) {
  pb::ConnectResponse connect_response;
  connect_response.set_pubprefix(pub_prefix);
  connect_response.set_subrequests(sub_requests);
  connect_response.set_unsubrequests(unsub_requests);
  connect_response.set_closerequests(close_requests);

  return serializeToString(connect_response);
}

std::string MessageUtility::createPubMsgMessage(const std::string &client_id,
                                                const std::string &guid,
                                                const std::string &subject,
                                                const std::string &data) {
  pb::PubMsg pub_msg;
  pub_msg.set_clientid(client_id);
  pub_msg.set_guid(guid);
  pub_msg.set_subject(subject);
  pub_msg.set_data(data);

  return serializeToString(pub_msg);
}

std::string MessageUtility::createPubAckMessage(const std::string &guid,
                                                const std::string &error) {
  pb::PubAck pub_ack;
  pub_ack.set_guid(guid);
  pub_ack.set_error(error);

  return serializeToString(pub_ack);
}

absl::optional<pb::PubAck>
MessageUtility::parsePubAckMessage(const std::string &pub_ack_message) {
  pb::PubAck pub_ack;
  if (pub_ack.ParseFromString(pub_ack_message)) {
    return pub_ack;
  }

  return {};
}

std::string
MessageUtility::getPubPrefix(const std::string &connect_response_message) {
  pb::ConnectResponse connect_response;
  connect_response.ParseFromString(connect_response_message);
  return connect_response.pubprefix();
}

} // namespace Streaming
} // namespace Nats
} // namespace Envoy
