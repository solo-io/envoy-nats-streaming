#include "envoy/nats/codec.h"

#include "common/buffer/buffer_impl.h"
#include "common/common/assert.h"
#include "common/nats/codec_impl.h"

#include "test/mocks/nats/mocks.h"
#include "test/test_common/printers.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Nats {

class NatsEncoderDecoderImplTest : public testing::Test,
                                   public Tcp::DecoderCallbacks<Message> {
public:
  NatsEncoderDecoderImplTest() : decoder_(*this) {}

  // Tcp::DecoderCallbacks
  void onValue(MessagePtr &&value) override {
    decoded_values_.emplace_back(std::move(value));
  }

  EncoderImpl encoder_;
  DecoderImpl decoder_;
  Buffer::OwnedImpl buffer_;
  std::vector<MessagePtr> decoded_values_;
};

TEST_F(NatsEncoderDecoderImplTest, Empty) {
  Message value;
  EXPECT_EQ("\"\"", value.toString());
  encoder_.encode(value, buffer_);
  EXPECT_EQ("\r\n", TestUtility::bufferToString(buffer_));
  decoder_.decode(buffer_);
  EXPECT_EQ(value, *decoded_values_[0]);
  EXPECT_EQ(0UL, buffer_.length());
}

TEST_F(NatsEncoderDecoderImplTest, SimpleString) {
  Message value;
  value.asString() = "simple string";
  EXPECT_EQ("\"simple string\"", value.toString());
  encoder_.encode(value, buffer_);
  EXPECT_EQ("simple string\r\n", TestUtility::bufferToString(buffer_));
  decoder_.decode(buffer_);
  EXPECT_EQ(value, *decoded_values_[0]);
  EXPECT_EQ(0UL, buffer_.length());
}

TEST_F(NatsEncoderDecoderImplTest, MultipleSimpleStrings) {
  Message value1;
  value1.asString() = "simple string 1";
  encoder_.encode(value1, buffer_);

  Message value2;
  value2.asString() = "simple string 2";
  encoder_.encode(value2, buffer_);

  EXPECT_EQ("simple string 1\r\nsimple string 2\r\n",
            TestUtility::bufferToString(buffer_));

  decoder_.decode(buffer_);
  EXPECT_EQ(value1, *decoded_values_[0]);
  EXPECT_EQ(value2, *decoded_values_[1]);
  EXPECT_EQ(0UL, buffer_.length());
}

TEST_F(NatsEncoderDecoderImplTest, MultipleSimpleStringsMultipleDecode) {
  Message value1;
  value1.asString() = "simple string 1";
  encoder_.encode(value1, buffer_);
  EXPECT_EQ("simple string 1\r\n", TestUtility::bufferToString(buffer_));
  decoder_.decode(buffer_);
  EXPECT_EQ(value1, *decoded_values_[0]);
  EXPECT_EQ(0UL, buffer_.length());

  Message value2;
  value2.asString() = "simple string 2";
  encoder_.encode(value2, buffer_);
  EXPECT_EQ("simple string 2\r\n", TestUtility::bufferToString(buffer_));
  decoder_.decode(buffer_);
  EXPECT_EQ(value2, *decoded_values_[1]);
  EXPECT_EQ(0UL, buffer_.length());
}

TEST_F(NatsEncoderDecoderImplTest, MultipleSimpleStringsFragmentedDecode) {
  Message value1;
  value1.asString() = "simple string 1";

  Message value2;
  value2.asString() = "simple string 2";

  Message value3;
  value3.asString() = "simple string 3";

  buffer_.add("simple string 1\r\nsimple s");
  decoder_.decode(buffer_);
  EXPECT_EQ(1, decoded_values_.size());
  EXPECT_EQ(value1, *decoded_values_[0]);
  EXPECT_EQ(0UL, buffer_.length());

  buffer_.add("tring 2\r\nsimple string 3\r\n");
  decoder_.decode(buffer_);
  EXPECT_EQ(3, decoded_values_.size());
  EXPECT_EQ(value2, *decoded_values_[1]);
  EXPECT_EQ(value3, *decoded_values_[2]);
  EXPECT_EQ(0UL, buffer_.length());
}

TEST_F(NatsEncoderDecoderImplTest, InvalidSimpleStringExpectLF) {
  buffer_.add(":-123\ra");
  EXPECT_THROW(decoder_.decode(buffer_), ProtocolError);
}

} // namespace Nats
} // namespace Envoy
