#include "../deps/googletest/include/gtest/gtest.h"

extern "C" {
uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k);
uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k) {
  return 0;
}

uint64_t siphash_nocase(const uint8_t *in, const size_t inlen, const uint8_t *k) {
  return 0;
}
}
