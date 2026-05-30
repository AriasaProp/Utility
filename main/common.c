/* *****************************************************************************
 * common.c v0.0.0000
 * 
 * Provide basic function and constant for all source code
 * 
 * 
 * 
 * *****************************************************************************/

#include "common.h"

// should exponent of 2
#define STRING_CAP_ROUND 16

// #define NO_STDMATH // not with -lm
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
// #define NO_STDC // not with -lc
#if !defined(NO_STDC)
  #include <byteswap.h>
  #include <stdarg.h>
  #include <stdio.h>
  #include <stddef.h>
  #include <string.h>
  #include <time.h>
#endif // NO_STDC
// #define FASTER_MATH // no use when STD_MATH defined
#if defined(_WIN32) || defined(_WIN64)
  #include <intrin.h>
  #pragma intrinsic(__rdtsc)
// win32/64 wide character support
int convert_wchar_to_utf8(char *buffer, iter bufferlen, const wchar_t *input) {
  return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, NULL, NULL);
}
#elif defined(__x86_64) && (defined(__GNUC__) || defined(__clang__))
  // CPUID stuff is only for X86 + GCC/Clang, MSVC has it in intrin.h
  #include <cpuid.h>
#endif

#define CHAR_WHITESPACE ' '


/* ================================
 *  Standar Utility Function
 * ================================
 */
void *util_malloc(iter bytes) {
#ifdef NO_STDC
  TODO("Implements malloc on non standar usage!");
#else
  return malloc(bytes);
#endif // NO_STDC
}
void *util_calloc(iter n, iter bytes) {
#ifdef NO_STDC
  TODO("Implements calloc on non standar usage!");
#else
  return calloc(n, bytes);
#endif // NO_STDC
}
void *util_realloc(void *a, iter bytes) {
#ifdef NO_STDC
  TODO("Implements realloc on non standar usage!");
#else
  return realloc(a, bytes);
#endif // NO_STDC
}
void util_memfree(void *p) {
#ifdef NO_STDC
  TODO("Implements free on non standar usage!");
#else
  free(p);
#endif // NO_STDC
}
void util_memswap(void *a, void *b, iter bytes) {
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
#ifdef NO_STDC
  byte *DST = CAST(byte*)dst;
  const byte *SRC = CAST(byte*)src;
  for (iter i = 0; i < bytes; ++i)
    DST[i] = SRC[i];
#else 
  memcpy(dst,src,bytes);
#endif // NO_STDC
}
void util_memrcpy(void *dst, const void *src,iter bytes) {
  byte *DST = CAST(byte*)dst;
  const byte *SRC = CAST(byte*)src;
  while (bytes--) DST[bytes] = SRC[bytes];
}
int util_memcmp (const void *a, const void *b,iter bytes) {
#ifdef NO_STDC
  int ret = 0;
  byte *A = CAST(byte*)a, *N = CAST(byte*)b;
  for (iter i = 0; !ret && (i < bytes); ++i)
    ret = (A[i] > B[i]) - (A[i] < B[i]);
  return ret;
#else
  return memcmp(a,b,bytes);
#endif // NO_STDC
}
void util_memmove(void *dst, void *src,iter bytes) {
#ifdef NO_STDC
  byte *DST = CAST(byte*)dst, *SRC = CAST(byte*)src;
  byte *BUF = CAST(byte*)util_malloc(bytes);
  for (iter i = 0; i < bytes; ++i)
    BUF[i] = SRC[i];
  for (iter i = 0; i < bytes; ++i)
    DST[i] = BUF[i];
  util_memfree(BUF);
#else
  memmove(dst,src,bytes);
#endif // NO_STDC
}
void util_memset(void *dst, int src, iter bytes) {
#ifdef NO_STDC
  byte *DST = CAST(byte*)dst, SRC = CAST(byte)src;
  for (iter i = 0; i < bytes; ++i)
    DST[i] = SRC;
#else 
  memset(dst,src,bytes);
#endif // NO_STDC
}
iter util_strlen(const char *str) {
#ifdef NO_STDC
  iter i = 0;
  while (str && str[i]) ++i;
  return i;
#else
  return strlen(str);
#endif // NO_STDC
}
iter util_bitlead(ulong x) {
#if __has_builtin(__builtin_clzl)
  return sizeof(ulong) * 8 - __builtin_clzl(x);
#else
  iter r = 0;
  while (x) ++r, x >>= 1;
  return r;
#endif
}

