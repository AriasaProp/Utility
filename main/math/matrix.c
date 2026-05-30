/* *****************************************************************************
 * 
 * matrix.c v0.0.0000
 * 
 * Marrix(x,y) object
 *
 *  Matrix Initialize
 *      cols ->
 *  rows 00  01   02   03  ....
 *   |   10  11   12   13  .....
 *   v   20  21   22   23  .....
 *       .0  .1   .2   .3  ....
 *
 * index follows by [rows, cols]
 *
 * 
 * 
 *   1=>x1*a1+x2*b1+.+xn*$1 0=>x1*a2+x2*b2+.+xn*$2  
 *   0=>y1*a1+y2*b1+.+yn*$1 1=>y1*a2+y2*b2+.+yn*$2
 *   0=>z1*a1+z2*b1+.+zn*$1 0=>z1*a2+z2*b2+.+zn*$2
 *            .....
 *   
 * 
 * 
 *       
 * *****************************************************************************/



#include "math/matrix.h"
#include "util/console_out.h"
#include "common.h"

#define EPSILON            1.0e-24f
#define FZ                 (sizeof(float))
#define float__0(x)        (imath_fabs(x) < EPSILON)
#define float__eq(x,y)     float__0((x) - (y))
#define mat__empty(M)      !((M)->cols && (M)->rows)
#define mat__size(M)       ((M)->cols * (M)->rows)
#define mat__bytes(M)      (mat__size((M)) * FZ)
#define mat__idx(M, x, y)  (M)->data[(y)*(M)->cols+(x)]
#define mat__foreachRow(M,i)   for (iter i = 0; i < (M)->rows; ++i)
#define mat__foreachCol(M,i)   for (iter i = 0; i < (M)->cols; ++i)
#define mat__foreach(M,i)      for (iter i = 0; i < mat__size((M)); ++i)
#define mat__foreach2D(M,i,j)  for (iter i = 0, j; i < (M)->rows; ++i) for(j = 0; j < (M)->cols; ++j)

static inline int    matrix__dim_eq (const matrix a, const matrix b) {
  return (a.rows == b.rows) && (a.cols == b.cols);
}
static inline int    matrix__squared(const matrix a) {
  return a.cols == a.rows;
} 
static int           matrix__data_eq(const float *a, const float *b, iter s) {
  for (iter i = 0; i < s; ++i) if (!float__eq(a[i], b[i])) return 0;
  return 1;
}
static inline float *matrix__datadup(const matrix r) {
  iter bytes = mat__bytes(&r);
  float *ret = CAST(float*)util_malloc(bytes);
  util_memcpy(ret, r.data, bytes);
  return ret;
}
static inline void   matrix__move   (matrix *a, matrix *b) {
  a->data = CAST(float*)util_realloc(a->data, mat__bytes(b));
  util_memcpy(a->data, b->data, FZ * mat__size(b));
  a->cols = b->cols;
  a->rows = b->rows;
  matrix_free(b);
}

// initialize
matrix matrix_idt(iter cols,iter rows) {
  matrix r = (matrix){cols, rows, CAST(float*)util_calloc(sizeof(float), cols*rows)};
  for(iter i = 0, j = MIN(cols,rows); i < j; ++i) mat__idx(&r,i,i) = 1.0;
  return r;
}
inline matrix matrix_dup(const matrix a) {
  return (matrix){a.cols, a.rows, matrix__datadup(a)};
}
void matrix_midt(matrix *r) {
  util_memset(r->data, 0, mat__bytes(r));
  for(iter i = 0, j = MIN(r->cols,r->rows); i < j; ++i) mat__idx(r,i,i) = 1.0;
}
inline void matrix_set(matrix *a, const matrix b) {
  a->cols = b.cols;
  a->rows = b.rows;
  a->data = CAST(float*)util_realloc(a->data, mat__bytes(&b));
}
// free
void matrix_free(matrix *r) {
  util_memfree(r->data);
  util_memset(r, 0, sizeof(matrix));
}
// operaror compare
int matrix_equal(const matrix a, const matrix b) {
  return matrix__dim_eq(a,b) && matrix__data_eq(a.data, b.data, mat__size(&a));
}
// property
float matrix_det(const matrix r) {
  float res = 0.0f;
  if (matrix__squared(r)) {
    if (r.cols == 1) return r.data[0];
    float pm,mm;
    for (iter i = 1; i < r.rows; ++i) {
      mm = pm = 1.0f;
      mat__foreachCol(&r, j) {
        pm *= mat__idx(&r, (i - 1 + j) % r.cols, j);
        mm *= mat__idx(&r, (r.cols - i + j) % r.cols, j);
      }
      res += pm - mm;
    }
  }
  return res;
}
matrix matrix_minor(const matrix a, iter i, iter j) {
  matrix r = {.cols = a.cols - 1, .rows = a.rows - 1};
  if (mat__empty(&r)) {
    matrix_free(&r);
  } else {
    r.data = CAST(float*)util_malloc(mat__bytes(&r));
    mat__foreach2D(&a, x, y) {
      if (x == i || y == j) continue;
      mat__idx(&r, x - (x > i), y - (y > j)) = mat__idx(&a, x, y);
    }
  }
  return r;
}
matrix matrix_trn (const matrix r) {
  matrix d = {r.cols, r.rows, CAST(float*)util_malloc(mat__bytes(&r))};
  mat__foreach2D(&d,i,j) mat__idx(&d, j, i) = mat__idx(&r, i, j);
  return d;
}
matrix matrix_cof (const matrix r) {
  matrix d = {r.cols, r.rows, CAST(float*)util_malloc(mat__bytes(&r))};
  mat__foreach2D(&d,i,j) {
    matrix m = matrix_minor(r,i,j);
    mat__idx(&d,i,j) = matrix_det(m) * (1 - 2*((j&1)^(i&1)));
    matrix_free(&m);
  }
  return d;
}

