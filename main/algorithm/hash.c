/* *****************************************************************************
 * hash.c v0.0.0000
 * 
 * Hashing algorithm
 * 
 * 
 * 
 * *****************************************************************************/

#include "algorithm/hash.h"
#include "util/console_out.h"
#if (defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)) || \
    (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || \
    defined(MANUAL_CHECK_LITTLE_ENDIAN)
  #define BYTE_FLIP
#else
  #error "Need byte order manual check and define with MANUAL_CHECK_LITTLE_ENDIAN"
#endif

/* visually hex string writen from left
 * 12af....
 * so, in binary follow it as BIG ENDIAN
 *
 *
 */
int hash_cstr_to_ubyte(ubyte *b, const char *s, iter n) {
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
void hash_ubyte_append_string(String *str, const ubyte *b, iter n) {
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

void hash_md5(uint32 *out, const char *str, uint64 l) {
#define DIGEST          4
#define CHUNK           16
#define DIGEST_BYTE     (sizeof(uint32) * DIGEST)
#define DIGEST_ALL_BYTE (sizeof(uint32) * (DIGEST + 1))
#define CHUNK_BYTE      (sizeof(uint32) * CHUNK)
#define CHUNK_ALL_BYTE  (sizeof(uint32) * (CHUNK + 1))
#define CHUNK_KUOTA     (sizeof(uint32) * (CHUNK - 2))
  static const uint32 H[DIGEST] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
  union { ubyte b[CHUNK_ALL_BYTE]; uint32 i[CHUNK + 1]; } W = {0};
  uint32 digest[DIGEST + 1];
  util_memcpy (out, H, DIGEST_BYTE);
  iter i, k = l;
  do {
    i = MIN(k, CHUNK_BYTE);
    k -= i;
    util_memcpy(W.b, str, i);
    str += i;
    if (i < CHUNK_BYTE) W.b[i++] = 0x80; // add last 1 bit
    util_memset(W.b + i, 0, CHUNK_ALL_BYTE - i);
#ifndef BYTE_FLIP
    W.i[ 0] = imath_flip32(W.i[ 0]);
    W.i[ 1] = imath_flip32(W.i[ 1]);
    W.i[ 2] = imath_flip32(W.i[ 2]);
    W.i[ 3] = imath_flip32(W.i[ 3]);
    W.i[ 4] = imath_flip32(W.i[ 4]);
    W.i[ 5] = imath_flip32(W.i[ 5]);
    W.i[ 6] = imath_flip32(W.i[ 6]);
    W.i[ 7] = imath_flip32(W.i[ 7]);
    W.i[ 8] = imath_flip32(W.i[ 8]);
    W.i[ 9] = imath_flip32(W.i[ 9]);
    W.i[10] = imath_flip32(W.i[10]);
    W.i[11] = imath_flip32(W.i[11]);
    W.i[12] = imath_flip32(W.i[12]);
    W.i[13] = imath_flip32(W.i[13]);
    if (i < CHUNK_KUOTA) {
      // put 64 bit length in reverse order from other so, no swap
      W.i[15] = imath_flip32(l <<  3);
      W.i[14] = imath_flip32(l >> 29);
    } else {
      W.i[14] = imath_flip32(W.i[14]);
      W.i[15] = imath_flip32(W.i[15]);
    }
#else
    if (i < CHUNK_KUOTA) {
      // put 64 bit length in reverse order from other so, no swap
      W.i[15] = l <<  3;
      W.i[14] = l >> 29;
    }
#endif // BYTE_FLIP
    // initialize
    util_memcpy (digest, out, DIGEST_BYTE);
    // main
#define SIG_BOT(K,S) do {\
  digest[4] += K; \
  digest[4] += digest[0];\
  digest[0] = digest[3];\
  util_memmove(digest + 2, digest + 1, sizeof(uint32) * 2);\
  digest[1] += imath_rotl32(digest[4], S);\
} while(0)
#define SIG_TOP(I) do {\
  digest[4] = digest[1];\
  digest[4] &= digest[2];\
  digest[4] |= ~digest[1] & digest[3];\
  digest[4] += W.i[I];\
} while(0)
    SIG_TOP( 0); SIG_BOT(0xd76aa478, 7);
    SIG_TOP( 1); SIG_BOT(0xe8c7b756,12);
    SIG_TOP( 2); SIG_BOT(0x242070db,17);
    SIG_TOP( 3); SIG_BOT(0xc1bdceee,22);
    SIG_TOP( 4); SIG_BOT(0xf57c0faf, 7);
    SIG_TOP( 5); SIG_BOT(0x4787c62a,12);
    SIG_TOP( 6); SIG_BOT(0xa8304613,17);
    SIG_TOP( 7); SIG_BOT(0xfd469501,22);
    SIG_TOP( 8); SIG_BOT(0x698098d8, 7);
    SIG_TOP( 9); SIG_BOT(0x8b44f7af,12);
    SIG_TOP(10); SIG_BOT(0xffff5bb1,17);
    SIG_TOP(11); SIG_BOT(0x895cd7be,22);
    SIG_TOP(12); SIG_BOT(0x6b901122, 7);
    SIG_TOP(13); SIG_BOT(0xfd987193,12);
    SIG_TOP(14); SIG_BOT(0xa679438e,17);
    SIG_TOP(15); SIG_BOT(0x49b40821,22);
#undef SIG_TOP
#define SIG_TOP(I) do {\
  digest[4] = digest[3] & digest[1]; \
  digest[4] |= ~digest[3] & digest[2];\
  digest[4] += W.i[I];\
} while(0)
    SIG_TOP( 1); SIG_BOT(0xf61e2562, 5);
    SIG_TOP( 6); SIG_BOT(0xc040b340, 9);
    SIG_TOP(11); SIG_BOT(0x265e5a51,14);
    SIG_TOP( 0); SIG_BOT(0xe9b6c7aa,20);
    SIG_TOP( 5); SIG_BOT(0xd62f105d, 5);
    SIG_TOP(10); SIG_BOT(0x02441453, 9);
    SIG_TOP(15); SIG_BOT(0xd8a1e681,14);
    SIG_TOP( 4); SIG_BOT(0xe7d3fbc8,20);
    SIG_TOP( 9); SIG_BOT(0x21e1cde6, 5);
    SIG_TOP(14); SIG_BOT(0xc33707d6, 9);
    SIG_TOP( 3); SIG_BOT(0xf4d50d87,14);
    SIG_TOP( 8); SIG_BOT(0x455a14ed,20);
    SIG_TOP(13); SIG_BOT(0xa9e3e905, 5);
    SIG_TOP( 2); SIG_BOT(0xfcefa3f8, 9);
    SIG_TOP( 7); SIG_BOT(0x676f02d9,14);
    SIG_TOP(12); SIG_BOT(0x8d2a4c8a,20);
#undef SIG_TOP
#define SIG_TOP(I) do {\
  digest[4] = digest[1];\
  digest[4] ^= digest[2];\
  digest[4] ^= digest[3];\
  digest[4] += W.i[I];\
} while(0)
    SIG_TOP( 5); SIG_BOT(0xfffa3942, 4);
    SIG_TOP( 8); SIG_BOT(0x8771f681,11);
    SIG_TOP(11); SIG_BOT(0x6d9d6122,16);
    SIG_TOP(14); SIG_BOT(0xfde5380c,23);
    SIG_TOP( 1); SIG_BOT(0xa4beea44, 4);
    SIG_TOP( 4); SIG_BOT(0x4bdecfa9,11);
    SIG_TOP( 7); SIG_BOT(0xf6bb4b60,16);
    SIG_TOP(10); SIG_BOT(0xbebfbc70,23);
    SIG_TOP(13); SIG_BOT(0x289b7ec6, 4);
    SIG_TOP( 0); SIG_BOT(0xeaa127fa,11);
    SIG_TOP( 3); SIG_BOT(0xd4ef3085,16);
    SIG_TOP( 6); SIG_BOT(0x04881d05,23);
    SIG_TOP( 9); SIG_BOT(0xd9d4d039, 4);
    SIG_TOP(12); SIG_BOT(0xe6db99e5,11);
    SIG_TOP(15); SIG_BOT(0x1fa27cf8,16);
    SIG_TOP( 2); SIG_BOT(0xc4ac5665,23);
#undef SIG_TOP
#define SIG_TOP(I) do {\
  digest[4] = digest[1];\
  digest[4] |= ~digest[3];\
  digest[4] ^= digest[2];\
  digest[4] += W.i[I];\
} while(0)
    SIG_TOP( 0); SIG_BOT(0xf4292244, 6);
    SIG_TOP( 7); SIG_BOT(0x432aff97,10);
    SIG_TOP(14); SIG_BOT(0xab9423a7,15);
    SIG_TOP( 5); SIG_BOT(0xfc93a039,21);
    SIG_TOP(12); SIG_BOT(0x655b59c3, 6);
    SIG_TOP( 3); SIG_BOT(0x8f0ccc92,10);
    SIG_TOP(10); SIG_BOT(0xffeff47d,15);
    SIG_TOP( 1); SIG_BOT(0x85845dd1,21);
    SIG_TOP( 8); SIG_BOT(0x6fa87e4f, 6);
    SIG_TOP(15); SIG_BOT(0xfe2ce6e0,10);
    SIG_TOP( 6); SIG_BOT(0xa3014314,15);
    SIG_TOP(13); SIG_BOT(0x4e0811a1,21);
    SIG_TOP( 4); SIG_BOT(0xf7537e82, 6);
    SIG_TOP(11); SIG_BOT(0xbd3af235,10);
    SIG_TOP( 2); SIG_BOT(0x2ad7d2bb,15);
    SIG_TOP( 9); SIG_BOT(0xeb86d391,21);
