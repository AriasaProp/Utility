#include "common.h"
#include "array/dstring.h"
#include "util/console_out.h"

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
#define ALL_POSSIBLE 0xffff
#define TRY          0x100000
#define TRY_P        0x10000
  dstring qstr = NULL;
  UNUSED(qstr);
  float *prob = CAST(float*)calloc(sizeof(iter), ALL_POSSIBLE + 1);
  iter i, j;
  float p1 = 1.0f / CAST(float)TRY;
  for (i = 0; i < TRY;)
    for (j = 0; i < TRY && j < TRY_P; ++j, ++i)
      prob[imath_rand_ushrt()] += p1;
  float entropy = 0.0f;
  for (i = 0; i <= ALL_POSSIBLE; ++i)
    entropy -= prob[i] * imath_log2(prob[i]);
  PRINT_INF("Random Test, entropy %08.4f done!.\n", entropy);
  free(prob);
  return EXIT_SUCCESS;
}