/* *****************************************************************************
 * 
 * mat4.c v0.0.0000
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



#include "math/mat4.h"
#include "util/console_out.h"
#include "common.h"

#define EPSILON            1.0e-24f
#define FZ                 (sizeof(float))
#define float__0(x)        (imath_fabs(x) < EPSILON)
#define float__eq(x,y)     float__0((x) - (y))

#define mat4__foreach(i)      for (iter i = 0; i < 16; ++i)
#define mat4__foreachSide(i)  for (iter i = 0; i <  4; ++i)
#define mat4__foreach2D(i,j)  for (iter i = 0, j; i < 4; ++i) for(j = 0; j < 4; ++j)

// operaror compare
bool mat4_equal(const mat4 a, const mat4 b) {
  mat4__foreach(i) if (!float__eq(a.v[i],b.v[i])) return false;
  return true;
}
// property
float mat4_det(const mat4 r) {
  float res = 0.0f;
  float pm,mm;
  for (iter i = 1; i < 4; ++i) {
    mm = pm = 1.0f;
    mat4__foreachSide(j) {
      pm *= r.m[j][(i - 1 + j) % 4];
      mm *= r.m[j][(4 - i + j) % 4];
    }
    res += pm;
    res -= mm;
  }
  return res;
}
mat4 mat4_trn(const mat4 r) {
	mat4 d;
  mat4__foreachSide(i) d.m[i][j] = r.m[j][i];
  return d;
}
mat4 mat4_mulf(const mat4 a, float s) {
  mat4 r;
  mat4__foreach(i) r.v[i] = a.v[i] * s;
  return r;
}
mat4 mat4_divf(const mat4 a, float s) {
  mat4 r;
  mat4__foreach(i) r.v[i] = a.v[i] / s;
  return r;
}
mat4 mat4_add(const mat4 a, const mat4 b) {
  mat4 r;
  mat4__foreach(i) r.v[i] = a.v[i] + b.v[i];
  return r;
}
mat4 mat4_sub(const mat4 a, const mat4 b) {
  mat4 r;
  mat4__foreach(i) r.v[i] = a.v[i] - b.v[i];
  return r;
}
mat4 mat4_mul(const mat4 a, const mat4 b) {
  mat4 r = {0};
  mat4__foreach2D(i,j)
    mat4__foreachSide(k)
      r.m[i][k] += a.m[i][j] * b.m[j][k];
  return r;
}
void mat4_mtrn(mat4 r) {
  mat4__foreach2D(i,j) if (j ^ i) {
    *CAST(int*)&(r.m[i][j]) ^= *CAST(int*)&(r.m[j][i]);
    *CAST(int*)&(r.m[j][i]) ^= *CAST(int*)&(r.m[i][j]);
    *CAST(int*)&(r.m[i][j]) ^= *CAST(int*)&(r.m[j][i]);
  }
}
// operator modify, 0 success 1 fail
void mat4_mmulf(mat4 r, float s) {
  mat4__foreach(i) r.v[i] *= s;
}
void mat4_mdivf(mat4 r, float s) {
  mat4__foreach(i) r.v[i] /= s;
}
void mat4_madd(mat4 a,const mat4 b) {
  mat4__foreach(i) a.v[i] += b.v[i];
}
void mat4_msub(mat4 a,const mat4 b) {
  mat4__foreach(i) a.v[i] -= b.v[i];
}
void mat4_mmul(mat4 a,const mat4 b) {
  a = mat4_mul(a, b);
}
void mat4_append_dstring(dstring *str,const mat4 a) {
  string_append(str, "[4x4]{");
  mat4__foreachSide(i) {
    string_append_char(str, '{');
    mat4__foreachSide(j) {
      if(j) string_append_char(str, ',');
      string_append(str, "%02.2e", a[i][j]);
    }
    string_append_char(str, '}');
  }
  string_append_char(str, '}');
}