#undef SIG_TOP
#undef SIG_BOT
    out[0] += digest[0];
    out[1] += digest[1];
    out[2] += digest[2];
    out[3] += digest[3];
  } while (i >= CHUNK_KUOTA);
#ifndef BYTE_FLIP
  out[0] = imath_flip32(out[0]);
  out[1] = imath_flip32(out[1]);
  out[2] = imath_flip32(out[2]);
  out[3] = imath_flip32(out[3]);
#endif // BYTE_FLIP
#undef DIGEST
#undef CHUNK
#undef CHUNK_KUOTA
#undef DIGEST_BYTE
#undef CHUNK_BYTE
#undef DIGEST_ALL_BYTE
#undef CHUNK_ALL_BYTE
}
/* ***********
 *  SHA family
 *
 *  try unroll unneccesarry forloop
 *
 * ***********/
void hash_sha1(uint32 *out, const char *str, uint64 l) {
#define DIGEST          5
#define CHUNK           16
#define DIGEST_BYTE     (sizeof(uint32) * DIGEST)
#define DIGEST_ALL_BYTE (sizeof(uint32) * (DIGEST + 1))
#define CHUNK_BYTE      (sizeof(uint32) * CHUNK)
#define CHUNK_ALL_BYTE  (sizeof(uint32) * (CHUNK + 1))
#define CHUNK_KUOTA     (sizeof(uint32) * (CHUNK - 2))
  static const uint32 H[DIGEST] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
  union { ubyte b[CHUNK_ALL_BYTE]; uint32 i[CHUNK + 1]; } W = {0};
  uint32 digest[DIGEST + 1];
  util_memcpy (out, H, DIGEST_BYTE);
  iter i, k = l;
  do {
    i = MIN(k, CHUNK_BYTE);
    k -= i;
    util_memcpy(W.b, str, i);
    str += i;
    if (i < CHUNK_BYTE) W.b[i++] = 0x80; // add last 1 bit
    util_memset(W.b + i, 0, CHUNK_ALL_BYTE - i);
#ifdef BYTE_FLIP
    W.i[ 0] = imath_flip32(W.i[ 0]);
    W.i[ 1] = imath_flip32(W.i[ 1]);
    W.i[ 2] = imath_flip32(W.i[ 2]);
    W.i[ 3] = imath_flip32(W.i[ 3]);
    W.i[ 4] = imath_flip32(W.i[ 4]);
    W.i[ 5] = imath_flip32(W.i[ 5]);
    W.i[ 6] = imath_flip32(W.i[ 6]);
    W.i[ 7] = imath_flip32(W.i[ 7]);
    W.i[ 8] = imath_flip32(W.i[ 8]);
    W.i[ 9] = imath_flip32(W.i[ 9]);
    W.i[10] = imath_flip32(W.i[10]);
    W.i[11] = imath_flip32(W.i[11]);
    W.i[12] = imath_flip32(W.i[12]);
    W.i[13] = imath_flip32(W.i[13]);
    W.i[14] = imath_flip32(W.i[14]);
    W.i[15] = imath_flip32(W.i[15]);
#endif // BYTE_FLIP
    if (i < CHUNK_KUOTA) {
      // put 64 bit length in reverse order from other so, no swap
      W.i[15] = l << 3;
      W.i[14] = l >> 29;
    }
    // initialize
    util_memcpy (digest, out, DIGEST_BYTE);
    // main
#define CHUNK16(I) do {\
  digest[5] = digest[1] & digest[2]; \
  digest[5] |= ~digest[1] & digest[3]; \
  digest[5] += 0x5A827999;\
  digest[5] += imath_rotl32(digest[0], 5);\
  digest[5] += digest[4];\
  digest[5] += W.i[I];\
  digest[4] = digest[3];\
  digest[3] = digest[2];\
  digest[2] = imath_rotr32(digest[1], 2);\
  digest[1] = digest[0];\
  digest[0] = digest[5];\
} while(0)
    CHUNK16( 0); CHUNK16( 1); CHUNK16( 2); CHUNK16( 3);
    CHUNK16( 4); CHUNK16( 5); CHUNK16( 6); CHUNK16( 7);
    CHUNK16( 8); CHUNK16( 9); CHUNK16(10); CHUNK16(11);
    CHUNK16(12); CHUNK16(13); CHUNK16(14); CHUNK16(15);
#undef CHUNK16
#define CORE_END do {\
  W.i[16] = imath_rotl32(W.i[13] ^ W.i[8] ^ W.i[2] ^ W.i[0], 1); \
  digest[5] += imath_rotl32(digest[0], 5);\
  digest[5] += digest[4];\
  digest[5] += W.i[16];\
  digest[4] = digest[3];\
  digest[3] = digest[2];\
  digest[2] = imath_rotr32(digest[1], 2);\
  digest[1] = digest[0];\
  digest[0] = digest[5];\
  util_memcpy(W.i, W.i + 1, CHUNK_BYTE);\
} while(0)
#define CORE_START do {\
  digest[5] = digest[1] & digest[2]; \
  digest[5] |= ~digest[1] & digest[3]; \
  digest[5] += 0x5A827999;\
} while(0)
    CORE_START; CORE_END; // 16
    CORE_START; CORE_END; // 17
    CORE_START; CORE_END; // 18
    CORE_START; CORE_END; // 19
