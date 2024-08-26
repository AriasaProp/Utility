#include <random>
#include <atomic>
#include <limits>

#include "random.hpp"

// Thread-local random number generator
static thread_local std::mt19937 rng(std::random_device{}());
static const std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());

uint32_t random_uint32() {
  return dist(rng);
}
