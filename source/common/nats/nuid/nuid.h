#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

#include "envoy/runtime/runtime.h"

#include "common/common/assert.h"

namespace Envoy {
namespace Nats {

/**
 * A unique identifier generator that is high performance, very fast, and tries
 * to be entropy pool friendly.
 *
 * See https://github.com/nats-io/nuid/blob/master/nuid.go
 */
namespace Nuid {

/**
 * NUID needs to be very fast to generate and truly unique, all while being
 * entropy pool friendly. We will use 12 bytes of crypto generated data (entropy
 * draining), and 10 bytes of sequential data that is started at a pseudo random
 * number and increments with a pseudo-random increment. Total is 22 bytes of
 * base 62 ascii text :)
 * */
class Nuid {
public:
  static constexpr const char DIGITS[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  static constexpr uint8_t BASE = 62;
  static constexpr uint8_t PRE_LEN = 12;
  static constexpr uint8_t SEQ_LEN = 10;
  static constexpr uint64_t MAX_SEQ =
      839299365868340224L; // BASE ^ SEQ_LEN == 62^10
  static constexpr uint64_t MIN_INC = 33;
  static constexpr uint64_t MAX_INC = 333;
  static constexpr uint8_t TOTAL_LEN = PRE_LEN + SEQ_LEN;

  /**
   * Generate a new NUID and properly initialize the prefix, sequential start,
   * and sequential increment.
   */
  Nuid(Runtime::RandomGenerator &random_generator);
  Nuid(Runtime::RandomGenerator &random_generator, uint64_t seq);

  /**
   * Generate the next NUID string.
   */
  std::string next();

  std::string pre();

private:
  /**
   * Returns, as an `uint64_t`, a non-negative pseudo-random number in [0,n).
   * n is assumed to be > 0.
   *
   * See: https://golang.org/pkg/math/rand/#Int63n
   */
  inline uint64_t int63_n(int64_t n);

  /**
   * Resets the sequential portion of the NUID.
   */
  inline void resetSequential();
  /**
   * Generate a new pseudo-random prefix.
   */
  inline void randomizePrefix();

  template <uint8_t len> inline void convert(uint64_t n, char *output);

  Runtime::RandomGenerator &random_generator_;
  char pre_[PRE_LEN];
  uint64_t seq_;
  uint64_t inc_;
};

} // namespace Nuid
} // namespace Nats
} // namespace Envoy