#undef CORE_START
#define CORE_START do {\
  digest[5] = digest[1]; \
  digest[5] ^= digest[2]; \
  digest[5] ^= digest[3]; \
  digest[5] += 0x6ED9EBA1;\
} while(0)
    CORE_START; CORE_END; // 20
    CORE_START; CORE_END; // 21
    CORE_START; CORE_END; // 22
    CORE_START; CORE_END; // 23
    CORE_START; CORE_END; // 24
    CORE_START; CORE_END; // 25
    CORE_START; CORE_END; // 26
    CORE_START; CORE_END; // 27
    CORE_START; CORE_END; // 28
    CORE_START; CORE_END; // 29
    CORE_START; CORE_END; // 30
    CORE_START; CORE_END; // 31
    CORE_START; CORE_END; // 32
    CORE_START; CORE_END; // 33
    CORE_START; CORE_END; // 34
    CORE_START; CORE_END; // 35
    CORE_START; CORE_END; // 36
    CORE_START; CORE_END; // 37
    CORE_START; CORE_END; // 38
    CORE_START; CORE_END; // 39
#undef CORE_START
#define CORE_START do {\
  digest[5] = digest[1] & digest[2]; \
  digest[5] |= digest[1] & digest[3]; \
  digest[5] |= digest[2] & digest[3]; \
  digest[5] += 0x8F1BBCDC;\
} while(0)
    CORE_START; CORE_END; // 40
    CORE_START; CORE_END; // 41
    CORE_START; CORE_END; // 42
    CORE_START; CORE_END; // 43
    CORE_START; CORE_END; // 44
    CORE_START; CORE_END; // 45
    CORE_START; CORE_END; // 46
    CORE_START; CORE_END; // 47
    CORE_START; CORE_END; // 48
    CORE_START; CORE_END; // 49
    CORE_START; CORE_END; // 50
    CORE_START; CORE_END; // 51
    CORE_START; CORE_END; // 52
    CORE_START; CORE_END; // 53
    CORE_START; CORE_END; // 54
    CORE_START; CORE_END; // 55
    CORE_START; CORE_END; // 56
    CORE_START; CORE_END; // 57
    CORE_START; CORE_END; // 58
    CORE_START; CORE_END; // 59
#undef CORE_START
#define CORE_START do {\
  digest[5] = digest[1]; \
  digest[5] ^= digest[2]; \
  digest[5] ^= digest[3]; \
  digest[5] += 0xCA62C1D6;\
} while(0)
    CORE_START; CORE_END; // 60
    CORE_START; CORE_END; // 61
    CORE_START; CORE_END; // 62
    CORE_START; CORE_END; // 63
    CORE_START; CORE_END; // 64
    CORE_START; CORE_END; // 65
    CORE_START; CORE_END; // 66
    CORE_START; CORE_END; // 67
    CORE_START; CORE_END; // 68
    CORE_START; CORE_END; // 69
    CORE_START; CORE_END; // 70
    CORE_START; CORE_END; // 71
    CORE_START; CORE_END; // 72
    CORE_START; CORE_END; // 73
    CORE_START; CORE_END; // 74
    CORE_START; CORE_END; // 75
    CORE_START; CORE_END; // 76
    CORE_START; CORE_END; // 77
    CORE_START; CORE_END; // 78
    CORE_START; CORE_END; // 79
#undef CORE_START
#undef CORE_END
    out[0] += digest[0];
    out[1] += digest[1];
    out[2] += digest[2];
    out[3] += digest[3];
    out[4] += digest[4];
  } while (i >= CHUNK_KUOTA);
#ifdef BYTE_FLIP
  out[0] = imath_flip32(out[0]);
  out[1] = imath_flip32(out[1]);
  out[2] = imath_flip32(out[2]);
  out[3] = imath_flip32(out[3]);
  out[4] = imath_flip32(out[4]);
#endif // BYTE_FLIP
#undef DIGEST
#undef CHUNK
#undef CHUNK_KUOTA
#undef DIGEST_BYTE
#undef CHUNK_BYTE
#undef DIGEST_ALL_BYTE
#undef CHUNK_ALL_BYTE
}
void hash_sha224(uint32 *out, const char *input, uint64 l) {
#define DIGEST          8
#define CHUNK           16
#define DIGEST_BYTE     (sizeof(uint32) * DIGEST)
#define DIGEST_ALL_BYTE (sizeof(uint32) * (DIGEST + 1))
#define CHUNK_BYTE      (sizeof(uint32) * CHUNK)
#define CHUNK_ALL_BYTE  (sizeof(uint32) * (CHUNK + 1))
#define CHUNK_KUOTA     (sizeof(uint32) * (CHUNK - 2))
  // square root first 8 prime from 2
  static const uint32 H[DIGEST] = {0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4};
  union {ubyte b[CHUNK_ALL_BYTE]; uint32 i[CHUNK + 1]; } W = {0};
  uint32 digest[DIGEST + 1] = {0};
  util_memcpy (out, H, DIGEST_BYTE);
  iter i, k = l;
  do {
    // reset digest
    util_memcpy (digest, H, DIGEST_BYTE);
    // put input to chunk W
    i = MIN (k, CHUNK_BYTE);
    k -= i;
    util_memcpy (W.b, input, i);
    input += i;
    if (i < CHUNK_BYTE) W.b[i++] = 0x80; // add last 1 bit
    util_memset (W.b + i, 0, CHUNK_ALL_BYTE - i);
    // flip chunk W
#ifdef BYTE_FLIP
    W.i[ 0] = imath_flip32(W.i[ 0]);
    W.i[ 1] = imath_flip32(W.i[ 1]);
    W.i[ 2] = imath_flip32(W.i[ 2]);
    W.i[ 3] = imath_flip32(W.i[ 3]);
    W.i[ 4] = imath_flip32(W.i[ 4]);
    W.i[ 5] = imath_flip32(W.i[ 5]);
    W.i[ 6] = imath_flip32(W.i[ 6]);
    W.i[ 7] = imath_flip32(W.i[ 7]);
    W.i[ 8] = imath_flip32(W.i[ 8]);
    W.i[ 9] = imath_flip32(W.i[ 9]);
    W.i[10] = imath_flip32(W.i[10]);
    W.i[11] = imath_flip32(W.i[11]);
    W.i[12] = imath_flip32(W.i[12]);
    W.i[13] = imath_flip32(W.i[13]);
    W.i[14] = imath_flip32(W.i[14]);
    W.i[15] = imath_flip32(W.i[15]);
#endif // BYTE_FLIP
    if (i < CHUNK_KUOTA) {
      // put 64 bit length in reverse order from other so, no swap
      W.i[15] = l << 3;
      W.i[14] = l >> 29;
    }
#define SIG_BOT do { \
	digest[8] = digest[5] & digest[6]; \
	digest[8] ^= ~digest[5] & digest[7]; \
  digest[0] += digest[8]; \
	digest[4] += digest[0]; \
	digest[8] = imath_rotr32(digest[1], 2); \
	digest[8] ^= imath_rotr32(digest[1], 13); \
	digest[8] ^= imath_rotr32(digest[1], 22); \
	digest[0] += digest[8]; \
	digest[8] = digest[1] & digest[2]; \
	digest[8] ^= digest[1] & digest[3]; \
	digest[8] ^= digest[2] & digest[3]; \
	digest[0] += digest[8]; \
} while(0)
#define SIG_TOP(I) do { \
	util_memmove(digest + 1, digest, DIGEST_BYTE); \
	digest[0] = imath_rotr32(digest[5], 6); \
	digest[0] ^= imath_rotr32(digest[5], 11); \
	digest[0] ^= imath_rotr32(digest[5], 25); \
	digest[0] += digest[8]; \
	digest[0] += I; \
} while(0)
  	// Add from chunk
