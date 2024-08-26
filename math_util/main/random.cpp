#include <random>
#include <atomic>

#include "random.hpp"

uint32_t random_uint32() {
	static std::atomic<std::mt19937> rng(std::random_device{}());
	static std::uniform_int_distribution<uint32_t> dist;
  return dist(rng);
}
