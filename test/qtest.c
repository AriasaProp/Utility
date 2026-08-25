#include "util/console_out.h"
#include "common.h"
#include "array/dstring.h"


static ulong cstr_to_word(const char *str, ulong *ret) {
   word base = 0;
#ifdef __ARM_NEON
#  ifdef __aarch64_
  ubyte res[16], mask[16];
  // may return 16 digits decimal
  uint8x16_t digits = vld1q_u8(CAST(const ubyte *)str);
  uint8x16_t vzero = vdupq_n_u8(CAST(const ubyte)'0');
  uint8x16_t vmax = vmaxq_u8(digits, vzero);
  vmax = vceqq_u8(vmax, digits);
  uint8x16_t vmin = vdupq_n_u8(CAST(const ubyte)'9');
  vmin = vminq_u8(digits, vmin);
  vmin = vceqq_u8(vmin, digits);
  digits = vsubq_u8(digits, vzero);
  uint8x16_t vres = vandq_u8(vmax, vmin);
  digits = vandq_u8(digits, vres);
  vst1q_u8(mask, vres);
  base = 1;
  for (iter i = 0; i < 16; ++i) {
    if (mask[i] == 0xff) base *= 10;
    else break;
  }
  if (base == 1) {
    *ret = 0;
    return 0;
  }
#  else
  ubyte res[8], mask[8];
  // may return 8 digits decimal
  uint8x8_t digits = vld1_u8(CAST(const ubyte *)str);
  uint8x8_t vzero = vdup_n_u8(CAST(const ubyte)'0');
  uint8x8_t vmax = vmax_u8(digits, vzero);
  vmax = vceq_u8(vmax, digits);
  uint8x8_t vmin = vdup_n_u8(CAST(const ubyte)'9');
  vmin = vmin_u8(digits, vmin);
  vmin = vceq_u8(vmin, digits);
  uint8x8_t vres = vand_u8(vmax, vmin);
  vst1_u8(mask, vres);
  base = 1;
  for (iter i = 0; i < 8; ++i) {
    if (mask[i] == 0xff) base *= 10;
    else break;
  }
  if (base == 1) {
    *ret = 0;
    return 0;
  }
#  endif
#elif defined(__AVX2__)
  // may return 32 digits decimal
  __m256i digits = _mm256_loadu_si256((const __m256i *)str);
  __m256i vmax = _mm256_set1_epi8('0');
  vmax = _mm256_max_epu8(digits, vmax);
  vmax = _mm256_cmpeq_epi8(vmax, digits);
  __m256i vmin = _mm256_set1_epi8('9');
  vmin = _mm256_min_epu8(digits, vmin);
  vmin = _mm256_cmpeq_epi8(vmin, digits);
  __m256i valid = _mm256_and_si256(vmax, vmin);
  int mask = _mm_movemask_epi8(_mm256_extracti128_si256(valid, 0));
  mask |= _mm_movemask_epi8(_mm256_extracti128_si256(valid, 1)) << 16;
  base = util_ctz(~mask);
#elif defined(__AVX__) || defined(__SSE__)
  // may return 16 digits decimal
  __m128i digits = _mm_loadu_si128((const __m128i *)str);
  __m128i vmax = _mm_set1_epi8('0');
  vmax = _mm_max_epu8(digits, vmax);
  vmax = _mm_cmpeq_epi8(vmax, digits);
  __m128i vmin = _mm_set1_epi8('9');
  vmin = _mm_min_epu8(digits, vmin);
  vmin = _mm_cmpeq_epi8(vmin, digits);
  __m128i valid = _mm_and_si128(vmax, vmin);
  int mask = _mm_movemask_epi8(valid);
  base = util_ctz(~mask);
#else
  char tstr[WORD_DECIMAL] = {0};
  base = MIN(WORD_DECIMAL, strlen(str));
  *ret = strtoull(tstr, NULL, 10);
  if (*ret == ULLONG_MAX) return 0;
#endif
  return base;
}


int main() {
  PRINT_INF("Hello, QTest!\n");
  dstring S = NULL;
  ulong uw;
  dstring_append(&S, "%lu", imath_rand_ulong());
  PRINT_INF("string %s\n", S);
  iter r = cstr_to_word(S, &uw);
  PRINT_INF("parsed %lu\n", uw);
  PRINT_INF("based  %zu\n", r);
  dstring_free(&S);
  return 0;
}
