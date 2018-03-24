#include "common/nats/nuid/nuid.h"

namespace Envoy {
namespace Nats {
namespace Nuid {

constexpr const char Nuid::DIGITS[];
constexpr uint8_t Nuid::BASE;
constexpr uint8_t Nuid::PRE_LEN;
constexpr uint8_t Nuid::SEQ_LEN;
constexpr uint64_t Nuid::MAX_SEQ;
constexpr uint64_t Nuid::MIN_INC;
constexpr uint64_t Nuid::MAX_INC;
constexpr uint8_t Nuid::TOTAL_LEN;

Nuid::Nuid(Runtime::RandomGenerator &random_generator)
    : random_generator_(random_generator), seq_(int63_n(MAX_SEQ)),
      inc_(MIN_INC + int63_n(MAX_INC - MIN_INC)) {

  randomizePrefix();
}

Nuid::Nuid(Runtime::RandomGenerator &random_generator, uint64_t seq)
    : random_generator_(random_generator), seq_(seq),
      inc_(MIN_INC + int63_n(MAX_INC - MIN_INC)) {
  ASSERT(seq < Nuid::MAX_SEQ);

  randomizePrefix();
}

std::string Nuid::next() {
  // Increment and capture.
  seq_ += inc_;
  if (seq_ >= MAX_SEQ) {
    randomizePrefix();
    resetSequential();
  }

  // Convert the sequential into base62.
  char seq_str[SEQ_LEN];
  convert<SEQ_LEN>(seq_, seq_str);

  // Instantiate a string.
  std::string result;
  result.reserve(TOTAL_LEN);

  // Copy prefix.
  result.append(pre_, PRE_LEN);

  // Copy sequential in base62.
  result.append(seq_str, SEQ_LEN);

  return result;
}

std::string Nuid::pre() { return std::string(pre_, PRE_LEN); }

uint64_t Nuid::int63_n(int64_t n) { return random_generator_.random() % n; }

void Nuid::resetSequential() {
  seq_ = int63_n(MAX_SEQ);
  inc_ = MIN_INC + int63_n(MAX_INC - MIN_INC);
}

void Nuid::randomizePrefix() {
  convert<SEQ_LEN>(int63_n(MAX_SEQ), pre_);

  // BASE ^ (PRE_LEN - SEQ_LEN) == 62 ^ (12 - 10) == 62 ^ 2
  constexpr uint64_t max = BASE * BASE;

  convert<PRE_LEN - SEQ_LEN>(int63_n(max), pre_ + SEQ_LEN);
}

template <uint8_t len> void Nuid::convert(uint64_t n, char *output) {
  static_assert(len <= 10, "Output length should not exceed 10 characters");
  for (uint64_t i = 1; i <= len; ++i, n /= BASE) {
    output[len - i] = DIGITS[n % BASE];
  }
}

} // namespace Nuid
} // namespace Nats
} // namespace Envoy