typedef struct {
  iter cap, count;
} string_head;

static inline string_head *string__get_head(String str) {
  if (str) return (CAST(string_head*)str) - 1;
  return NULL;
}
static inline String string__get_string(string_head *sh) {
  return CAST(String) (!sh ? NULL : (sh + 1));
}
static inline string_head *string__reserve(string_head *sh, iter more) {
  if (sh) more += sh->count;
  iter need = (more & ~(STRING_CAP_ROUND - 1)) + STRING_CAP_ROUND * !!(more & (STRING_CAP_ROUND - 1));
  if (!sh || (sh->cap < need)) {
    sh = CAST(string_head*) util_realloc(sh, need + sizeof(string_head));
    sh->cap = need;
  }
  return sh;
}

/* ================================
 *  String Function
 * ================================
 */
inline void string_append_char(String *str,char c) {
  string_head *sh = string__reserve(string__get_head(*str), 2);
  *str = string__get_string(sh);
  *(*str + (sh->count++)) = c;
  str[sh->count] = 0;
}
inline void string_append_cstr(String *str, const char *cstr, iter len) {
  string_head *sh = string__reserve(string__get_head(*str), len + 1);
  *str = string__get_string(sh);
  util_memcpy(*str + sh->count, cstr, len + 1);
  sh->count += len;
  // is always memcpy end with null?
  // str[sh->count += len] = 0;
}
inline void string_append(String *str, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int r = vsnprintf(NULL, 0, fmt, args);
  ASSERT(r >= 0);
  string_head *sh = string__reserve(string__get_head(*str), r + 1);
  *str = string__get_string(sh);
  r = vsnprintf((*str) + sh->count, r + 1, fmt, args);
  va_end(args);
  sh->count += r;
  // is always vsnprintf end with null?
  // str[sh->count += r] = 0;
}
inline void string_reserve(String *str, iter sz) {
  *str = string__get_string(string__reserve(string__get_head(*str), sz + 1));
}
inline iter string_len(const String str) {
  const string_head *sh = string__get_head(str);
  return sh ? sh->count : 0;
}
inline void string_clean(String str) {
  string_head *sh = string__get_head(str);
  if (!sh) return;
  sh->count = 0;
  *(CAST(char*)str) = 0;
}
inline void string_free (String *str) {
  free(string__get_head(*str));
  *str = NULL;
}


/* ================================
 *  String View Functions
 * ================================
 */
#define STRINGVIEW_NONNULL(X) if (!X) return (StringView){0}
// generate StringView from String
StringView stringview_str(const String s) {
  STRINGVIEW_NONNULL(s);
  return (StringView){.count = string_len(s), .cstr = CAST(const char*)s};
}
// chop by whitespace
StringView stringview_chop(const String s) {
  return stringview_chop_char(s, CHAR_WHITESPACE);
}
StringView stringview_chop_char(const String s,const char d) {
  STRINGVIEW_NONNULL(s);
  StringView ret = {.count = 0, .cstr = CAST(const char*)s};
  for (iter i = string_len(s); (ret.count < i) && s[ret.count] && (s[ret.count] != d); ++(ret.count)) ;
  return ret;
}
StringView stringview_chop_chars(const String s,const char *d) {
  STRINGVIEW_NONNULL(s);
  STRINGVIEW_NONNULL(d);
  StringView ret = {.count = 0, .cstr = CAST(const char*)s};
  for (iter i = string_len(s),j; (ret.count < i) && s[ret.count]; ) {
    for (j = 0; d[j]; ++j) if (d[j] == s[ret.count]) return ret;
    ++(ret.count);
  }
  return ret;
}
StringView stringview_chop_cstr(const String s,const char *cstr) {
  STRINGVIEW_NONNULL(s);
  STRINGVIEW_NONNULL(cstr);
  StringView ret = {.count = 0, .cstr = CAST(const char*)s};
  for (iter i = string_len(s), j = util_strlen(cstr); ((i - ret.count) > j) && (ret.count < i) && s[ret.count] && !util_memcmp(s + ret.count, cstr, j); ++(ret.count)) ;
  return ret;
}
// StringView helper
int stringview_equal (const StringView a,const StringView b) {
  int ret = (a.count>b.count) - (a.count<b.count);
  if (!ret) ret = util_memcmp(a.cstr, b.cstr, a.count);
  return ret;
}

