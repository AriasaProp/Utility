#include <stdio.h>

#include "math/matrix.h"
#include "util/profiling.h"
#include "common.h"

typedef enum {
  MATRIX_ACT_ADD,
  MATRIX_ACT_SUB,
  MATRIX_ACT_MUL,
  MATRIX_ACT_DIV,
} act_type;

const struct {
  act_type t;
  const matrix a, b, c;
} matrix_tests[] = {
  {.t = MATRIX_ACT_ADD,
   .a = (matrix){2,2,(float[4]){1.0f, 2.0f, 3.0f, 4.0f}},
   .b = (matrix){2,2,(float[4]){5.0f, 6.0f, 7.0f, 8.0f}},
   .c = (matrix){2,2,(float[4]){6.0f, 8.0f,10.0f,12.0f}} },
  {.t = MATRIX_ACT_SUB,
   .a = (matrix){2,2,(float[4]){9.0f, 8.0f, 7.0f, 6.0f}},
   .b = (matrix){2,2,(float[4]){1.0f, 2.0f, 3.0f, 4.0f}},
   .c = (matrix){2,2,(float[4]){8.0f, 6.0f, 4.0f, 2.0f}} },
  {.t = MATRIX_ACT_MUL,
   .a = (matrix){2,2,(float[4]){1.0f,2.0f,3.0f,4.0f}}, 
   .b = (matrix){2,2,(float[4]){1.0f,0.0f,0.0f,1.0f}}, 
   .c = (matrix){2,2,(float[4]){1.0f,2.0f,3.0f,4.0f}} },
  {.t = MATRIX_ACT_MUL,
   .a = (matrix){2,1,(float[2]){ 1.0f,2.0f}}, 
   .b = (matrix){1,2,(float[2]){ 3.0f,4.0f}},
   .c = (matrix){1,1,(float[1]){11.0f}} },
  {.t = MATRIX_ACT_DIV,
   .a = (matrix){2, 2, (float[4]){ 6.0f, 8.0f, 10.0f, 12.0f}},
   .b = (matrix){2, 2, (float[4]){ 2.0f, 2.0f,  5.0f,  6.0f}},
   .c = (matrix){2, 2, (float[4]){-2.0f, 2.0f,  0.0f,  2.0f}} },
  {.t = MATRIX_ACT_DIV,
   .a = (matrix){2, 2, (float[4]){28.0f, 27.0f, 56.0f, 48.0f}},
   .b = (matrix){2, 2, (float[4]){ 7.0f,  6.0f,  0.0f,  1.0f}},
   .c = (matrix){2, 2, (float[4]){ 4.0f,  3.0f,  8.0f,  0.0f}} },
};
TODO("Need more matrix tests.");

int main (int UNUSED_ARG(argc), char **UNUSED_ARG(argv)) {
  String main_str = NULL;
  printf( "matrix -> ");
  pr_time ct = profiling_current_time();
  matrix res = {0};
  int eq;
#define CASE(A,B,S) case MATRIX_ACT_##A: { \
  res = matrix_##B(matrix_tests[i].a,matrix_tests[i].b); \
  eq = matrix_equal(res, matrix_tests[i].c); \
  if(!eq) { \
    string_clean(main_str); \
    matrix_append_string(&main_str, res);\
    printf("fail do matrix " S " at index %zu with result %s\n", i, main_str); \
  }\
  matrix_free(&res); \
  if (!eq) goto matrix_test_end; \
} break
  for (iter i = 0; i < STACK_ARR_LEN(matrix_tests); ++i) {
    switch (matrix_tests[i].t) {
      default: break;
      CASE(ADD, add, "addition");
      CASE(SUB, sub, "subtraction");
      CASE(MUL, mul, "multiplication");
      CASE(DIV, div, "division");
    }
  }
#undef CASE
matrix_test_end:
  string_clean(main_str);
  profiling_append_as_time(&main_str, profiling_time_since(ct));
  printf( "done! %s\n", main_str);
  string_free(&main_str);
}
