/* *****************************************************************************
 * 
 * matrix4.c v0.0.0000
 * 
 * Marrix 4x4 object
 *
 *  Matrix Initialize
 *      cols ->
 *  rows 00  01   02   03
 *   |   10  11   12   13
 *   v   20  21   22   23
 *       30  31   32   33
 *
 * 
 * 
 *       
 * *****************************************************************************/



#include "math/matrix4.h"
#include "util/console_out.h"
#include "common.h"

#define EPSILON            1.0e-24f
#define FZ                 (sizeof(float))
#define float__0(x)        (imath_fabs(x) < EPSILON)
#define float__eq(x,y)     float__0((x) - (y))

#define mat4__idx(M, x, y)  (M)[(ASSERT(((y) < MAT4_SIDE) && ((x) < MAT4_SIDE),(y)*MAT4_SIDE+(x)))]
#define mat4__foreach(i)      for (iter i = 0; i < MAT4_SIZE; ++i)
#define mat4__foreachSide(i)  for (iter i = 0; i < MAT4_SIDE; ++i)
#define mat4__foreach2D(i,j)  for (iter i = 0, j; i < MAT4_SIDE; ++i) for(j = 0; j < MAT4_SIDE; ++j)

// initialize
matrix4 matrix4_idt() {
  matrix4 r = CAST(matrix4)util_calloc(MAT4_SIZE, FZ);
  mat4__foreachSide(i) mat4__idx(r,i,i) = 1.0;
  return r;
}
matrix4 matrix4_dup(const matrix4 a) {
  matrix4 r = CAST(matrix4)util_malloc(MAT4_BYTES);
  util_memcpy(r, a, MAT4_BYTES);
  return r;
}
void matrix4_move(matrix4 a, matrix4 b) {
  util_memcpy(a, b, MAT4_BYTES);
  matrix_free(b);
}
void matrix4_midt(matrix4 r) {
  mat4__foreach2D(i,j) mat4__idx(r,i,j) = CAST(float)(i == j);
}
void matrix4_set(matrix4 a, const matrix4 b) {
  util_memcpy(a, b, MAT4_BYTES);
}
// free
void matrix4_free(matrix4 r) {
  util_memfree(r);
}
// operaror compare
int matrix4_equal(const matrix4 a, const matrix4 b) {
  mat4__foreach(i) if (!float__eq(a[i],b[i])) return 1;
  return 0;
}
// property
float matrix4_det(const matrix4 r) {
  float res = 0.0f;
  float pm,mm;
  for (iter i = 1; i < MAT4_SIDE; ++i) {
    mm = pm = 1.0f;
    mat4__foreachSide(j) {
      pm *= mat4__idx(r, (i - 1 + j) % r.cols, j);
      mm *= mat4__idx(r, (r.cols - i + j) % r.cols, j);
    }
    res += pm;
    res -= mm;
  }
  return res;
}
matrix4 matrix4_trn(const matrix4 r) {
  matrix4 d = CAST(matrix4)util_malloc(MAT4_BYTES);
  mat4__foreach2D(i,j) mat4__idx(d, j, i) = mat4__idx(r, i, j);
  return d;
}
matrix4 matrix4_mulf(const matrix4 a, float s) {
  matrix4 r = matrix4_dup(a);
  matrix4_mmulf(r, s);
  return r;
}
matrix4 matrix4_divf(const matrix4 a, float s) {
  matrix4 r = matrix4_dup(a);
  matrix4_mdivf(r, s);
  return r;
}
matrix4 matrix4_add(const matrix4 a, const matrix4 b) {
  matrix4 r = matrix4_dup(a);
  matrix4_madd(r, b);
  return r;
}
matrix4 matrix4_sub(const matrix4 a, const matrix4 b) {
  matrix4 r = matrix4_dup(a);
  matrix4_madd(r, b);
  return r;
}
matrix4 matrix4_mul(const matrix4 a, const matrix4 b) {
  matrix4 r = CAST(matrix4) util_calloc(FZ, MAT4_SIZE);
  mat4__foreach2D(i,j)
    mat4__foreachSide(k)
      mat4__idx(r, k, i) += mat4__idx(a, j, i) * mat4__idx(b, k, j);
  return r;
}
void matrix4_mtrn(matrix4 r) {
  mat4__foreach2D(i,j) if (j ^ i) {
    *CAST(int*)&mat4__idx(r, j, i) ^= *CAST(int*)&mat4__idx(r, i, j);
    *CAST(int*)&mat4__idx(r, i, j) ^= *CAST(int*)&mat4__idx(r, j, i);
    *CAST(int*)&mat4__idx(r, j, i) ^= *CAST(int*)&mat4__idx(r, i, j);
  }
}
// operator modify, 0 success 1 fail
void matrix4_mmulf(matrix4 r, float s) {
  mat4__foreach(i) r[i] *= s;
}
void matrix4_mdivf(matrix4 r, float s) {
  mat4__foreach(i) r[i] /= s;
}
void matrix4_madd(matrix4 a,const matrix4 b) {
  mat4__foreach(i) a[i] += b[i];
}
void matrix4_msub(matrix4 a,const matrix4 b) {
  mat4__foreach(i) a[i] -= b[i];
}
void matrix4_mmul(matrix4 a,const matrix4 b) {
  matrix4 d = matrix4_mul(a, b);
  matrix4_move(a, d);
}
void matrix4_append_string(String *str,const matrix4 a) {
  string_append(str, "[4x4]{");
  mat4__foreachSide(i) {
    string_append_char(str, '{');
    mat4__foreachSide(j) {
      if(j) string_append_char(str, ',');
      string_append(str, "%02.2e", mat4__idx(a, j, i));
    }
    string_append_char(str, '}');
  }
  string_append_char(str, '}');
}