/* ================================
 *  IMath Function
 * ================================
 */


#if defined(NO_STDMATH) || !(__has_builtin(__builtin_sinf) || __has_builtin(__builtin_cosf) || __has_builtin(__builtin_tanf))
// src https://gist.githubusercontent.com/Sam-Belliveau/9c2e5a7584ec9900831877b3155f6f16/raw/238a72e91f46ad616fbaedddec30f163dbb8cb85/fast_trig.h
static inline float imath__cosine_wave(float x) {
  x *= M_PI2;
  x -= M_PI_INV * x * imath_fabs(x);
#ifdef FASTER_MATH
  x += 0.3451140202480f * x * imath_fabs(x);
#else
  x += 0.25f * x * imath_fabs(x);
  x += 0.0684571845286f * x * imath_fabs(x);
#endif //FASTER_MATH
  return x;
}
#endif // builtin trig

inline uint imath_iabs (int a) {
#if __has_builtin(__builtin_abs)
  return __builtin_abs(a);
#elif !defined(NO_STDMATH)
  return abs(a);
#else
  return a & 0x7fffffff; // assume big endian
#endif // builtin
}

inline float imath_fabs  (float a) {
#if __has_builtin(__builtin_fabs)
  return __builtin_fabs(a);
#elif !defined(NO_STDMATH)
  return fabs(a);
#else
	(*CAST(int*)&a) &= 0x7fffffff;
	return a;
#endif // builtin
}
inline float imath_fmax(float a, float b) {
#if __has_builtin(__builtin_fmax)
  return __builtin_fmax(a,b);
#elif !defined(NO_STDMATH)
  return fmax(a,b);
#else
  return MAX(a,b);
#endif // builtin
}
inline float imath_fmin(float a, float b) {
#if __has_builtin(__builtin_fmin)
  return __builtin_fmin(a,b);
#elif !defined(NO_STDMATH)
  return fmin(a,b);
#else
  return MIN(a,b);
#endif // builtin
}
inline float imath_round(float a) {
#if __has_builtin(__builtin_roundf)
  return __builtin_roundf(a);
#elif !defined(NO_STDMATH)
  return roundf(a);
#else
  return CAST(float)CAST(int)(a + 0.5f - (a < 0));
#endif // builtin
}
inline float imath_floor(float a) {
#if __has_builtin(__builtin_floorf)
  return __builtin_floorf(a);
#elif !defined(NO_STDMATH)
  return floorf(a);
#else
  return CAST(float)CAST(int)(a - (a < 0));
#endif // builtin
}
inline float imath_frexp(float a,int *i) {
#if __has_builtin(__builtin_frexpf)
  return __builtin_frexpf(a,i);
#elif !defined(NO_STDMATH)
  return frexpf(a,i);
#else
  *i = CAST(int)(a - (a < 0));
  return a - CAST(float)*i;
#endif // builtin
}
inline float imath_ceil(float a) {
#if __has_builtin(__builtin_ceilf)
  return __builtin_ceilf(a);
#elif !defined(NO_STDMATH)
  return ceilf(a);
#else
  return CAST(float)CAST(int)(a + (a >= 0));
#endif // builtin
}
inline float imath_sin(float x) {
#if __has_builtin(__builtin_sinf)
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
#if __has_builtin(__builtin_cosf)
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
#if __has_builtin(__builtin_tanf)
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
#if __has_builtin(__builtin_asinf)
  return __builtin_asinf(x);
#elif !defined(NO_STDMATH)
  return asinf(x);
#else
  if (x > 1.0f || x < -1.0f) {
    int *i = CAST(int*)&x;
    *i = 0x7f800000; // NAN
  } else {
    x *= 0.1666666666667f + 0.3333333333333f * imath_fabs(x);
    x *= M_PI;
  }
  return x;
#endif // builtin
}
inline float imath_acos(float x) {
#if __has_builtin(__builtin_acosf)
  return __builtin_acosf(x);
#elif !defined(NO_STDMATH)
  return acosf(x);
#else
  if (x > 1.0f || x < -1.0f) {
    int *i = CAST(int*)&x;
    *i = 0x7f800000; // NAN 
  } else {
    x *= 0.1666666666667f + 0.3333333333333f * imath_fabs(x);
    x *= -M_PI;
    x -= M_PI_HALF;
  }
  return x;
#endif // builtin
}
inline float imath_atan(float x) {
#if __has_builtin(__builtin_atanf)
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
#if __has_builtin(__builtin_atan2f)
  return __builtin_atan2f(x,y);
#elif !defined(NO_STDMATH)
  return atan2f(x,y);
#else
  return imath_atan(x/y);
#endif // builtin
}
inline float imath_pow(float a,float b) {
#if __has_builtin(__builtin_powf)
  return __builtin_powf(a,b);
#elif !defined(NO_STDMATH)
  return powf(a,b);
#else
  b *= imath_log(a);
  return imath_exp(b);
#endif // builtin
}
inline float imath_log(float a) {
#if __has_builtin(__builtin_logf)
  return __builtin_logf(a);
#elif !defined(NO_STDMATH)
  return logf(a);
#else
  // stolen from https://gist.github.com/LingDong-/7e4c4cae5cbbc44400a05fba65f06f23
  // FORMULA: 
  //       x  =    m  *   2^p
  // => ln(x) = ln(m) + ln(2)p
  // exp = 127 for a = 1, 
  // so 2^(exp-127) is the multiplier
  // evil floating point bit level hacking
  uint *bx = CAST(uint*)&a;
  // extract exp, since a>0, sign bit must be 0
  float t = CAST(float)((*bx >> 23) - 127);
  // get mantissa
  *bx &= 8388607;
  // set exp
  *bx |= 1065353216;
  return M_LN2 * t -
  #ifdef FASTER_MATH
    1.49278f+(2.11263f+(-0.729104f+0.10969f*a)*a)*a
  #else
    1.7417939f+(2.8212026f+(-1.4699568f+(0.44717955f-0.056570851f*a)*a)*a)*a
  #endif // FASTER_MATH
    ;
}
#endif // builtin
}
inline float imath_log2(float a) {
#if __has_builtin(__builtin_log2f)
  return __builtin_log2f(a);
#elif !defined(NO_STDMATH)
  return log2f(a);
#else
  // same logic as imath_log
  // FORMULA: 
  //        a  =    m  *2^p
  // => lg2(a) =lg2(m) + p
  // => lg2(a) = ln(m)/ln(2) + p
  uint *bx = CAST(uint*)&a;
  // extract exp, since a>0, sign bit must be 0
  float t = CAST(float)((*bx >> 23) - 127);
  // get mantissa
  *bx &= 8388607;
  // set exp
  *bx |= 1065353216;
  return t - (
  #ifdef FASTER_MATH
    1.49278f   + (  2.11263f - (0.729104f  - 0.10969f * a) * a) * a
  #else
    1.7417939f + (2.8212026f - (1.4699568f - (0.44717955f - 0.056570851f * a) * a) * a) * a
  #endif // FASTER_MATH
    ) * M_LN2_INV;
#endif // builtin
}
inline float imath_exp(float a) {
#if __has_builtin(__builtin_expf)
  return __builtin_expf(a);
#elif !defined(NO_STDMATH)
  return expf(a);
#else
  // FORMULA: 
  //       a  =    m*2^p
  // => e^(a) = e^(m*2^p)
  // => e^(a) = 2^((m*2^p)*lg2(e))
  // => e^(a) = 2^((m*2^p)/ln(2))
  return imath_exp2(a*M_LN2_INV);
#endif // builtin
}
inline float imath_exp2(float a) {
#if __has_builtin(__builtin_exp2f)
  return __builtin_exp2f(a);
#elif !defined(NO_STDMATH)
  return exp2f(a);
#else
  // FORMULA: 
  //       a  = n+f
  // => 2^(a) = 2^(n+f)
  // => 2^(a) = 2^n * 2^f
  int *n = CAST(int*)&a;
  if (a > 126.0f) {*n |= 0x7f800000; return a;} // INF
  else if (a < -149.0f) return 0.0f;
  // a => N(integer) + F(fraction 0 ~ <1)
  float f = imath_frexp(a, n);
  // small polynomial approximate 2^f on [0,1)
  f *= /* 1.0f + */ M_LN2 + f * (0.24022651f + f * 0.05550411f);
  // build float from exponent and mantissa: (n+bias) in exponent, m in mantissa
  *n += 127;
  return a * f;
#endif // builtin
}
float imath_len(const float *x, iter n) {
  float vs = 0.0f;
  for (iter i = 0; i < n; ++i)
    vs += x[i] * x[i];
  return imath_sqrt(vs);
}
inline float imath_hypot(float x, float y) {
#if __has_builtin(__builtin_hypotf)
  return __builtin_hypotf(x, y);
#elif !defined(NO_STDMATH)
  return hypotf(x,y);
#else
  return imath_sqrt(x*x+y*y);
#endif // builtin
}
inline float imath_sqrt(float x) {
#if __has_builtin(__builtin_sqrtf)
  return __builtin_sqrtf(x);
#elif !defined(NO_STDMATH)
  return sqrtf(x);
#else
  float x2 = x * 0.5f;
  int *i = CAST(int*)&x;
  *i = (*i >> 1) - 0x5f3759df;
  #ifndef FASTER_MATH
    x += x2;
    x *= 0.5f;
  #endif // faster math
  x += x2;
  x *= 0.5f;
  return x;
#endif // builtin
}
inline float imath_isqrt(float x) {
#if __has_builtin(__builtin_sqrtf)
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
#define NAIVE_FLIP(X,L,R) (((X) << (L)) | ((X) >> (R)))
ubyte imath_flip8(ubyte x) {
  return CAST(ubyte)NAIVE_FLIP(x,4,4);
}
ushrt imath_flip16(ushrt x) {
#if __has_builtin(__builtin_bswap16)
  return __builtin_bswap16(x);
#elif !defined(NO_STDC)
  return bswap_16(x);
#else
  return CAST(ushrt)NAIVE_FLIP(x,8,8);
#endif
} 
uint32 imath_flip32(uint32 x) {
#if __has_builtin(__builtin_bswap32)
  return __builtin_bswap32(x);
#elif !defined(NO_STDC)
  return bswap_32(x);
#else
  return CAST(uint32)NAIVE_FLIP(x,16,16);
#endif
}
uint64 imath_flip64(uint64 x) {
#if __has_builtin(__builtin_bswap64)
  return __builtin_bswap64(x);
#elif !defined(NO_STDC)
  return bswap_64(x);
#else
  return CAST(uint64)NAIVE_FLIP(x,32,32);
#endif
}
int32 imath_rotl32(int32 x, const iter n) {
#if __has_builtin(__builtin_rotateleft32)
  return __builtin_rotateleft32(x, n);
#elif defined(_MSC_VER)
  return _rotl(x, n);
#else
  return CAST(int32)NAIVE_FLIP(x,n,(-n&31));
#endif
}
int64 imath_rotl64(int64 x, const iter n) {
#if __has_builtin(__builtin_rotateleft64)
  return __builtin_rotateleft64(x, n);
#elif defined(_MSC_VER)
  return _rotl64(x, n);
#else
  return CAST(int64)NAIVE_FLIP(x,n,(-n&63));
#endif
}
int32 imath_rotr32(int32 x, const iter n) {
#if __has_builtin(__builtin_rotateright32)
  return __builtin_rotateright32(x, n);
#elif defined(_MSC_VER)
  return _rotr(x, n);
#else
  return CAST(int32)NAIVE_FLIP(x,(-n&31),n);
#endif
}
int64 imath_rotr64(int64 x, const iter n) {
#if __has_builtin(__builtin_rotateright64)
  return __builtin_rotateright64(x, n);
#elif defined(_MSC_VER)
  return _rotr64(x, n);
#else
  return CAST(int64)NAIVE_FLIP(x,(-n&63),n);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// random generate by time counter uint64
// rdtsc_rand.h: v0.7
// https://github.com/scottchiefbaker/rdtsc_rand
////////////////////////////////////////////////////////////////////////////////

inline uint64 imath_rand_uint64() {
	static int hwrng = -1;
	uint64 x;
	if (hwrng == -1) {
    // Hardware rand supported by x86_64 and ARM 8.5+
#ifdef __x86_64
  	uint eax = 0, ebx = 0, ecx = 0, edx = 0;
  	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
  	hwrng = (ecx & bit_RDRND) != 0;
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 8)
  	__asm__ volatile("mrs %0, ID_AA64ISAR0_EL1" : "=r"(x));
  	hwrng = ((x >> 60) & 0xF) != 0;  // Check RNDR bit field
#else
    hwrng = 0;
#endif
	}
	if (hwrng) {
#ifdef __x86_64
  	unsigned char ok;
  	__asm__ volatile("rdrand %0; setc %1" : "=r" (x), "=qm" (ok) : : "cc");
  	if (!ok) x = CAST(uint64)clock();
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 8)
  	__asm__ volatile("mrs %0, s3_3_c2_c4_0" : "=r"(x));
#else
    ASSERT("UNREACHABLE");
#endif
	} else {
// Get the instruction counter for various CPU/Platforms
#ifdef ARDUINO
	  x = micros();
#elif defined(_WIN32) || defined(_WIN64)
	  x = __rdtsc();
#elif defined(__aarch64__) || defined(__arm64)
	  __asm__ volatile ("mrs %0, cntvct_el0" : "=r" (x));
#elif (defined(__x86_64) || defined(__i386__) || defined(__i686__)) && (defined(__GNUC__) || defined(__clang__))
    uint32 *v = CAST(uint32*)&x;
	  __asm__ volatile ("rdtsc" : "=a"v[0], "=d"v[1]);
#elif defined(NO_STDC)
    static float private_seed = 0xa2cf27bb2397108fULL;
    x = 0xbf58476d1ce4e5b9ULL ^ private_seed;
    private_seed ^= x;
#else
    x = CAST(uint64)clock();
#endif
  }
	return x * 0xbf58476d1ce4e5b9ULL; // scramble rng (may small)
}
#define RAND_VARIANT(T) \
inline T imath_rand_##T () {\
  uint64 t = imath_rand_uint64();\
  return *((T*)&t);\
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
RAND_VARIANT(uint)
RAND_VARIANT(ulong)
#undef RAND_VARIANT
inline float imath_rand_float() {
  return imath_rand_int64() * ((3.8518597e-21f * imath_rand_uint64()) * imath_rand_uint64());
}




