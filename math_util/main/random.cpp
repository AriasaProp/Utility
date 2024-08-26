#include <random>
#include <atomic>
#include <limits>

#include "random.hpp"

static std::atomic<uint32_t> seed(std::random_device{}());
static const std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());

uint32_t random_uint32() {
  uint32_t n = dist(std::mt19937(seed.load()));
  seed.store(n);
  return n;
}

