/* *****************************************************************************
 * common.c v0.0.0000
 * 
 * Provide basic function and constant for all source code
 * 
 * 
 * 
 * *****************************************************************************/

#include "common.h"

// should be exponent of 2
#define STRING_CAP_ROUND 4
#define STRING_CAP_MASK  3


#if !defined(NO_STDMATH)
  #include <math.h>
#else
  #define M_PI_INV   0.3183098861838f // 1/π
  #define M_PI2_INV  0.1591549430919f // 1/2π
  #define M_PI_OVER4 0.7853981633975f // π/4
  #define M_PI       3.1415926535898f // π
  #define M_PI_HALF  1.5707963267949f // π/2
  #define M_PI_3HALF 4.7123889803847f // 3π/2
  #define M_PI2      6.2831853071796f // 2π
 
  #define M_LN2      0.6931471805700f // ln(2)
  #define M_LN2_INV  1.4426950408890f // 1/ln(2)

#endif // NO_STDMATH
#ifdef __GNUC__
#  include <byteswap.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
// #include <string.h>
#include <time.h>
#include <alloca.h>
// #define FASTER_MATH //  no use when STD_MATH defined
#if defined(_WIN32) || defined(_WIN64)
  #include <intrin.h>
  #pragma intrinsic(__rdtsc)
// win32/64 wide character support
int convert_wchar_to_utf8(char *buffer, iter bufferlen, const wchar_t *input) {
  return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, NULL, NULL);
}
#endif

#define CHAR_WHITESPACE ' '


/* ================================
 *  Standar Utility Function
 * ================================
 */
void *util_alloca(iter bytes) {
#if BLTN(__builtin_alloca)
  return __builtin_alloca(bytes);
#else
  return alloca(bytes);
#endif
}
void *util_malloc(iter bytes) {
#if BLTN(__builtin_malloc)
  return __builtin_malloc(bytes);
#else
  return malloc(bytes);
#endif
}
void *util_calloc(iter n, iter bytes) {
#if BLTN(__builtin_calloc)
  return __builtin_calloc(n, bytes);
#else
  return calloc(n, bytes);
#endif
}
void *util_realloc(void *a, iter bytes) {
#if BLTN(__builtin_realloc)
  return __builtin_realloc(a, bytes);
#else
  return realloc(a, bytes);
#endif
}
void util_memfree(void *p) {
#if BLTN(__builtin_free)
  __builtin_free(p);
#else
  free(p);
#endif
}
void util_memswap(void *a, void *b, iter bytes) {
  if (a == b) return;
  byte *A = CAST(byte*)a, *B = CAST(byte*)b;
  for (iter i = 0; i < bytes; ++i) {
    A[i] ^= B[i];
    B[i] ^= A[i];
    A[i] ^= B[i];
  }
}
void util_memflip(void *a, iter n) {
  byte *A = CAST(byte*)a;
  for (iter i = 0, j = n - 1; i < j; ++i, --j) {
    A[i] ^= A[j];
    A[j] ^= A[i];
    A[i] ^= A[j];
  }
}
void util_memcpy (void *dst, const void *src,iter bytes) {
#if BLTN(__builtin_memcpy)
  __builtin_memcpy(dst,src,bytes);
#else
  memcpy(dst,src,bytes);
#endif
}
int util_memcmp (const void *a, const void *b,iter bytes) {
#if BLTN(__builtin_memcmp)
  return __builtin_memcmp(a,b,bytes);
#else
  return memcmp(a,b,bytes);
#endif
}
void util_memmove(void *dst, void *src,iter bytes) {
#if BLTN(__builtin_memmove)
  __builtin_memmove(dst,src,bytes);
#else
  memmove(dst,src,bytes);
#endif
}
void util_memset(void *dst, int src, iter bytes) {
#if BLTN(__builtin_memset)
  __builtin_memset(dst,src,bytes);
#else
  memset(dst,src,bytes);
#endif
}
iter util_strlen(const char *str) {
#if BLTN(__builtin_strlen)
  return str ? __builtin_strlen(str) : 0;
#else
  return str ? strlen(str) : 0;
#endif
}

char *util_strncpy(char *dst, const char *src, iter n) {
#if BLTN(__builtin_strncpy)
  return __builtin_strncpy(dst, src, n);
#else
  return strncpy(dst, src, n);
#endif
}
char *util_strcpy (char *dst, const char *src) {
#if BLTN(__builtin_strcpy)
  return __builtin_strcpy(dst, src);
#else
  return strcpy(dst, src);
#endif
}
iter util_clz(ulong x) {
#if BLTN(__builtin_clzl)
  return __builtin_clzl(x);
#else
  iter r = sizeof(ulong) * 8;
  while (x) --r, x >>= 1;
  return r;
#endif
}
iter util_bitlead(ulong x) {
#if BLTN(__builtin_clzl)
  return sizeof(ulong) * 8 - __builtin_clzl(x);
#else
  iter r = 0;
  while (x) ++r, x >>= 1;
  return r;
#endif
}

