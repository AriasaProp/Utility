/* *****************************************************************************
 * hash.c v0.0.0000
 * 
 * Hashing decoder
 * 
 * 
 * 
 * *****************************************************************************/

#include "codec/hash.h"


/* visually hex string writen from left
 * 12af....
 * so, in binary follow it as BIG ENDIAN
 *
 *
 */
static int hash__cstr_to_bins(ubyte *b, const char *s, iter n) {
  char sif;
  ubyte *bend = b + n;
  do {
    sif = '0'*(*s>='0' && *s<='9') +
      ('A'-10)*(*s>='A' && *s<='F') + 
      ('a'-10)*(*s>='a' && *s<='f');
    if (!sif) return 1;
    *b = CAST(ubyte)((*s - sif) & 15) << 4;
    ++s;
    sif = '0'*(*s>='0' && *s<='9') +
      ('A'-10)*(*s>='A' && *s<='F') + 
      ('a'-10)*(*s>='a' && *s<='f');
    if (!sif) return 1;
    *b |= CAST(ubyte)((*s - sif) & 15);
    ++s;
  } while (++b < bend);
  return 0;
}
static void hash__bin_to_hexs(String *str, const ubyte *b, iter n) {
  string_reserve(str, string_len(*str) + n * 2);
  char *c = CAST(char*)*str;
  const ubyte *bend = b + n;
  ubyte bt;
  do {
    bt = (*b >> 4) & 15;
    *(c++) = bt+(bt<=9)*'0'+(bt>9)*('A'-10);
    bt = *b & 15;
    *(c++) = bt+(bt<=9)*'0'+(bt>9)*('A'-10);
  } while (++b < bend);
}


// return 0 on valid hex string
inline int hash_cstr_to_h256(hash256 *h, const char *cstr) {
  return hash__cstr_to_bins(h->b, cstr, HASH256_IN_BYTES);
}
inline int hash_cstr_to_h512(hash512 *h, const char *cstr) {
  return hash__cstr_to_bins(h->b, cstr, HASH512_IN_BYTES);
}
inline void hash_h256_append_string(String *str, const hash256 ch) {
  hash__bin_to_hexs(str, ch.b, HASH256_IN_BYTES);
}
inline void hash_h512_append_string(String *str, const hash512 ch) {
  hash__bin_to_hexs(str, ch.b, HASH512_IN_BYTES);
}

