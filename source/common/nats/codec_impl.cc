#include "common/nats/codec_impl.h"

#include "envoy/nats/codec.h"

namespace Envoy {
namespace Nats {

std::string Message::toString() const {
  return fmt::format("\"{}\"", asString());
}

std::string &Message::asString() { return string_; }

const std::string &Message::asString() const { return string_; }

void DecoderImpl::decode(Buffer::Instance &data) {
  uint64_t num_slices = data.getRawSlices(nullptr, 0);
  Buffer::RawSlice slices[num_slices];
  data.getRawSlices(slices, num_slices);
  for (const Buffer::RawSlice &slice : slices) {
    parseSlice(slice);
  }

  data.drain(data.length());
}

void DecoderImpl::parseSlice(const Buffer::RawSlice &slice) {
  const char *buffer = reinterpret_cast<const char *>(slice.mem_);
  uint64_t remaining = slice.len_;

  while (remaining || state_ == State::ValueComplete) {
    ENVOY_LOG(trace, "parse slice: {} remaining", remaining);
    switch (state_) {
    case State::ValueRootStart: {
      ENVOY_LOG(trace, "parse slice: ValueRootStart");
      pending_value_root_.reset(new Message());
      state_ = State::SimpleString;
      break;
    }

    case State::SimpleString: {
      ENVOY_LOG(trace, "parse slice: SimpleString: {}", buffer[0]);
      if (buffer[0] == '\r') {
        state_ = State::LF;
      } else {
        pending_value_root_->asString().push_back(buffer[0]);
      }

      remaining--;
      buffer++;
      break;
    }

    case State::LF: {
      ENVOY_LOG(trace, "parse slice: LF");
      if (buffer[0] != '\n') {
        // TODO(talnordan): Consider gracefully ignoring this error.
        throw ProtocolError("expected new line");
      }

      remaining--;
      buffer++;
      state_ = State::ValueComplete;
      break;
    }

    case State::ValueComplete: {
      ENVOY_LOG(trace, "parse slice: ValueComplete");
      callbacks_.onValue(std::move(pending_value_root_));
      state_ = State::ValueRootStart;

      break;
    }
    }
  }
}

void EncoderImpl::encode(const Message &value, Buffer::Instance &out) {
  out.add(value.asString());
  out.add("\r\n", 2);
}

} // namespace Nats
} // namespace Envoy