/* ================================
 *  File Functions
 * ================================
 */
inline FILE *file_open(const char *filename, const char *mode) {
  FILE *f;
#if defined(_WIN32) && defined(STBIW_WINDOWS_UTF8)
  wchar_t wMode[64];
  wchar_t wFilename[1024];
  if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, filename, -1, wFilename, sizeof(wFilename)/sizeof(*wFilename))) return 0;
  if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, mode, -1, wMode, sizeof(wMode)/sizeof(*wMode))) return 0;
  #if defined(_MSC_VER) && _MSC_VER >= 1400
  if (0 != _wfopen_s(&f, wFilename, wMode)) f = 0;
  #else
  f = _wfopen(wFilename, wMode);
  #endif
  #elif defined(_MSC_VER) && _MSC_VER >= 1400
  if (0 != fopen_s(&f, filename, mode))
    f=0;
  #else
  f = fopen(filename, mode);
  #endif
  return f;
}
inline void file_rewind(FILE *file) {
  rewind(file);
}
inline int file_read(void *buffer, iter n, FILE *file) {
  return fread(buffer, 1, n, file);
}
inline void file_seek(int n, FILE *file) {
  fseek(file, n, SEEK_CUR);
}
inline int file_write(const void *buffer,iter n, FILE *file) {
  return fwrite(buffer, 1, n, file);
}
inline bool file_eof(FILE *file) {
  return feof(file) || ferror(file);
}
inline void file_close(FILE *file) {
  fclose(file);
}

/* ================================
 *  IMath Function
 * ================================
 */


#if defined(NO_STDMATH) || !(BLTN(__builtin_sinf) || BLTN(__builtin_cosf))
// src https://gist.githubusercontent.com/Sam-Belliveau/9c2e5a7584ec9900831877b3155f6f16/raw/238a72e91f46ad616fbaedddec30f163dbb8cb85/fast_trig.h
static inline float imath__cosine_wave(float x) {
  x *= M_PI2;
  x = imath_fma(-M_PI_INV * x, imath_fabs(x), x);
#ifdef FASTER_MATH
  x = imath_fma(0.3451140202480f * x, imath_fabs(x), x);
#else
  x = imath_fma(0.25f * x, imath_fabs(x), x);
  x = imath_fma(0.0684571845286f * x, imath_fabs(x), x);
#endif //FASTER_MATH
  return x;
}
#endif // builtin trig

inline uint imath_iabs (int a) {
#if BLTN(__builtin_abs)
  return __builtin_abs(a);
#elif // libc
  return abs(a);
#endif // builtin
}

