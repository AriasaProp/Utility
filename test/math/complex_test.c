#include <stdio.h>

#include "math/complex.h"
#include "util/profiling.h"
#include "util/console_out.h"
#include "common.h"

int main (int UNUSED_ARG(argc), char ** UNUSED_ARG(argv)) {
  String main_str = NULL;
  printf("complex test -> ");
  pr_time ct = profiling_current_time(); \
	complex r;
	iter i, j;
  string_clean(main_str);

  // do calculation
#define CASE(A) do {\
  j = STACK_ARR_LEN(tests);\
  for (i = 0; i < j; ++i) { \
    r = complex_##A(tests[i][0], tests[i][1]); \
    if (!complex_eq(r, tests[i][2])) { \
      complex_append_string(&main_str, r); \
      string_append_cstr(&main_str, "!=", 2); \
      complex_append_string(&main_str, tests[i][2]); \
      printf(RED "%s fail[%zu] %s\n"RESET, #A, i, main_str); \
      goto complex_test_end; \
    } \
  } \
} while(0)
  {
    complex tests[][3] = {
     {{2,3},{3,4},{5,7}},
     {{0,1},{0,1},{0,2}},
     {{-3,7},{-2,-2},{-5,5}},
    };
    CASE(add);
  }
  {
    complex tests[][3] = {
     {{-6,8},{7,-1},{-13,9}},
    };
    CASE(sub);
  }
  {
    complex tests[][3] = {
     {{-1,2},{4,4},{-12,4}},
    };
    CASE(mul);
  }
  {
    complex tests[][3] = {
     {{7,14},{1,2},{7,0}},
    };
    CASE(div);
  }
  {
    complex tests[][3] = {
     {{2,3},{5,0},{122,-597}},
    };
    CASE(pow);
  }
#undef CASE
  profiling_append_as_time2(&main_str, profiling_time_since(ct));
  printf("%s\n", main_str);
complex_test_end:
  string_free(&main_str);
}