void hash_sha256 (hash256 *out, const char *input, uint64 l) {
  static const uint32 H[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  static const uint32 K[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  
  uint32 W[17] = {0}, State[9] = {0};
  util_memcpy (out->b, H, sizeof(H));
  const char *A = input;

  iter i, k = l;
  do {
    // reset state
    util_memcpy (State, H, 32);
    // put input to chunk W
    i = MIN (k, 64);
    k -= i;
    util_memcpy (W, A, i);
    if (i < 64) (CAST(char *)W)[i++] = 0x80; // add last 1 bit
    util_memset ((CAST(ubyte*)W) + i, 0, (68 - i));
    // flip chunk W
    W[ 0] = imath_flip32(W[ 0]);
    W[ 1] = imath_flip32(W[ 1]);
    W[ 2] = imath_flip32(W[ 2]);
    W[ 3] = imath_flip32(W[ 3]);
    W[ 4] = imath_flip32(W[ 4]);
    W[ 5] = imath_flip32(W[ 5]);
    W[ 6] = imath_flip32(W[ 6]);
    W[ 7] = imath_flip32(W[ 7]);
    W[ 8] = imath_flip32(W[ 8]);
    W[ 9] = imath_flip32(W[ 9]);
    W[10] = imath_flip32(W[10]);
    W[11] = imath_flip32(W[11]);
    W[12] = imath_flip32(W[12]);
    W[13] = imath_flip32(W[13]);
    W[14] = imath_flip32(W[14]);
    W[15] = imath_flip32(W[15]);
   
    if (i < 56) {
      // put 64 bit length in reverse order from other so, no swap
      W[15] = l << 3;
      W[14] = l >> 29;
    }
#define SIG_BOT(I) do { \
	State[0] += K[I]; \
	State[8] = State[5] & State[6]; \
	State[8] ^= ~State[5] & State[7]; \
  State[0] += State[8]; \
	State[4] += State[0]; \
	State[8] = imath_rotr32(State[1], 2); \
	State[8] ^= imath_rotr32(State[1], 13); \
	State[8] ^= imath_rotr32(State[1], 22); \
	State[0] += State[8]; \
	State[8] = State[1] & State[2]; \
	State[8] ^= State[1] & State[3]; \
	State[8] ^= State[2] & State[3]; \
	State[0] += State[8]; \
} while(0)
#define SIG_TOP do { \
	util_memmove(State + 1, State, sizeof(State) - sizeof(State[0])); \
	State[0] = imath_rotr32(State[5], 6); \
	State[0] ^= imath_rotr32(State[5], 11); \
	State[0] ^= imath_rotr32(State[5], 25); \
	State[0] += State[8]; \
} while(0)
  	// Add from chunk
    {
      #define CHUNK16(I) SIG_TOP; State[0] += W[I]; SIG_BOT(I)
      CHUNK16(0);
      CHUNK16(1);
      CHUNK16(2);
      CHUNK16(3);
      CHUNK16(4);
      CHUNK16(5);
      CHUNK16(6);
      CHUNK16(7);
      CHUNK16(8);
      CHUNK16(9);
      CHUNK16(10);
      CHUNK16(11);
      CHUNK16(12);
      CHUNK16(13);
      CHUNK16(14);
      CHUNK16(15);
      #undef CHUNK16
    }
    // do salsa
    {
      #define SALSA do {\
      	SIG_TOP; \
      	W[16] = imath_rotr32(W[1], 7); \
      	W[16] ^= imath_rotr32(W[1], 18); \
      	W[16] ^= (W[1] >> 3); \
      	W[16] += W[0];\
      	State[8] = imath_rotr32(W[14], 17);\
      	State[8] ^= imath_rotr32(W[14], 19);\
      	State[8] ^= W[14] >> 10;\
      	W[16] += State[8];\
      	W[16] += W[9];\
      	State[0] += W[16];\
      	util_memcpy(W, W + 1, sizeof(W) - sizeof(W[0]));\
      } while(0)
      SALSA; SIG_BOT(16);
      SALSA; SIG_BOT(17);
      SALSA; SIG_BOT(18);
      SALSA; SIG_BOT(19);
      SALSA; SIG_BOT(20);
      SALSA; SIG_BOT(21);
      SALSA; SIG_BOT(22);
      SALSA; SIG_BOT(23);
      SALSA; SIG_BOT(24);
      SALSA; SIG_BOT(25);
      SALSA; SIG_BOT(26);
      SALSA; SIG_BOT(27);
      SALSA; SIG_BOT(28);
      SALSA; SIG_BOT(29);
      SALSA; SIG_BOT(30);
      SALSA; SIG_BOT(31);
      SALSA; SIG_BOT(32);
      SALSA; SIG_BOT(33);
      SALSA; SIG_BOT(34);
      SALSA; SIG_BOT(35);
      SALSA; SIG_BOT(36);
      SALSA; SIG_BOT(37);
      SALSA; SIG_BOT(38);
      SALSA; SIG_BOT(39);
      SALSA; SIG_BOT(40);
      SALSA; SIG_BOT(41);
      SALSA; SIG_BOT(42);
      SALSA; SIG_BOT(43);
      SALSA; SIG_BOT(44);
      SALSA; SIG_BOT(45);
      SALSA; SIG_BOT(46);
      SALSA; SIG_BOT(47);
      SALSA; SIG_BOT(48);
      SALSA; SIG_BOT(49);
      SALSA; SIG_BOT(50);
      SALSA; SIG_BOT(51);
      SALSA; SIG_BOT(52);
      SALSA; SIG_BOT(53);
      SALSA; SIG_BOT(54);
      SALSA; SIG_BOT(55);
      SALSA; SIG_BOT(56);
      SALSA; SIG_BOT(57);
      SALSA; SIG_BOT(58);
      SALSA; SIG_BOT(59);
      SALSA; SIG_BOT(60);
      SALSA; SIG_BOT(61);
      SALSA; SIG_BOT(62);
      SALSA; SIG_BOT(63);
      #undef SALSA
    }
#undef SIG_BOT
#undef SIG_TOP
    out->i[0] += State[0];
    out->i[1] += State[1];
    out->i[2] += State[2];
    out->i[3] += State[3];
    out->i[4] += State[4];
    out->i[5] += State[5];
    out->i[6] += State[6];
    out->i[7] += State[7];
    A += 64;
  } while (i >= 56);
  // flip 2
  out->i[0] = imath_flip32(out->i[0]);
  out->i[1] = imath_flip32(out->i[1]);
  out->i[2] = imath_flip32(out->i[2]);
  out->i[3] = imath_flip32(out->i[3]);
  out->i[4] = imath_flip32(out->i[4]);
  out->i[5] = imath_flip32(out->i[5]);
  out->i[6] = imath_flip32(out->i[6]);
  out->i[7] = imath_flip32(out->i[7]);
}
