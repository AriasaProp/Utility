#include "common.h"
#include "util/profiling.h"
#include "util/console_out.h"

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  PRINT_INF("Random Test! ");
#define ALL_POSSIBLE 0xffff
#define TRY          0x100000
#define TRY_P        0x10000
  String qstr = NULL;
  float *prob = CAST(float*)util_calloc(sizeof(iter), ALL_POSSIBLE + 1);
  iter i, j;
  pr_time t = profiling_current_time();
  float p1 = 1.0f / CAST(float)TRY;
  for (i = 0; i < TRY;)
    for (j = 0; i < TRY && j < TRY_P; ++j, ++i)
      prob[imath_rand_ushrt()] += p1;
  float entropy = 0.0f;
  for (i = 0; i <= ALL_POSSIBLE; ++i)
    entropy -= prob[i] * imath_log2(prob[i]);
  t = profiling_time_since(t);
  profiling_append_as_time2(&qstr, t);
  printf("entropy %08.4f -> %s", entropy, qstr);
  string_free(&qstr);
  util_memfree(prob);
  printf(" done!\n");
  return EXIT_SUCCESS;
}