#include "hash.hpp"

#include <iomanip>
#include <cstring>
#include <cmath>


std::ostream &operator<<(std::ostream &o, const hash256 h)
{
	for (const int &i : h.i)
	{
		o << std::hex << std::setfill('0') << std::setw(8) << i;
	}
	return o;
}
static inline uint32_t _rot(uint32_t a, size_t x)
{
	return (a >> x) | (a << (32 - x));
}
hash256 sha256(const char *input, uint64_t l)
{
	static const uint32_t H_0[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	static const uint32_t K[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
								   0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
								   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
								   0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
								   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
								   0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
								   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
								   0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
								   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
								   0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
								   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
								   0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
								   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
								   0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
								   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
								   0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

	uint32_t W[64]{};
	uint32_t State[10];
	hash256 out;
	memcpy(out.i, H_0, 32);
	const char *A = input;

	size_t x, y;
	size_t i, j, k = l;
	bool addedLastBit = false, addedLength = false;
	do
	{
		// reset state
		memcpy(State, H_0, 32);
		// put input to chunk W
		memset(W, 0, 64);
		j = std::min(k, (size_t)64);
		k -= j;
		for (i = 0, y = 0; i < j; ++y)
			for (x = 4; x && i < j; ++i)
				reinterpret_cast<char *>(W + y)[--x] = A[i];

		if (j < 64)
		{ // add last 1 bit
			reinterpret_cast<char *>(W + (j / 4))[3 - (j % 4)] = 0x80;
			++j;
			addedLastBit = true;
		}
		if (j < 56)
		{
			// put 64 bit length in reverse
			W[15] = l << 3;
			W[14] = l >> 29;
			addedLength = true;
		}
		// Calculate
		for (i = 0; i < 54; ++i)
		{
			uint32_t &C = W[16 + i];
			// Sigma 0
			C = _rot(W[1 + i], 7);
			C ^= _rot(W[1 + i], 18);
			C ^= (W[1 + i] >> 3);

			C += W[i];
			// Sigma 1
			C += _rot(W[14 + i], 17) ^ _rot(W[14 + i], 19) ^ (W[14 + i] >> 10);
			C += W[9 + i];
		}

		for (i = 0; i < 64; ++i)
		{
			State[8] = State[7];
			// Sigma1
			State[8] += _rot(State[4], 6) ^ _rot(State[4], 11) ^ _rot(State[4], 25);
			// Choice
			State[8] += (State[4] & State[5]) ^ ((~State[4]) & State[6]);
			State[8] += K[i];
			State[8] += W[i];
			//Sigma0
			State[9] = _rot(State[0], 2) ^ _rot(State[0], 13) ^ _rot(State[0], 22);
			State[9] += (State[0] & State[1]) ^ (State[0] & State[2]) ^ (State[1] & State[2]);
			State[7] = State[6];
			State[6] = State[5];
			State[5] = State[4];
			State[4] = State[3] + State[8];
			State[3] = State[2];
			State[2] = State[1];
			State[1] = State[0];
			State[0] = State[8] + State[9];
		}

		for (i = 0; i < 8; ++i)
			out.i[i] += State[i];

		A += 64;
	} while (!addedLastBit || !addedLength);

	return out;
}
