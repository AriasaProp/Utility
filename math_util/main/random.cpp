#include <random>
#include <atomic>

#include "random.hpp"

std::atomic<std::mt19937> rng(std::random_device{}());

uint32_t random_uint32() {
    std::uniform_int_distribution<uint32_t> dist;
    return dist(rng);
}
