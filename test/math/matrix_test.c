#include <stdio.h>

#include "math/matrix.h"
#include "util/profiling.h"
#include "util/console_out.h"
#include "common.h"

#define TEST 10000

static float rander() {
  return CAST(float)imath_rand_shrt() * 0.01f;
}
static ubyte matrix_rdim() {
  return imath_rand_ubyte() % 5 + 2;
}

int main (int UNUSED_ARG(argc), char ** UNUSED_ARG(argv)) {
  String main_str = NULL;
  printf("Matrix Test! -> ");
  pr_time ct = profiling_current_time(); \
  string_clean(main_str);
  iter i,j;
  float c;
  matrix m[3] = {0};
#define TY 256
  char types[TY] = {0};
  for (i = 0; i < TEST; ++i) {
#define CASE(A,B) do {\
  strncpy(types,#A " " #B " matrix ",TY);\
  m[0].cols = m[1].cols = matrix_rdim();\
  m[0].rows = m[1].rows = matrix_rdim();\
  m[0].data = CAST(float*)util_realloc(m[0].data, matrix_bytes(m[0])); \
  m[1].data = CAST(float*)util_realloc(m[1].data, matrix_bytes(m[1])); \
  for (j = 0; j < matrix_size(m[0]); ++j) {\
    m[0].data[j] = rander();\
    m[1].data[j] = rander();\
  }\
  matrix_free(m + 2); \
  m[0] = matrix_##A (m[0], m[1]);\
  matrix_m##B (m + 2, m[1]);\
  if (!matrix_equal(m[0],m[2])) break;\
} while (0)
#define CASE2(A,B) do {\
  strncpy(types,#A " " #B " floating ",TY);\
  c = rander();\
  m[0].cols = matrix_rdim();\
  m[0].rows = matrix_rdim();\
  m[0].data = CAST(float*)util_realloc(m[0].data, matrix_bytes(m[0])); \
  for (j = 0; j < matrix_size(m[0]); ++j) \
    m[0].data[j] = rander();\
  matrix_free(m + 2); \
  m[2] = matrix_##A##f(m[0], c);\
  matrix_m##B##f(m + 2, c);\
  if (!matrix_equal(m[0],m[2])) break;\
  strncpy(types,#A " " #B " matrix ", TY);\
  m[0].cols = m[1].cols = m[0].rows = m[1].rows = matrix_rdim();\
  m[0].data = CAST(float*)util_realloc(m[0].data, matrix_bytes(m[0])); \
  m[1].data = CAST(float*)util_realloc(m[1].data, matrix_bytes(m[1])); \
  for (j = 0; j < matrix_size(m[0]); ++j) {\
    m[0].data[j] = rander();\
    m[1].data[j] = rander();\
  }\
  matrix_free(m + 2); \
  m[2] = matrix_##A (m[0], m[1]);\
  matrix_m##B (m + 2, m[1]);\
  if (!matrix_equal(m[0],m[2])) break;\
} while (0)

    CASE(add,sub);
    CASE(sub,add);
    CASE2(mul,div);
    CASE2(div,mul);
  }
  for (j = 0; j < 3; ++j)
    matrix_free(m + j);
  string_clean(main_str);
  profiling_append_as_time2(&main_str, profiling_time_since(ct));
  printf("%s ", main_str);
  if (i < TEST) {
    string_clean(main_str);
    matrix_append_string(&main_str, m[0]);
    printf(RED"failure at %s \n"RESET" [0] -> %s\n", types, main_str);
    string_clean(main_str);
    matrix_append_string(&main_str, m[2]);
    printf(" [1] -> %s\n", main_str);
  } else printf(GREEN"success\n"RESET);
  string_free(&main_str);
  return (i < TEST) ? EXIT_FAILURE : EXIT_SUCCESS;
}