#define CHUNK16(X,Y) SIG_TOP(Y); digest[0] += W.i[X]; SIG_BOT
    CHUNK16( 0,0x428a2f98); CHUNK16( 1,0x71374491); CHUNK16( 2,0xb5c0fbcf); CHUNK16( 3,0xe9b5dba5);
    CHUNK16( 4,0x3956c25b); CHUNK16( 5,0x59f111f1); CHUNK16( 6,0x923f82a4); CHUNK16( 7,0xab1c5ed5);
    CHUNK16( 8,0xd807aa98); CHUNK16( 9,0x12835b01); CHUNK16(10,0x243185be); CHUNK16(11,0x550c7dc3);
    CHUNK16(12,0x72be5d74); CHUNK16(13,0x80deb1fe); CHUNK16(14,0x9bdc06a7); CHUNK16(15,0xc19bf174);
#undef CHUNK16
#define CHUNK64(I) do {\
  SIG_TOP(I); \
	W.i[16] = imath_rotr32(W.i[1], 7); \
	W.i[16] ^= imath_rotr32(W.i[1], 18); \
	W.i[16] ^= (W.i[1] >> 3); \
	W.i[16] += W.i[0];\
	digest[8] = imath_rotr32(W.i[14], 17);\
	digest[8] ^= imath_rotr32(W.i[14], 19);\
	digest[8] ^= W.i[14] >> 10;\
	W.i[16] += digest[8];\
	W.i[16] += W.i[9];\
	digest[0] += W.i[16];\
	util_memcpy(W.i, W.i + 1, CHUNK_BYTE);\
  SIG_BOT;\
} while (0)
    // with salsa
    CHUNK64(0xe49b69c1); CHUNK64(0xefbe4786); CHUNK64(0x0fc19dc6); CHUNK64(0x240ca1cc);
    CHUNK64(0x2de92c6f); CHUNK64(0x4a7484aa); CHUNK64(0x5cb0a9dc); CHUNK64(0x76f988da);
    CHUNK64(0x983e5152); CHUNK64(0xa831c66d); CHUNK64(0xb00327c8); CHUNK64(0xbf597fc7);
    CHUNK64(0xc6e00bf3); CHUNK64(0xd5a79147); CHUNK64(0x06ca6351); CHUNK64(0x14292967);
    CHUNK64(0x27b70a85); CHUNK64(0x2e1b2138); CHUNK64(0x4d2c6dfc); CHUNK64(0x53380d13);
    CHUNK64(0x650a7354); CHUNK64(0x766a0abb); CHUNK64(0x81c2c92e); CHUNK64(0x92722c85);
    CHUNK64(0xa2bfe8a1); CHUNK64(0xa81a664b); CHUNK64(0xc24b8b70); CHUNK64(0xc76c51a3);
    CHUNK64(0xd192e819); CHUNK64(0xd6990624); CHUNK64(0xf40e3585); CHUNK64(0x106aa070);
    CHUNK64(0x19a4c116); CHUNK64(0x1e376c08); CHUNK64(0x2748774c); CHUNK64(0x34b0bcb5);
    CHUNK64(0x391c0cb3); CHUNK64(0x4ed8aa4a); CHUNK64(0x5b9cca4f); CHUNK64(0x682e6ff3);
    CHUNK64(0x748f82ee); CHUNK64(0x78a5636f); CHUNK64(0x84c87814); CHUNK64(0x8cc70208);
    CHUNK64(0x90befffa); CHUNK64(0xa4506ceb); CHUNK64(0xbef9a3f7); CHUNK64(0xc67178f2);
#undef CHUNK64
#undef SIG_BOT
#undef SIG_TOP
    out[0] += digest[0];
    out[1] += digest[1];
    out[2] += digest[2];
    out[3] += digest[3];
    out[4] += digest[4];
    out[5] += digest[5];
    out[6] += digest[6];
  } while (i >= CHUNK_KUOTA);
  // flip 2
#ifdef BYTE_FLIP
  out[0] = imath_flip32(out[0]);
  out[1] = imath_flip32(out[1]);
  out[2] = imath_flip32(out[2]);
  out[3] = imath_flip32(out[3]);
  out[4] = imath_flip32(out[4]);
  out[5] = imath_flip32(out[5]);
  out[6] = imath_flip32(out[6]);