matrix matrix_adj (const matrix r) {
  matrix d = {0};
  // is non square matrix can do adjoint?
  if (matrix__squared(r)) {
    d = (matrix){r.cols, r.rows, CAST(float*)util_malloc(mat__bytes(&r))};
    mat__foreach2D(&d,i,j) {
      matrix m = matrix_minor(r,i,j);
      mat__idx(&d,j,i) = matrix_det(m);
      mat__idx(&d,j,i)*= (1.0f - 2.0f*((j&1)^(i&1)));
      matrix_free(&m);
    }
  }
  return d;
}
matrix matrix_inv (const matrix r) {
  matrix d = matrix_dup(r);
  if (matrix_minv(&d)) matrix_free(&d);
  return d;
}
matrix matrix_mulf(const matrix a, float s) {
  matrix r = {a.cols, a.rows, CAST(float*)util_malloc(mat__bytes(&a))};
  mat__foreach(&r,i) r.data[i] = a.data[i] * s;
  return r;
}
matrix matrix_divf(const matrix a, float s) {
  matrix r = {a.cols, a.rows, CAST(float*)util_malloc(mat__bytes(&a))};
  mat__foreach(&r,i) r.data[i] = a.data[i] / s;
  return r;
}
matrix matrix_add (const matrix a, const matrix b) {
  matrix r = {a.cols, a.rows, CAST(float*)util_malloc(mat__bytes(&a))};
  mat__foreach(&r,i) r.data[i] = a.data[i] + b.data[i];
  return r;
}
matrix matrix_sub (const matrix a, const matrix b) {
  matrix r = {a.cols, a.rows, CAST(float*)util_malloc(mat__bytes(&a))};
  mat__foreach(&r,i) r.data[i] = a.data[i] - b.data[i];
  return r;
}
matrix matrix_mul (const matrix a,const matrix b) {
  matrix d = {0};
  if (a.cols == b.rows) {
    d.cols = b.cols;
    d.rows = a.rows;
    d.data = CAST(float*) util_calloc(FZ, mat__size(&d));
    mat__foreach2D(&a,i,j)
      mat__foreachCol(&b, k)
        mat__idx(&d, k, i) += mat__idx(&a, j, i) * mat__idx(&b, k, j);
  }
  return d;
}
inline matrix matrix_div (const matrix a,const matrix b) {
  matrix d = matrix_dup(a);
  if (matrix_mdiv(&d, b)) matrix_free(&d);
  return d;
}
void       matrix_mtrn(matrix *r) {
  mat__foreach2D(r,i,j) if (j ^ i) {
    *CAST(int*)&mat__idx(r, j, i) ^= *CAST(int*)&mat__idx(r, i, j);
    *CAST(int*)&mat__idx(r, i, j) ^= *CAST(int*)&mat__idx(r, j, i);
    *CAST(int*)&mat__idx(r, j, i) ^= *CAST(int*)&mat__idx(r, i, j);
  }
}
inline void matrix_mcof(matrix *r) {
  matrix d = matrix_cof(*r);
  matrix__move(r, &d);
}
// operator modify, 0 success 1 fail
inline int matrix_madj(matrix *r) {
  matrix d = matrix_adj(*r);
  if (mat__empty(&d)) return 1;
  matrix__move(r, &d);
  return 0;
}
inline int matrix_minv (matrix *r) {
  float det = matrix_det(*r);
  if (float__0(det) || matrix_madj(r)) return 1;
  matrix_mdivf(r, det);
  return 0;
}
void       matrix_mmulf(matrix *r, float s) {
  mat__foreach(r,i) r->data[i] *= s;
}
void       matrix_mdivf(matrix *r, float s) {
  mat__foreach(r,i) r->data[i] /= s;
}
int        matrix_madd(matrix *a,const matrix b) {
  if (!matrix__dim_eq(*a,b)) return 1; 
  mat__foreach(a,i) a->data[i] += b.data[i];
  return 0;
}
int        matrix_msub(matrix *a,const matrix b) {
  if (!matrix__dim_eq(*a,b)) return 1; 
  mat__foreach(a,i) a->data[i] -= b.data[i];
  return 0;
}
inline int matrix_mmul(matrix *a,const matrix b) {
  matrix d = matrix_mul(*a, b);
  if (mat__empty(&d)) return 1;
  matrix__move(a, &d);
  return 0;
}
inline int matrix_mdiv(matrix *a,const matrix b) {
  matrix d = matrix_dup(b);
  int ret = matrix_minv(&d) || matrix_mmul(a, d);
  matrix_free(&d);
  return ret;
}
void matrix_append_string(String *str,const matrix a) {
  string_append(str, "[%d, %d]{", a.cols, a.rows);
  mat__foreachRow(&a,i) {
    string_append_char(str, '{');
    mat__foreachCol(&a,j) {
      if(j) string_append_char(str, ',');
      string_append(str, "%02.2f", mat__idx(&a, j, i));
    }
    string_append_char(str, '}');
  }
  string_append_char(str, '}');
}