inline bool imath_isnormal(float a) {
#if !defined(NO_STDMATH)
  return !!isnormal(a);
#else
  return (*(CAST(int*)&a) & 0x7fe00000) != 0x7fe00000;
#endif // builtin
}
inline float imath_fabs  (float a) {
#if BLTN(__builtin_fabsf)
  return __builtin_fabsf(a);
#elif !defined(NO_STDMATH)
  return fabsf(a);
#else
	(*CAST(int*)&a) &= 0x7fffffff;
	return a;
#endif // builtin
}
inline float imath_fmax(float a, float b) {
#if BLTN(__builtin_fmaxf)
  return __builtin_fmaxf(a,b);
#elif !defined(NO_STDMATH)
  return fmaxf(a,b);
#else
  return MAX(a,b);
#endif // builtin
}
inline float imath_fmin(float a, float b) {
#if BLTN(__builtin_fminf)
  return __builtin_fminf(a,b);
#elif !defined(NO_STDMATH)
  return fminf(a,b);
#else
  return MIN(a,b);
#endif // builtin
}
inline float imath_round(float a) {
#if BLTN(__builtin_roundf)
  return __builtin_roundf(a);
#elif !defined(NO_STDMATH)
  return roundf(a);
#else
  return CAST(float)CAST(int)(a + 0.5f - (a < 0));
#endif // builtin
}
inline float imath_floor(float a) {
#if BLTN(__builtin_floorf)
  return __builtin_floorf(a);
#elif !defined(NO_STDMATH)
  return floorf(a);
#else
  return CAST(float)CAST(int)(a - (a < 0));
#endif // builtin
}
inline float imath_frexp(float a,int *i) {
#if BLTN(__builtin_frexpf)
  return __builtin_frexpf(a,i);
#elif !defined(NO_STDMATH)
  return frexpf(a,i);
#else
  *i = CAST(int)(a - (a < 0));
  return a - CAST(float)*i;
#endif // builtin
}
inline float imath_ceil(float a) {
#if BLTN(__builtin_ceilf)
  return __builtin_ceilf(a);
#elif !defined(NO_STDMATH)
  return ceilf(a);
#else
  return CAST(float)CAST(int)(a + (a >= 0));
#endif // builtin
}
inline float imath_fma(float a, float b, float c) {
#if BLTN(__builtin_fmaf)
  return __builtin_fmaf(a,b,c);
#elif !defined(NO_STDMATH)
  return fmaf(a,b,c);
#else
  return a * b + c;
#endif // builtin
}
inline float imath_sin(float x) {
#if BLTN(__builtin_sinf)
  return __builtin_sinf(x);
#elif !defined(NO_STDMATH)
  return sinf(x);
#else
  x *= M_PI2_INV;
  x -= imath_floor(x + 0.5f);
  return imath__cosine_wave(x);
#endif // builtin
}
inline float imath_cos(float x) {
#if BLTN(__builtin_cosf)
  return __builtin_cosf(x);
#elif !defined(NO_STDMATH)
  return cosf(x);
#else
  x *= M_PI2_INV;
  x -= imath_floor(x + 0.75f) - 0.25f;
  return imath__cosine_wave(x);
#endif // builtin
}
inline float imath_tan(float x) {
#if BLTN(__builtin_tanf)
  return __builtin_tanf(x);
#elif !defined(NO_STDMATH)
  return tanf(x);
#else
  /*  a = (x/π - floor(x/π + 0.5))*π/2
   *  tan(a) = a + (8 - π) * a / (2π - 8|a|)
   *
   *
   */
  x *= M_PI_INV;
  x -= imath_floor(x + 0.5f);
  x *= 1.45649292289f + 0.63587677f / (0.5f - imath_fabs(x));
  return x;
#endif // builtin
}
inline float imath_asin(float x) {
#if BLTN(__builtin_asinf)
  return __builtin_asinf(x);
#elif !defined(NO_STDMATH)
  return asinf(x);
#else
  if (x > 1.0f || x < -1.0f) {
    int *i = CAST(int*)&x;
    *i = 0x7f800000; // NAN
  } else {
    x *= imath_fma(0.3333333333333f, imath_fabs(x), 0.1666666666667f);
    x *= M_PI;
  }
  return x;
#endif // builtin
}
inline float imath_acos(float x) {
#if BLTN(__builtin_acosf)
  return __builtin_acosf(x);
#elif !defined(NO_STDMATH)
  return acosf(x);
#else
  if (x > 1.0f || x < -1.0f) {
    int *i = CAST(int*)&x;
    *i = 0x7f800000; // NAN 
  } else {
    x *= imath_fma(0.3333333333333f, imath_fabs(x), 0.1666666666667f);
    x *= -M_PI;
    x -= M_PI_HALF;
  }
  return x;
#endif // builtin
}
inline float imath_atan(float x) {
#if BLTN(__builtin_atanf)
  return __builtin_atanf(x);
#elif !defined(NO_STDMATH)
  return atanf(x);
#else
  x *= -0.125f;
  x *= (imath_fabs(x) - 2.375f) / (imath_fabs(x) + 0.25f);
  x *= M_PI * 0.34715062811f;
  return x;
#endif // builtin
}
// idk what is right order argument of atan 2? y,x or x,y
inline float imath_atan2(float x, float y) {
#if BLTN(__builtin_atan2f)
  return __builtin_atan2f(x,y);
#elif !defined(NO_STDMATH)
  return atan2f(x,y);
#else
  return imath_atan(x/y);
#endif // builtin
}
/*   b     b*ln(a)
 *  a  => e
 *
 */
inline float imath_pow(float a,float b) {
#if BLTN(__builtin_powf)
  return __builtin_powf(a,b);
#elif !defined(NO_STDMATH)
  return powf(a,b);
#else
  return imath_exp(b * imath_log(a));
#endif // builtin
}
/*
 * stolen from https://gist.github.com/LingDong-/7e4c4cae5cbbc44400a05fba65f06f23
 *       x  =    m  *   2^p
 * => ln(x) = ln(m) + ln(2)p
 * exp = 127 for a = 1, 
 * so 2^(exp-127) is the multiplier
 */