#endif // BYTE_FLIP
#undef DIGEST
#undef CHUNK
#undef CHUNK_KUOTA
#undef DIGEST_BYTE
#undef CHUNK_BYTE
#undef DIGEST_ALL_BYTE
#undef CHUNK_ALL_BYTE
}
void hash_sha256(uint32 *out, const char *input, uint64 l) {
#define DIGEST          8
#define CHUNK           16
#define DIGEST_BYTE     (sizeof(uint32) * DIGEST)
#define DIGEST_ALL_BYTE (sizeof(uint32) * (DIGEST + 1))
#define CHUNK_BYTE      (sizeof(uint32) * CHUNK)
#define CHUNK_ALL_BYTE  (sizeof(uint32) * (CHUNK + 1))
#define CHUNK_KUOTA     (sizeof(uint32) * (CHUNK - 2))
  // square root first 8 prime from 2
  static const uint32 H[DIGEST] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  
  union {ubyte b[CHUNK_ALL_BYTE]; uint32 i[CHUNK + 1]; } W = {0};
  uint32 digest[DIGEST + 1] = {0};
  util_memcpy (out, H, DIGEST_BYTE);

  iter i, k = l;
  do {
    // reset digest
    util_memcpy (digest, H, DIGEST_BYTE);
    // put input to chunk W
    i = MIN (k, CHUNK_BYTE);
    k -= i;
    util_memcpy (W.b, input, i);
    input += i;
    if (i < CHUNK_BYTE) W.b[i++] = 0x80; // add last 1 bit
    util_memset (W.b + i, 0, CHUNK_ALL_BYTE - i);
    // flip chunk W
#ifdef BYTE_FLIP
    W.i[ 0] = imath_flip32(W.i[ 0]);
    W.i[ 1] = imath_flip32(W.i[ 1]);
    W.i[ 2] = imath_flip32(W.i[ 2]);
    W.i[ 3] = imath_flip32(W.i[ 3]);
    W.i[ 4] = imath_flip32(W.i[ 4]);
    W.i[ 5] = imath_flip32(W.i[ 5]);
    W.i[ 6] = imath_flip32(W.i[ 6]);
    W.i[ 7] = imath_flip32(W.i[ 7]);
    W.i[ 8] = imath_flip32(W.i[ 8]);
    W.i[ 9] = imath_flip32(W.i[ 9]);
    W.i[10] = imath_flip32(W.i[10]);
    W.i[11] = imath_flip32(W.i[11]);
    W.i[12] = imath_flip32(W.i[12]);
    W.i[13] = imath_flip32(W.i[13]);
    if (i < CHUNK_KUOTA) {
      // put 64 bit length in reverse order from other so, no swap
      W.i[15] = l << 3;
      W.i[14] = l >> 29;
    } else {
      W.i[14] = imath_flip32(W.i[14]);
      W.i[15] = imath_flip32(W.i[15]);
    }
#else
    if (i < CHUNK_KUOTA) {
      // put 64 bit length in reverse order from other so, no swap
      W.i[15] = imath_flip32(l <<  3);
      W.i[14] = imath_flip32(l >> 29);
    }
#endif // BYTE_FLIP
#define SIG_BOT do { \
	digest[8] = digest[5] & digest[6]; \
	digest[8] ^= ~digest[5] & digest[7]; \
  digest[0] += digest[8]; \
	digest[4] += digest[0]; \
	digest[8] = imath_rotr32(digest[1], 2); \
	digest[8] ^= imath_rotr32(digest[1], 13); \
	digest[8] ^= imath_rotr32(digest[1], 22); \
	digest[0] += digest[8]; \
	digest[8] = digest[1] & digest[2]; \
	digest[8] ^= digest[1] & digest[3]; \
	digest[8] ^= digest[2] & digest[3]; \
	digest[0] += digest[8]; \
} while(0)
#define SIG_TOP(I) do { \
	util_memmove(digest + 1, digest, DIGEST_BYTE); \
	digest[0] = imath_rotr32(digest[5], 6); \
	digest[0] ^= imath_rotr32(digest[5], 11); \
	digest[0] ^= imath_rotr32(digest[5], 25); \
	digest[0] += digest[8]; \
	digest[0] += (I); \
} while(0)
  	// Add from chunk
#define CHUNK16(X,Y) SIG_TOP(Y); digest[0] += W.i[X]; SIG_BOT
    CHUNK16( 0,0x428a2f98); CHUNK16( 1,0x71374491); CHUNK16( 2,0xb5c0fbcf); CHUNK16( 3,0xe9b5dba5);
    CHUNK16( 4,0x3956c25b); CHUNK16( 5,0x59f111f1); CHUNK16( 6,0x923f82a4); CHUNK16( 7,0xab1c5ed5);
    CHUNK16( 8,0xd807aa98); CHUNK16( 9,0x12835b01); CHUNK16(10,0x243185be); CHUNK16(11,0x550c7dc3);
    CHUNK16(12,0x72be5d74); CHUNK16(13,0x80deb1fe); CHUNK16(14,0x9bdc06a7); CHUNK16(15,0xc19bf174);
#undef CHUNK16
#define CHUNK64(I) do {\
  SIG_TOP(I); \
	W.i[16] = imath_rotr32(W.i[1], 7); \
	W.i[16] ^= imath_rotr32(W.i[1], 18); \
	W.i[16] ^= (W.i[1] >> 3); \
	W.i[16] += W.i[0];\
	digest[8] = imath_rotr32(W.i[14], 17);\
	digest[8] ^= imath_rotr32(W.i[14], 19);\
	digest[8] ^= W.i[14] >> 10;\
	W.i[16] += digest[8];\
	W.i[16] += W.i[9];\
	digest[0] += W.i[16];\
	util_memcpy(W.i, W.i + 1, CHUNK_BYTE);\
  SIG_BOT;\
} while (0)
    // with salsa
    CHUNK64(0xe49b69c1); CHUNK64(0xefbe4786); CHUNK64(0x0fc19dc6); CHUNK64(0x240ca1cc);
    CHUNK64(0x2de92c6f); CHUNK64(0x4a7484aa); CHUNK64(0x5cb0a9dc); CHUNK64(0x76f988da);
    CHUNK64(0x983e5152); CHUNK64(0xa831c66d); CHUNK64(0xb00327c8); CHUNK64(0xbf597fc7);
    CHUNK64(0xc6e00bf3); CHUNK64(0xd5a79147); CHUNK64(0x06ca6351); CHUNK64(0x14292967);
    CHUNK64(0x27b70a85); CHUNK64(0x2e1b2138); CHUNK64(0x4d2c6dfc); CHUNK64(0x53380d13);
    CHUNK64(0x650a7354); CHUNK64(0x766a0abb); CHUNK64(0x81c2c92e); CHUNK64(0x92722c85);
    CHUNK64(0xa2bfe8a1); CHUNK64(0xa81a664b); CHUNK64(0xc24b8b70); CHUNK64(0xc76c51a3);
    CHUNK64(0xd192e819); CHUNK64(0xd6990624); CHUNK64(0xf40e3585); CHUNK64(0x106aa070);
    CHUNK64(0x19a4c116); CHUNK64(0x1e376c08); CHUNK64(0x2748774c); CHUNK64(0x34b0bcb5);
    CHUNK64(0x391c0cb3); CHUNK64(0x4ed8aa4a); CHUNK64(0x5b9cca4f); CHUNK64(0x682e6ff3);
    CHUNK64(0x748f82ee); CHUNK64(0x78a5636f); CHUNK64(0x84c87814); CHUNK64(0x8cc70208);
    CHUNK64(0x90befffa); CHUNK64(0xa4506ceb); CHUNK64(0xbef9a3f7); CHUNK64(0xc67178f2);
#undef CHUNK64
#undef SIG_BOT
#undef SIG_TOP
    out[0] += digest[0];
    out[1] += digest[1];
    out[2] += digest[2];
    out[3] += digest[3];
    out[4] += digest[4];
    out[5] += digest[5];
    out[6] += digest[6];
    out[7] += digest[7];
  } while (i >= CHUNK_KUOTA);
  // flip 2
#ifdef BYTE_FLIP
  out[0] = imath_flip32(out[0]);
  out[1] = imath_flip32(out[1]);
  out[2] = imath_flip32(out[2]);
  out[3] = imath_flip32(out[3]);
  out[4] = imath_flip32(out[4]);
  out[5] = imath_flip32(out[5]);
  out[6] = imath_flip32(out[6]);
  out[7] = imath_flip32(out[7]);
