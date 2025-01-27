#include "hash.hpp"

#include <cmath>
#include <cstring>

hash256 hash256FromString(const char *src) {
	hash256 r{};
	char y;
  for (char &i : r.b) {
    i = *src - 48 - (*src >= 'a') * 39;
    i <<= 4;
  	++src;
    i |= *src - 48 - (*src >= 'a') * 39;
  	++src;
  }
  return r;
}
hash512 hash512FromString(const char *src) {
	hash512 r{};
	char y;
  for (char &i : r.b) {
    i = *src - 48 - (*src >= 'a') * 39;
    i <<= 4;
  	++src;
    i |= *src - 48 - (*src >= 'a') * 39;
  	++src;
  }
  return r;
}
std::ostream &operator<< (std::ostream &o, const hash256 h) {
	char y;
  for (const char &i : h.b) {
  	y = (i >> 4) & 0xf;
		o << char(48 + y + (y > 9) * 39);
  	y = i & 0xf;
		o << char(48 + y + (y > 9) * 39);
	}
	return o;
}
std::ostream &operator<< (std::ostream &o, const hash512 h) {
  char y;
  for (const char &i : h.b) {
  	y = (i >> 4) & 0xf;
		o << char(48 + y + (y > 9) * 39);
  	y = i & 0xf;
		o << char(48 + y + (y > 9) * 39);
	}
	return o;
}
namespace SHA {
static inline uint32_t _rot (uint32_t a, size_t x) {
  return (a >> x) | (a << (32 - x));
}
static inline void _swap (uint32_t &w) {
#if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
	w = __builtin_bswap32(w);
#else
	w = ((w & 0xff) << 24) | ((w & 0xff00u) << 8) | ((w >> 8) & 0xff00u) | ((w >> 24) & 0xff);
#endif
}
static const uint32_t H[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
static const uint32_t K[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
}
hash256 sha256 (const char *input, uint64_t l) {
  uint32_t W[17]{}, State[9];
  hash256 out;
  memcpy (out.i, SHA::H, 32);
  const char *A = input;

  size_t i, j, k = l;
  int addedFlag = 0;
  do {
    // reset state
    memcpy (State, SHA::H, 32);
    // put input to chunk W
    j = std::min (k, (size_t)64);
    k -= j;
    memset (W, 0, 64);
    memcpy (W, A, j);
    if (j < 64) { // add last 1 bit
      reinterpret_cast<char *> (W)[j] = 0x80;
      ++j;
      addedFlag |= 1;
    }
    for (uint32_t &w : W)
    	SHA::_swap(w);
    
    if (j < 56) {
      // put 64 bit length in reverse
      W[15] = l << 3;
      W[14] = l >> 29;
      addedFlag |= 2;
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
  } while (!addedFlag);
  for (i = 0; i < 8; ++i)
  	SHA::_swap(out.i[i]);
  return out;
}

namespace SCRYPT {
static inline uint32_t _rot (uint32_t a, size_t x) {
  return (a << x) | (a >> (32 - x));
}
#define QR(a, b, c, d)(  \
	b ^= _rot(a + d, 7), \
	c ^= _rot(b + a, 9), \
	d ^= _rot(c + b,13), \
	a ^= _rot(d + c,18))
void salsa8(uint32_t out[16], uint32_t const in[16]) {
	size_t i;
	uint32_t x[16];

	for (i = 0; i < 16; ++i)
		x[i] = in[i];
	for (i = 0; i < 4; ++i) {
		// Odd round
		QR(x[ 0], x[ 4], x[ 8], x[12]); // column 1
		QR(x[ 5], x[ 9], x[13], x[ 1]); // column 2
		QR(x[10], x[14], x[ 2], x[ 6]); // column 3
		QR(x[15], x[ 3], x[ 7], x[11]); // column 4
		// Even round
		QR(x[ 0], x[ 1], x[ 2], x[ 3]); // row 1
		QR(x[ 5], x[ 6], x[ 7], x[ 4]); // row 2
		QR(x[10], x[11], x[ 8], x[ 9]); // row 3
		QR(x[15], x[12], x[13], x[14]); // row 4
	}
	for (i = 0; i < 16; ++i)
		out[i] = x[i] + in[i];
}

void blockMix() {
	
}

	/*
	Function BlockMix(B):

    The block B is r 128-byte chunks (which is equivalent of 2r 64-byte chunks)
    r ← Length(B) / 128;

    Treat B as an array of 2r 64-byte chunks
    [B0...B2r-1] ← B

    X ← B2r−1
    for i ← 0 to 2r−1 do
        X ← Salsa20/8(X xor Bi)  // Salsa20/8 hashes from 64-bytes to 64-bytes
        Yi ← X

    return ← Y0∥Y2∥...∥Y2r−2 ∥ Y1∥Y3∥...∥Y2r−1
	*/
void ROMix() {
	
}
	/*
	Function ROMix(Block, Iterations)

   Create Iterations copies of X
   X ← Block
   for i ← 0 to Iterations−1 do
      Vi ← X
      X ← BlockMix(X)

   for i ← 0 to Iterations−1 do
      j ← Integerify(X) mod Iterations 
      X ← BlockMix(X xor Vj)

   return X
	*/


}
/*
void pbkdf2_single(const char *passphrase, const char *salt, size_t key_length, prf):
    '''Returns the result of the Password-Based Key Derivation Function 2 with
       a single iteration (i.e. count = 1).

       prf - a psuedorandom function

       See http://en.wikipedia.org/wiki/PBKDF2
    '''

    block_number = 0
    result = b''

    # The iterations
    while len(result) < key_length:
        block_number += 1
        result += prf(password, salt + struct.pack('>L', block_number))

    return result[:key_length]
*/


hash256 scrypt (const char *passphrase, const char *salt,
 N = 1024, 
 r = 1, 
 p = 1, 
 dkLen = 32) {
	/*
	input:
	Passphrase:                Bytes    string of characters to be hashed
  Salt:                      Bytes    string of random characters that modifies the hash to protect against Rainbow table attacks
  CostFactor (N):            Integer  CPU/memory cost parameter – Must be a power of 2 (e.g. 1024)
	BlockSizeFactor (r):       Integer  blocksize parameter, which fine-tunes sequential memory read size and performance. (8 is commonly used)
	ParallelizationFactor (p): Integer  Parallelization parameter. (1 .. 232-1 * hLen/MFlen)
  DesiredKeyLen (dkLen):     Integer  Desired key length in bytes (Intended output length in octets of the derived key; a positive integer satisfying dkLen ≤ (232− 1) * hLen.)
  hLen:                      Integer  The length in octets of the hash function (32 for SHA256).
  MFlen:                     Integer  The length in octets of the output of the mixing function (SMix below). Defined as r * 128 in RFC7914.
  
  Output:
	DerivedKey:                Bytes    array of bytes, DesiredKeyLen long
	
	Step 1. Generate expensive salt
	blockSize ← 128*BlockSizeFactor  // Length (in bytes) of the SMix mixing function output (e.g. 128*8 = 1024 bytes)
	
	Use PBKDF2 to generate initial 128*BlockSizeFactor*p bytes of data (e.g. 128*8*3 = 3072 bytes)
	Treat the result as an array of p elements, each entry being blocksize bytes (e.g. 3 elements, each 1024 bytes)
	[B0...Bp−1] ← PBKDF2HMAC-SHA256(Passphrase, Salt, 1, blockSize*ParallelizationFactor)

   Mix each block in B Costfactor times using ROMix function (each block can be mixed in parallel)
   for i ← 0 to p-1 do
      Bi ← ROMix(Bi, CostFactor)

   All the elements of B is our new "expensive" salt
   expensiveSalt ← B0∥B1∥B2∥ ... ∥Bp-1  // where ∥ is concatenation
 
   Step 2. Use PBKDF2 to generate the desired number of bytes, but using the expensive salt we just generated
   return PBKDF2HMAC-SHA256(Passphrase, expensiveSalt, 1, DesiredKeyLen);
	*/
	size_t blockSizeFactor = 8;
	size_t blockSize = 128 * blockSizeFactor;
	size_t p = 
	return 0;
}

hash256 randomX256 (const char *, uint64_t) {
  return hash256 ();
}
hash512 randomX512 (const char *, uint64_t) {
  return hash512 ();
}