inline float imath_log(float a) {
#if BLTN(__builtin_logf)
  return __builtin_logf(a);
#elif !defined(NO_STDMATH)
  return logf(a);
#else
  // evil floating point bit level hacking
  uint *bx = CAST(uint*)&a;
  // extract exp, since a>0, sign bit must be 0
  float t = CAST(float)((*bx >> 23) - 127);
  // get mantissa
  *bx &= 8388607;
  // set exp
  *bx |= 1065353216;
  float out;
#  ifdef FASTER_MATH
  out = imath_fma(-0.10969f, a, 0.729104f);
  out = imath_fma(out, a,-2.11263f);
  out = imath_fma(out, a,-1.49278f);
#  else
  out = imath_fma(0.056570851f, a, -0.44717955f);
  out = imath_fma(out, a,1.4699568f);
  out = imath_fma(out, a,-2.8212026f);
  out = imath_fma(out, a,-1.7417939f);
#  endif // FASTER_MATH
  return imath_fma(M_LN2, t, out);
}
#endif // builtin
}
/* same logic as imath_log
 *        a  =    m  *2^p
 * => lg2(a) =lg2(m) + p
 * => lg2(a) = ln(m)/ln(2) + p
 */
inline float imath_log2(float a) {
#if BLTN(__builtin_log2f)
  return __builtin_log2f(a);
#elif !defined(NO_STDMATH)
  return log2f(a);
#else
  uint *bx = CAST(uint*)&a;
  // extract exp, since a>0, sign bit must be 0
  float t = CAST(float)((*bx >> 23) - 127);
  // get mantissa
  *bx &= 8388607;
  // set exp
  *bx |= 1065353216;
  float out;
#  ifdef FASTER_MATH
  out = imath_fma(-0.10969f, a, 0.729104f);
  out = imath_fma(out, a, -2.11263f);
  out = imath_fma(out, a, -1.49278f);
#  else
  out = imath_fma(0.056570851f, a, -0.44717955f);
  out = imath_fma(out, a, 1.4699568f);
  out = imath_fma(out, a, -2.8212026f);
  out = imath_fma(out, a, -1.7417939f);
#  endif // FASTER_MATH
  return imath_fma(M_LN2_INV, out, t);
#endif // builtin
}
/*
 *       a  =    m*2^p
 * => e^(a) = e^(m*2^p)
 * => e^(a) = 2^((m*2^p)*lg2(e))
 * => e^(a) = 2^((m*2^p)/ln(2))
 */
inline float imath_exp(float a) {
#if BLTN(__builtin_expf)
  return __builtin_expf(a);
#elif !defined(NO_STDMATH)
  return expf(a);
#else
  return imath_exp2(a*M_LN2_INV);
#endif // builtin
}
// a * 2 ^ x
inline float imath_ldexp(float a, int x) {
#if BLTN(__builtin_ldexpf)
  return __builtin_ldexpf(a, x);
#elif !defined(NO_STDMATH)
  return ldexpf(a, x);
#else
  int *ae = CAST(int*)&a;
  int c = ((*ae >> 23) & 0x3ff) - 127;
  c += x;
  if (c > 125) {
    *ae |= 0x7ff; // inf
  } else if (c < -125) {
    *ae = 0; // zero
  } else {
    *ae &= 0x807fffff;
    *ae |= (c + 127) << 23;
  }
  return a;
#endif // builtin
  
}
/*
 *       a  = n+f
 * => 2^(a) = 2^(n+f)
 * => 2^(a) = 2^n * 2^f
 */
