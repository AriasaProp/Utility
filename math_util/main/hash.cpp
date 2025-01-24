#include "hash.hpp"

#include <cmath>
#include <cstring>

hash256 hash256FromString(const char *src) {
	hash256 r{};
	for (size_t i = 0, j; i < 8; ++i) {
    for (j = 0; j < 8; ++j) {
      r.i[i] <<= 4;
    	++src;
      r.i[i] |= *src - 48 - (*src >= 'a') * 39;
    }
  }
  return r;
}
hash512 hash512FromString(const char *src) {
	hash512 r{};
	for (size_t i = 0, j; i < 16; ++i) {
    for (j = 0; j < 8; ++j) {
      r.i[i] <<= 4;
    	++src;
      r.i[i] |= *src - 48 - (*src >= 'a') * 39;
    }
  }
  return r;
}

std::ostream &operator<< (std::ostream &o, const hash256 h) {
  uint32_t x, y;
  for (const uint32_t &i : h.i) {
		for (x = 32; x;) {
			x -= 4;
			y = (i >> x) & 0xf;
			o << char(48 + y + (y > 9) * 39);
		}
	}
	return o;
}
std::ostream &operator<< (std::ostream &o, const hash512 h) {
  uint32_t x, y;
  for (const uint32_t &i : h.i) {
		for (x = 32; x;) {
			x -= 4;
			y = (i >> x) & 0xf;
			o << char(48 + y + (y > 9) * 39);
		}
	}
	return o;
}
namespace SHA {
static inline uint32_t _rot (uint32_t a, size_t x) {
  return (a >> x) | (a << (32 - x));
}
static const uint32_t H[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
static const uint32_t K[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
}
hash256 sha256 (const char *input, uint64_t l) {
  uint32_t W[17]{}, State[9];
  hash256 out;
  memcpy (out.i, SHA::H, 32);
  const char *A = input;

  size_t x, y, i, j, k = l;
  bool addedLastBit = false, addedLength = false;
  do {
    // reset state
    memcpy (State, SHA::H, 32);
    // put input to chunk W
    memset (W, 0, 64);
    j = std::min (k, (size_t)64);
    k -= j;
    for (i = 0, y = 0; i < j; ++y)
      for (x = 4; x && i < j; ++i)
        reinterpret_cast<char *> (W + y)[--x] = A[i];

    if (j < 64) { // add last 1 bit
      reinterpret_cast<char *> (W + (j / 4))[3 - (j % 4)] = 0x80;
      ++j;
      addedLastBit = true;
    }
    if (j < 56) {
      // put 64 bit length in reverse
      W[15] = l << 3;
      W[14] = l >> 29;
      addedLength = true;
    }

    for (i = 0; i < 64; ++i) {
			// shift variables
			memmove(State + 1, State, 32);
			// Sigma1
			State[0] = SHA::_rot(State[5], 6);
			State[0] ^= SHA::_rot(State[5], 11);
			State[0] ^= SHA::_rot(State[5], 25);
			// add last variable
			State[0] += State[8];
			// Add from chunk or generate it
			if (i > 15) { // generate chunk
				// Sigma 0
				W[16] = SHA::_rot(W[1], 7);
				W[16] ^= SHA::_rot(W[1], 18);
				W[16] ^= (W[1] >> 3);
				// chunk 0
				W[16] += W[0];
				// Sigma 1
				State[8] = SHA::_rot(W[14], 17);
				State[8] ^= SHA::_rot(W[14], 19);
				State[8] ^= W[14] >> 10;
				W[16] += State[8];
				// chunk 1
				W[16] += W[9];
				State[0] += W[16];
				memmove(W, W + 1, 64);
			} else {
				State[0] += W[i];
			}
			// Add constant
			State[0] += SHA::K[i];
			// use last variable as temp choice
			State[8] = State[5] & State[6];
			State[8] ^= ~State[5] & State[7];
			// Add Choice
			State[0] += State[8];

			State[4] += State[0];
			// Sigma0
			State[8] = SHA::_rot(State[1], 2);
			State[8] ^= SHA::_rot(State[1], 13);
			State[8] ^= SHA::_rot(State[1], 22);
			State[0] += State[8];
			// Choice
			State[8] = State[1] & State[2];
			State[8] ^= State[1] & State[3];
			State[8] ^= State[2] & State[3];
			State[0] += State[8];
    }

    for (i = 0; i < 8; ++i)
      out.i[i] += State[i];

    A += 64;
  } while (!addedLastBit || !addedLength);

  return out;
}

hash256 randomX256 (const char *, uint64_t) {
  return hash256 ();
}
hash512 randomX512 (const char *, uint64_t) {
  return hash512 ();
}