#endif // BYTE_FLIP
#undef CHUNK
#undef CHUNK_KUOTA
#undef DIGEST
#undef DIGEST_BYTE
#undef CHUNK_BYTE
#undef DIGEST_ALL_BYTE
#undef CHUNK_ALL_BYTE
}
void hash_sha384(uint64 *out, const char *input, uint64 l) {
#define DIGEST          8
#define CHUNK           16
#define DIGEST_BYTE     sizeof(uint64) * DIGEST
#define DIGEST_ALL_BYTE sizeof(uint64) * 9
#define CHUNK_BYTE      sizeof(uint64) * CHUNK
#define CHUNK_ALL_BYTE  sizeof(uint64) * 17
#define CHUNK_KUOTA     sizeof(uint64) * (CHUNK - 1)
#define OUT_BYTE        sizeof(uint64) * 6
  // square root first 8 prime from 2
  static const uint64 H[DIGEST] = {0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL, 0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL, 0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL};
  union {ubyte b[CHUNK_ALL_BYTE]; uint64 i[CHUNK + 1]; } W = {0};
  uint64 digest[DIGEST + 1] = {0};
  // init
  util_memcpy (out, H, OUT_BYTE);
  iter i, k = l;
  do {
    // reset digest
    util_memcpy (digest, H, DIGEST_BYTE);
    // put input to chunk W
    i = MIN (k, CHUNK_BYTE);
    k -= i;
    util_memcpy (W.b, input, i);
    input += i;
    if (i < CHUNK_BYTE) W.b[i++] = 0x80; // add last 1 bit
    util_memset (W.b + i, 0, (CHUNK_ALL_BYTE - i));
    // flip chunk W
#ifdef BYTE_FLIP
    W.i[ 0] = imath_flip64(W.i[ 0]);
    W.i[ 1] = imath_flip64(W.i[ 1]);
    W.i[ 2] = imath_flip64(W.i[ 2]);
    W.i[ 3] = imath_flip64(W.i[ 3]);
    W.i[ 4] = imath_flip64(W.i[ 4]);
    W.i[ 5] = imath_flip64(W.i[ 5]);
    W.i[ 6] = imath_flip64(W.i[ 6]);
    W.i[ 7] = imath_flip64(W.i[ 7]);
    W.i[ 8] = imath_flip64(W.i[ 8]);
    W.i[ 9] = imath_flip64(W.i[ 9]);
    W.i[10] = imath_flip64(W.i[10]);
    W.i[11] = imath_flip64(W.i[11]);
    W.i[12] = imath_flip64(W.i[12]);
    W.i[13] = imath_flip64(W.i[13]);
    W.i[14] = imath_flip64(W.i[14]);
    if (i < CHUNK_KUOTA) {
      // put 128 bit length in reverse order from other so, no swap
      W.i[15] = CAST(uint64)(l <<  3);
    } else {
      W.i[15] = imath_flip64(W.i[15]);
    }
#else
    if (i < CHUNK_KUOTA) {
      // put 128 bit length in reverse order from other so, no swap
      W.i[15] = imath_flip64(l <<  3);
    }
#endif // BYTE_FLIP
#define SIG_BOT do { \
	digest[8] = digest[5] & digest[6]; \
	digest[8] ^= ~digest[5] & digest[7]; \
  digest[0] += digest[8]; \
	digest[4] += digest[0]; \
	digest[8] = imath_rotr64(digest[1], 28); \
	digest[8] ^= imath_rotr64(digest[1], 34); \
	digest[8] ^= imath_rotr64(digest[1], 39); \
	digest[0] += digest[8]; \
	digest[8] = digest[1] & digest[2]; \
	digest[8] ^= digest[1] & digest[3]; \
	digest[8] ^= digest[2] & digest[3]; \
	digest[0] += digest[8]; \
} while(0)
#define SIG_TOP(I) do { \
	util_memmove(digest + 1, digest, DIGEST_BYTE); \
	digest[0] = imath_rotr64(digest[5], 14); \
	digest[0] ^= imath_rotr64(digest[5], 18); \
	digest[0] ^= imath_rotr64(digest[5], 41); \
	digest[0] += digest[8]; \
	digest[0] += I; \
} while(0)
  	// Add from chunk
#define CHUNK16(X,Y) SIG_TOP(Y); digest[0] += W.i[X]; SIG_BOT
    CHUNK16( 0,0x428a2f98d728ae22ULL); CHUNK16( 1,0x7137449123ef65cdULL);
    CHUNK16( 2,0xb5c0fbcfec4d3b2fULL); CHUNK16( 3,0xe9b5dba58189dbbcULL);
    CHUNK16( 4,0x3956c25bf348b538ULL); CHUNK16( 5,0x59f111f1b605d019ULL);
    CHUNK16( 6,0x923f82a4af194f9bULL); CHUNK16( 7,0xab1c5ed5da6d8118ULL);
    CHUNK16( 8,0xd807aa98a3030242ULL); CHUNK16( 9,0x12835b0145706fbeULL);
    CHUNK16(10,0x243185be4ee4b28cULL); CHUNK16(11,0x550c7dc3d5ffb4e2ULL);
    CHUNK16(12,0x72be5d74f27b896fULL); CHUNK16(13,0x80deb1fe3b1696b1ULL);
    CHUNK16(14,0x9bdc06a725c71235ULL); CHUNK16(15,0xc19bf174cf692694ULL);
#undef CHUNK16
#define CHUNK80(I) do {\
  SIG_TOP(I); \
	W.i[16] = imath_rotr64(W.i[1], 1); \
	W.i[16] ^= imath_rotr64(W.i[1], 8); \
	W.i[16] ^= (W.i[1] >> 7); \
	W.i[16] += W.i[0];\
	digest[8] = imath_rotr64(W.i[14], 19);\
	digest[8] ^= imath_rotr64(W.i[14], 61);\
	digest[8] ^= W.i[14] >> 6;\
	W.i[16] += digest[8];\
	W.i[16] += W.i[9];\
	digest[0] += W.i[16];\
	util_memcpy(W.i, W.i + 1, CHUNK_BYTE);\
  SIG_BOT;\
} while (0)
    // with salsa
    CHUNK80(0xe49b69c19ef14ad2ULL); CHUNK80(0xefbe4786384f25e3ULL);
    CHUNK80(0x0fc19dc68b8cd5b5ULL); CHUNK80(0x240ca1cc77ac9c65ULL);
    CHUNK80(0x2de92c6f592b0275ULL); CHUNK80(0x4a7484aa6ea6e483ULL);
    CHUNK80(0x5cb0a9dcbd41fbd4ULL); CHUNK80(0x76f988da831153b5ULL);
    CHUNK80(0x983e5152ee66dfabULL); CHUNK80(0xa831c66d2db43210ULL);
    CHUNK80(0xb00327c898fb213fULL); CHUNK80(0xbf597fc7beef0ee4ULL);
    CHUNK80(0xc6e00bf33da88fc2ULL); CHUNK80(0xd5a79147930aa725ULL);
    CHUNK80(0x06ca6351e003826fULL); CHUNK80(0x142929670a0e6e70ULL);
    CHUNK80(0x27b70a8546d22ffcULL); CHUNK80(0x2e1b21385c26c926ULL);
    CHUNK80(0x4d2c6dfc5ac42aedULL); CHUNK80(0x53380d139d95b3dfULL);
    CHUNK80(0x650a73548baf63deULL); CHUNK80(0x766a0abb3c77b2a8ULL);
    CHUNK80(0x81c2c92e47edaee6ULL); CHUNK80(0x92722c851482353bULL);
    CHUNK80(0xa2bfe8a14cf10364ULL); CHUNK80(0xa81a664bbc423001ULL);
    CHUNK80(0xc24b8b70d0f89791ULL); CHUNK80(0xc76c51a30654be30ULL);
    CHUNK80(0xd192e819d6ef5218ULL); CHUNK80(0xd69906245565a910ULL);
    CHUNK80(0xf40e35855771202aULL); CHUNK80(0x106aa07032bbd1b8ULL);
    CHUNK80(0x19a4c116b8d2d0c8ULL); CHUNK80(0x1e376c085141ab53ULL);
    CHUNK80(0x2748774cdf8eeb99ULL); CHUNK80(0x34b0bcb5e19b48a8ULL);
    CHUNK80(0x391c0cb3c5c95a63ULL); CHUNK80(0x4ed8aa4ae3418acbULL);
    CHUNK80(0x5b9cca4f7763e373ULL); CHUNK80(0x682e6ff3d6b2b8a3ULL);
    CHUNK80(0x748f82ee5defb2fcULL); CHUNK80(0x78a5636f43172f60ULL);
    CHUNK80(0x84c87814a1f0ab72ULL); CHUNK80(0x8cc702081a6439ecULL);
    CHUNK80(0x90befffa23631e28ULL); CHUNK80(0xa4506cebde82bde9ULL);
    CHUNK80(0xbef9a3f7b2c67915ULL); CHUNK80(0xc67178f2e372532bULL);
    CHUNK80(0xca273eceea26619cULL); CHUNK80(0xd186b8c721c0c207ULL);
    CHUNK80(0xeada7dd6cde0eb1eULL); CHUNK80(0xf57d4f7fee6ed178ULL);
    CHUNK80(0x06f067aa72176fbaULL); CHUNK80(0x0a637dc5a2c898a6ULL);
    CHUNK80(0x113f9804bef90daeULL); CHUNK80(0x1b710b35131c471bULL);
    CHUNK80(0x28db77f523047d84ULL); CHUNK80(0x32caab7b40c72493ULL);
    CHUNK80(0x3c9ebe0a15c9bebcULL); CHUNK80(0x431d67c49c100d4cULL);
    CHUNK80(0x4cc5d4becb3e42b6ULL); CHUNK80(0x597f299cfc657e2aULL);
    CHUNK80(0x5fcb6fab3ad6faecULL); CHUNK80(0x6c44198c4a475817ULL);