inline float imath_exp2(float a) {
#if BLTN(__builtin_exp2f)
  return __builtin_exp2f(a);
#elif !defined(NO_STDMATH)
  return exp2f(a);
#else
  int *n = CAST(int*)&a;
  if (a > 126.0f) {*n |= 0x7f800000; return a;} // INF
  else if (a < -149.0f) return 0.0f;
  // a => N(integer) + F(fraction 0 ~ <1)
  float f = imath_frexp(a, n);
  // small polynomial approximate 2^f on [0,1)
  f *= imath_fma(f, imath_fma(f, 0.05550411f, 0.24022651f), M_LN2);
  // build float from exponent and mantissa: (n+bias) in exponent, m in mantissa
  *n += 127;
  return a * f;
#endif // builtin
}
float imath_len(const float *x, iter n) {
  float vs = 0.0f;
  for (iter i = 0; i < n; ++i)
    vs = imath_fma(x[i], x[i], vs);
  return imath_sqrt(vs);
}
inline float imath_hypot(float x, float y) {
#if BLTN(__builtin_hypotf)
  return __builtin_hypotf(x, y);
#elif !defined(NO_STDMATH)
  return hypotf(x,y);
#else
  return imath_sqrt(x*x+y*y);
#endif // builtin
}
inline float imath_sqrt(float x) {
#if BLTN(__builtin_sqrtf)
  return __builtin_sqrtf(x);
#elif !defined(NO_STDMATH)
  return sqrtf(x);
#else
  float x2 = x * 0.5f;
  int *i = CAST(int*)&x;
  *i = (*i >> 1) - 0x5f3759df;
  #ifndef FASTER_MATH
  x += x2; x *= 0.5f;
  #endif // faster math
  x += x2; x *= 0.5f;
  return x;
#endif // builtin
}
inline float imath_isqrt(float x) {
#if BLTN(__builtin_sqrtf)
  return 1.0f / __builtin_sqrtf(x);
#elif !defined(NO_STDMATH)
  return sqrtf(x);
#else
  float x2 = x * 0.5f;
  int *i = CAST(int*)&x;
  *i = 0x5f3759df - (*i >> 1);
  #ifndef FASTER_MATH
    x *= 1.5f - x2 * x * x;
  #endif
  x *= 1.5f - x2 * x * x;
  return u.f;
#endif // builtin
}
#define NAIVE_FLIP(X,L,R) (((X) << (L % (sizeof(X) * 8))) | ((X) >> (R % (sizeof(X) * 8))))
ubyte imath_flip8(ubyte x) {
  return CAST(ubyte)NAIVE_FLIP(x,4,4);
}
ushrt imath_flip16(ushrt x) {
#if BLTN(__builtin_bswap16)
  return __builtin_bswap16(x);
#elif defined(__GNUC__)
  return bswap_16(x);
#else
	return NAIVE_FLIP(x,1,1);
#endif
} 
uint32 imath_flip32(uint32 x) {
#if BLTN(__builtin_bswap32)
  return __builtin_bswap32(x);
#elif defined(__GNUC__)
  return bswap_32(x);
#else
	util_memflip(&x, 4);
	return x;
#endif
}
uint64 imath_flip64(uint64 x) {
#if BLTN(__builtin_bswap64)
  return __builtin_bswap64(x);
#elif defined(__GNUC__)
  return bswap_64(x);
#else
	util_memflip(&x, 8);
	return x;
#endif
}
int32 imath_rotl32(int32 x, const iter n) {
#if BLTN(__builtin_rotateleft32)
  return __builtin_rotateleft32(x, n);
#elif defined(_MSC_VER)
  return _rotl(x, n);
#else
  return CAST(int32)NAIVE_FLIP(x,n,(-n&31));
#endif
}
int64 imath_rotl64(int64 x, const iter n) {
#if BLTN(__builtin_rotateleft64)
  return __builtin_rotateleft64(x, n);
#elif defined(_MSC_VER)
  return _rotl64(x, n);
#else
  return CAST(int64)NAIVE_FLIP(x,n,(-n&63));
#endif
}
int32 imath_rotr32(int32 x, const iter n) {
#if BLTN(__builtin_rotateright32)
  return __builtin_rotateright32(x, n);
#elif defined(_MSC_VER)
  return _rotr(x, n);
#else
  return CAST(int32)NAIVE_FLIP(x,(-n&31),n);
#endif
}
int64 imath_rotr64(int64 x, const iter n) {
#if BLTN(__builtin_rotateright64)
  return __builtin_rotateright64(x, n);
#elif defined(_MSC_VER)
  return _rotr64(x, n);
#else
  return CAST(int64)NAIVE_FLIP(x,(-n&63),n);
#endif
}
#define RAND_VARIANT(T) inline T imath_rand_##T (void) { \
  T r = CAST(T)rand();\
  return r ^ (CAST(T)CAST(long)&r);\
}

RAND_VARIANT(byte)
RAND_VARIANT(shrt)
RAND_VARIANT(int32)
RAND_VARIANT(int64)
RAND_VARIANT(int)
RAND_VARIANT(long)
RAND_VARIANT(ubyte)
RAND_VARIANT(ushrt)
RAND_VARIANT(uint32)
RAND_VARIANT(uint64)
RAND_VARIANT(uint)
RAND_VARIANT(ulong)

#undef RAND_VARIANT
inline float imath_rand_float(void) {
  union { float f; int i; } U;
  do { U.i = imath_rand_int();
  } while (!imath_isnormal(U.f));
  return U.f;
}




