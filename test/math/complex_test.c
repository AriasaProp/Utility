#include <stdio.h>

#include "math/complex.h"
#include "util/profiling.h"
#include "util/console_out.h"
#include "common.h"

#define TEST 10000

static float rander() {
  return CAST(float)imath_rand_shrt() * 0.01f;
}

int main (int UNUSED_ARG(argc), char ** UNUSED_ARG(argv)) {
  String main_str = NULL;
  printf("Complex Test! -> ");
  pr_time ct = profiling_current_time(); \
	complex left, right, mid;
	float c;
  string_clean(main_str);
  iter i;
#define TY_LEN 256
  char types[TY_LEN] = {0};
  for (i = 0; i < TEST; ++i) {
#define CASE(A,B)    do {\
  strncpy(types,#A " " #B " real ", TY_LEN);\
  complex_set_cartesian(&left, rander(), rander());\
  c = rander();\
  right = complex_##A##f(left, c);\
  complex_m##B##f(&right, c);\
  if (!complex_equal(left,right)) break;\
  strncpy(types,#A " " #B " imaginary ", TY_LEN);\
  complex_set_cartesian(&left, rander(), rander());\
  c = rander();\
  right = complex_##A##fi(left, c);\
  complex_m##B##fi(&right, c);\
  if (!complex_equal(left,right)) break;\
  strncpy(types,#A " " #B " complex ", TY_LEN);\
  complex_set_cartesian(&left, rander(), rander());\
  complex_set_cartesian(&mid, rander(), rander());\
  right = complex_##A (left, mid);\
  complex_m##B (&right, mid);\
  if (!complex_equal(left,right)) break;\
} while (0)
#define CASER(A,B)  do {\
  CASE(A,B);\
  CASE(B,A);\
} while (0)
    CASER(add,sub);
    CASER(mul,div);
    CASER(pow,root);
  }
  string_clean(main_str);
  profiling_append_as_time2(&main_str, profiling_time_since(ct));
  printf("%s ", main_str);
  if (i < TEST) {
    string_clean(main_str);
    complex_append_string(&main_str, left);
    printf(RED"failure at %s \n"RESET"left -> %s\n", types, main_str);
    string_clean(main_str);
    complex_append_string(&main_str, right);
    printf("right -> %s\n", main_str);
  } else printf(GREEN"success\n"RESET);
  
  string_free(&main_str);
  return (i < TEST) ? EXIT_FAILURE : EXIT_SUCCESS;
}