#undef CHUNK80
#undef SIG_BOT
#undef SIG_TOP
    out[0] += digest[0];
    out[1] += digest[1];
    out[2] += digest[2];
    out[3] += digest[3];
    out[4] += digest[4];
    out[5] += digest[5];
    // out[6] += digest[6];
    // out[7] += digest[7];
  } while (i >= CHUNK_KUOTA);
  // flip 2
#ifdef BYTE_FLIP
  out[0] = imath_flip64(out[0]);
  out[1] = imath_flip64(out[1]);
  out[2] = imath_flip64(out[2]);
  out[3] = imath_flip64(out[3]);
  out[4] = imath_flip64(out[4]);
  out[5] = imath_flip64(out[5]);
  // out[6] = imath_flip64(out[6]);
  // out[7] = imath_flip64(out[7]);
#endif // BYTE_FLIP
#undef DIGEST
#undef CHUNK
#undef CHUNK_KUOTA
#undef DIGEST_BYTE
#undef CHUNK_BYTE
#undef DIGEST_ALL_BYTE
#undef CHUNK_ALL_BYTE
#undef OUT_BYTE
}
void hash_sha512(uint64 *out, const char *input, uint64 l) {
#define DIGEST          8
#define CHUNK           16
#define DIGEST_BYTE     sizeof(uint64) * DIGEST
#define DIGEST_ALL_BYTE sizeof(uint64) * 9
#define CHUNK_BYTE      sizeof(uint64) * CHUNK
#define CHUNK_ALL_BYTE  sizeof(uint64) * 17
#define CHUNK_KUOTA     sizeof(uint64) * (CHUNK - 1)
  // square root first 8 prime from 2
  static const uint64 H[DIGEST] = {0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL};
  union {ubyte b[CHUNK_ALL_BYTE]; uint64 i[CHUNK + 1]; } W = {0};
  uint64 digest[DIGEST + 1] = {0};
  // init
  util_memcpy (out, H, DIGEST_BYTE);

  iter i, k = l;
  do {
    // reset digest
    util_memcpy (digest, H, DIGEST_BYTE);
    // put input to chunk W
    i = MIN (k, CHUNK_BYTE);
    k -= i;
    util_memcpy (W.b, input, i);
    input += i;
    if (i < CHUNK_BYTE) W.b[i++] = 0x80; // add last 1 bit
    util_memset (W.b + i, 0, (CHUNK_ALL_BYTE - i));
    // flip chunk W
#ifdef BYTE_FLIP
    W.i[ 0] = imath_flip64(W.i[ 0]);
    W.i[ 1] = imath_flip64(W.i[ 1]);
    W.i[ 2] = imath_flip64(W.i[ 2]);
    W.i[ 3] = imath_flip64(W.i[ 3]);
    W.i[ 4] = imath_flip64(W.i[ 4]);
    W.i[ 5] = imath_flip64(W.i[ 5]);
    W.i[ 6] = imath_flip64(W.i[ 6]);
    W.i[ 7] = imath_flip64(W.i[ 7]);
    W.i[ 8] = imath_flip64(W.i[ 8]);
    W.i[ 9] = imath_flip64(W.i[ 9]);
    W.i[10] = imath_flip64(W.i[10]);
    W.i[11] = imath_flip64(W.i[11]);
    W.i[12] = imath_flip64(W.i[12]);
    W.i[13] = imath_flip64(W.i[13]);
    W.i[14] = imath_flip64(W.i[14]);
    if (i < CHUNK_KUOTA) {
      // put 128 bit length in reverse order from other so, no swap
      W.i[15] = CAST(uint64)(l <<  3);
    } else {
      W.i[15] = imath_flip64(W.i[15]);
    }
#else
    if (i < CHUNK_KUOTA) {
      // put 128 bit length in reverse order from other so, no swap
      W.i[15] = imath_flip64(l <<  3);
    }
#endif // BYTE_FLIP
#define SIG_BOT do { \
	digest[8] = digest[5] & digest[6]; \
	digest[8] ^= ~digest[5] & digest[7]; \
  digest[0] += digest[8]; \
	digest[4] += digest[0]; \
	digest[8] = imath_rotr64(digest[1], 28); \
	digest[8] ^= imath_rotr64(digest[1], 34); \
	digest[8] ^= imath_rotr64(digest[1], 39); \
	digest[0] += digest[8]; \
	digest[8] = digest[1] & digest[2]; \
	digest[8] ^= digest[1] & digest[3]; \
	digest[8] ^= digest[2] & digest[3]; \
	digest[0] += digest[8]; \
} while(0)
#define SIG_TOP(I) do { \
	util_memmove(digest + 1, digest, DIGEST_BYTE); \
	digest[0] = imath_rotr64(digest[5], 14); \
	digest[0] ^= imath_rotr64(digest[5], 18); \
	digest[0] ^= imath_rotr64(digest[5], 41); \
	digest[0] += digest[8]; \
	digest[0] += I; \
} while(0)
  	// Add from chunk
