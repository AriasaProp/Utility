#include <random>

#include "random.hpp"

// Thread-local random number generator
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, 0xffffffff);

uint32_t random_uint32() {
  return dist(gen);
}