#define CHUNK16(X,Y) SIG_TOP(Y); digest[0] += W.i[X]; SIG_BOT
    CHUNK16( 0,0x428a2f98d728ae22ULL); CHUNK16( 1,0x7137449123ef65cdULL);
    CHUNK16( 2,0xb5c0fbcfec4d3b2fULL); CHUNK16( 3,0xe9b5dba58189dbbcULL);
    CHUNK16( 4,0x3956c25bf348b538ULL); CHUNK16( 5,0x59f111f1b605d019ULL);
    CHUNK16( 6,0x923f82a4af194f9bULL); CHUNK16( 7,0xab1c5ed5da6d8118ULL);
    CHUNK16( 8,0xd807aa98a3030242ULL); CHUNK16( 9,0x12835b0145706fbeULL);
    CHUNK16(10,0x243185be4ee4b28cULL); CHUNK16(11,0x550c7dc3d5ffb4e2ULL);
    CHUNK16(12,0x72be5d74f27b896fULL); CHUNK16(13,0x80deb1fe3b1696b1ULL);
    CHUNK16(14,0x9bdc06a725c71235ULL); CHUNK16(15,0xc19bf174cf692694ULL);
#undef CHUNK16
#define CHUNK80(I) do {\
  SIG_TOP(I); \
	W.i[16] = imath_rotr64(W.i[1], 1); \
	W.i[16] ^= imath_rotr64(W.i[1], 8); \
	W.i[16] ^= (W.i[1] >> 7); \
	W.i[16] += W.i[0];\
	digest[8] = imath_rotr64(W.i[14], 19);\
	digest[8] ^= imath_rotr64(W.i[14], 61);\
	digest[8] ^= W.i[14] >> 6;\
	W.i[16] += digest[8];\
	W.i[16] += W.i[9];\
	digest[0] += W.i[16];\
	util_memcpy(W.i, W.i + 1, CHUNK_BYTE);\
  SIG_BOT;\
} while (0)
    // with salsa
    CHUNK80(0xe49b69c19ef14ad2ULL); CHUNK80(0xefbe4786384f25e3ULL);
    CHUNK80(0x0fc19dc68b8cd5b5ULL); CHUNK80(0x240ca1cc77ac9c65ULL);
    CHUNK80(0x2de92c6f592b0275ULL); CHUNK80(0x4a7484aa6ea6e483ULL);
    CHUNK80(0x5cb0a9dcbd41fbd4ULL); CHUNK80(0x76f988da831153b5ULL);
    CHUNK80(0x983e5152ee66dfabULL); CHUNK80(0xa831c66d2db43210ULL);
    CHUNK80(0xb00327c898fb213fULL); CHUNK80(0xbf597fc7beef0ee4ULL);
    CHUNK80(0xc6e00bf33da88fc2ULL); CHUNK80(0xd5a79147930aa725ULL);
    CHUNK80(0x06ca6351e003826fULL); CHUNK80(0x142929670a0e6e70ULL);
    CHUNK80(0x27b70a8546d22ffcULL); CHUNK80(0x2e1b21385c26c926ULL);
    CHUNK80(0x4d2c6dfc5ac42aedULL); CHUNK80(0x53380d139d95b3dfULL);
    CHUNK80(0x650a73548baf63deULL); CHUNK80(0x766a0abb3c77b2a8ULL);
    CHUNK80(0x81c2c92e47edaee6ULL); CHUNK80(0x92722c851482353bULL);
    CHUNK80(0xa2bfe8a14cf10364ULL); CHUNK80(0xa81a664bbc423001ULL);
    CHUNK80(0xc24b8b70d0f89791ULL); CHUNK80(0xc76c51a30654be30ULL);
    CHUNK80(0xd192e819d6ef5218ULL); CHUNK80(0xd69906245565a910ULL);
    CHUNK80(0xf40e35855771202aULL); CHUNK80(0x106aa07032bbd1b8ULL);
    CHUNK80(0x19a4c116b8d2d0c8ULL); CHUNK80(0x1e376c085141ab53ULL);
    CHUNK80(0x2748774cdf8eeb99ULL); CHUNK80(0x34b0bcb5e19b48a8ULL);
    CHUNK80(0x391c0cb3c5c95a63ULL); CHUNK80(0x4ed8aa4ae3418acbULL);
    CHUNK80(0x5b9cca4f7763e373ULL); CHUNK80(0x682e6ff3d6b2b8a3ULL);
    CHUNK80(0x748f82ee5defb2fcULL); CHUNK80(0x78a5636f43172f60ULL);
    CHUNK80(0x84c87814a1f0ab72ULL); CHUNK80(0x8cc702081a6439ecULL);
    CHUNK80(0x90befffa23631e28ULL); CHUNK80(0xa4506cebde82bde9ULL);
    CHUNK80(0xbef9a3f7b2c67915ULL); CHUNK80(0xc67178f2e372532bULL);
    CHUNK80(0xca273eceea26619cULL); CHUNK80(0xd186b8c721c0c207ULL);
    CHUNK80(0xeada7dd6cde0eb1eULL); CHUNK80(0xf57d4f7fee6ed178ULL);
    CHUNK80(0x06f067aa72176fbaULL); CHUNK80(0x0a637dc5a2c898a6ULL);
    CHUNK80(0x113f9804bef90daeULL); CHUNK80(0x1b710b35131c471bULL);
    CHUNK80(0x28db77f523047d84ULL); CHUNK80(0x32caab7b40c72493ULL);
    CHUNK80(0x3c9ebe0a15c9bebcULL); CHUNK80(0x431d67c49c100d4cULL);
    CHUNK80(0x4cc5d4becb3e42b6ULL); CHUNK80(0x597f299cfc657e2aULL);
    CHUNK80(0x5fcb6fab3ad6faecULL); CHUNK80(0x6c44198c4a475817ULL);
#undef CHUNK80
#undef SIG_BOT
#undef SIG_TOP
    out[0] += digest[0];
    out[1] += digest[1];
    out[2] += digest[2];
    out[3] += digest[3];
    out[4] += digest[4];
    out[5] += digest[5];
    out[6] += digest[6];
    out[7] += digest[7];
  } while (i >= CHUNK_KUOTA);
  // flip 2
#ifdef BYTE_FLIP
  out[0] = imath_flip64(out[0]);
  out[1] = imath_flip64(out[1]);
  out[2] = imath_flip64(out[2]);
  out[3] = imath_flip64(out[3]);
  out[4] = imath_flip64(out[4]);
  out[5] = imath_flip64(out[5]);
  out[6] = imath_flip64(out[6]);
  out[7] = imath_flip64(out[7]);
#endif // BYTE_FLIP
#undef DIGEST
#undef CHUNK
#undef CHUNK_KUOTA
#undef DIGEST_BYTE
#undef CHUNK_BYTE
#undef DIGEST_ALL_BYTE
#undef CHUNK_ALL_BYTE
}
inline void hash_crc32_start(uint32 *crc) {
  *crc = 0xffffffff;
}
#define POLYNOMIAL 0xEDB88320
void hash_crc32_append(uint32 *crc, const byte buffer) {
  uint32 table = (buffer ^ (*crc & 0xff)) & 0xff;
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  table = (table >> 1) ^ ((table & 1) * POLYNOMIAL);
  *crc = (*crc >> 8) ^ table;
}
void hash_crc32_appends(uint32 *crc, const byte *buffer, iter n) {
  for (iter i = 0; i < n; ++i)
    hash_crc32_append(crc, buffer[i]);
}
#undef POLYNOMIAL
inline void hash_crc32_end (uint *crc) {
  *crc ^= 0xffffffff;
}
