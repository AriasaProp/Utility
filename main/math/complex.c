/* *****************************************************************************
 * complex.c v0.0.0000
 * 
 * Complex Number object
 * 
 * 
 * 
 * *****************************************************************************/

#include "math/complex.h"
#include "common.h"

#define EPSILON 9.7144060021e-5f
//initialize
void complex_set_cartesian(complex *c,const float r, const float i) {
  c->r = r; c->i = i;
}
void complex_set_polar(complex *c,const float m, const float a) {
  c->r = m * imath_cos(a);
  c->i = m * imath_sin(a);
}
// attribute 
inline float complex_real(const complex c) {
  return c.r;
}
inline float complex_imaginary(const complex c) {
  return c.i;
}
inline float complex_argument(const complex c) {
  return imath_atan2(c.i,c.r);
}
inline float complex_magnitude(const complex c) {
  return imath_hypot(c.r,c.i);
}
// compare 
inline int complex_eq  (const complex a, const complex b) {
  return (imath_fabs(b.r - a.r) < EPSILON) && (imath_fabs(b.i - a.i) < EPSILON);
} 
// operator duplicate 
inline complex complex_addf(const complex c, const float f) {
  return complex_cartesian(c.r + f, c.i);
}
inline complex complex_addfi(const complex c, const float f) {
  return complex_cartesian(c.r, c.i + f);
}
inline complex complex_subf(const complex c, const float f) {
  return complex_cartesian(c.r - f, c.i);
}
inline complex complex_subfi(const complex c, const float f) {
  return complex_cartesian(c.r, c.i - f);
}
inline complex complex_mulf(const complex c, const float f) {
  return complex_cartesian(c.r * f, c.i * f);
}
inline complex complex_mulfi(const complex c, const float f) {
  return complex_cartesian(-c.i * f, c.r * f);
}
inline complex complex_divf(const complex c, const float f) {
  return complex_cartesian(c.r / f, c.i / f);
}
inline complex complex_divfi(const complex c, const float f) {
  /*
   *  (a+bi)    (b-ai)
   *  ------ => ------
   *    ci        c
   */
  return complex_cartesian(c.i / f, -c.r / f);
}
inline complex complex_powf(const complex c, const float f) {
  float mag = imath_exp(imath_log(complex_magnitude(c)) * f);
  float arg = complex_argument(c) * f;
  return complex_polar(mag,arg);
}
inline complex complex_powfi(const complex c, const float f) {
  /*   (  i@)(bi)     bi*ln(r)   -b@
   *   (re  )    => e           e
   *
   */
  float mag = imath_exp(-f * complex_argument(c));
  float arg = imath_log(complex_magnitude(c)) * f;
  return complex_polar(mag,arg);
}
inline complex complex_add (const complex a, const complex b) {
  return complex_cartesian(a.r + b.r, a.i + b.i);
}
inline complex complex_sub (const complex a, const complex b) {
  return complex_cartesian(a.r - b.r, a.i - b.i);
}
inline complex complex_mul (const complex a, const complex b) {
  return complex_cartesian(
    a.r * b.r - a.i * b.i,
    a.r * b.i + a.i * b.r
  );
}
inline complex complex_div (const complex a, const complex b) {
   /*
    *  (a+bi)(c-di)     (ac+bd)      (bc-ad)i
    *  ------------ => ---------- + ----------
    *    (c2+d2)        (c2+d2)      (c2+d2)
    *
    *
    *
    */
  float d = (b.r*b.r + b.i*b.i);
  return complex_cartesian(
    (a.r * b.r + a.i * b.i) / d,
    (a.i * b.r - a.r * b.i) / d
  );
}
inline complex complex_pow (const complex a, const complex b) {
  /*
   *  r = √(c2+d2)
   *  @ = 2 * atan((c - r) / d)
   *              (            -d2              )
   *   => 4 * atan(-----------------------------)
   *              (d +  √(d2 + dc - d√(c2 + d2))) * (c - √(c2 + d2))
   *  R = ln(c2+d2)/2
   *
   *
   *   (  i@)(a+bi)    (ln(r)a-@b)    (ln(r)b+@a)i
   *   (re  )      => e              e 
   *
   *
   *                     a         (b*ln(r)+@a)i
   *                =>  r         e 
   *                   -----  ×
   *                     @b
   *                    e
   *
   *
   *
   *
   */
  float lmagA = imath_log(complex_magnitude(a));
  float argA = complex_argument(a);
  float mag = imath_exp(lmagA * b.r - argA * b.i);
  float arg = lmagA * b.i + argA * b.r;
  return complex_polar(mag,arg);
}
// operator modify 
inline void complex_addsf(complex *c, const float f) {
  c->r += f;
}
inline void complex_addsfi(complex *c, const float f) {
  c->i += f;
}
inline void complex_subsf(complex *c, const float f) {
  c->r -= f;
}
inline void complex_subsfi(complex *c, const float f) {
  c->i -= f;
}
inline void complex_mulsf(complex *c, const float f) {
  c->r *= f;
  c->i *= f;
}
inline void complex_mulsfi(complex *c, const float f) {
  float *F = CAST(float*)c;
  imath_swap(F, F + 1, sizeof(float));
  c->r *= -f;
  c->i *= f;
}
inline void complex_divsf(complex *c, const float f) {
  c->r /= f;
  c->i /= f;
}
inline void complex_divsfi(complex *c, const float f) {
  float *F = CAST(float*)c;
  imath_swap(F, F + 1, sizeof(float));
  c->r /= f;
  c->i /= -f;
}
inline void complex_powsf(complex *c, const float f) {
  float mag = imath_exp(imath_log(complex_magnitude(*c)) * f);
  float arg = complex_argument(*c) * f;
  complex_set_polar(c,mag,arg);
}
inline void complex_powsfi(complex *c, const float f) {
  float mag = imath_exp(-complex_argument(*c) * f);
  float arg = complex_magnitude(*c) * f;
  complex_set_polar(c,mag,arg);
}
inline void complex_adds (complex *a, const complex b) {
  a->r += b.r;
  a->i += b.i;
}
inline void complex_subs (complex *a, const complex b) {
  a->r -= b.r;
  a->i -= b.i;
}
inline void complex_muls (complex *a, const complex b) {
  float r = a->r * b.r - a->i * b.i;
  float i = a->r * b.i + a->i * b.r;
  a->r = r;
  a->i = i;
}
inline void complex_divs (complex *a, const complex b) {
  float d = b.r * b.r - b.i * b.i;
  a->r /= d;
  a->i /= d;
  float r = a->r * b.r + a->i * b.i;
  float i = a->i * b.r - a->r * b.i;
  a->r = r;
  a->i = i;
}
inline void complex_pows (complex *a, const complex b) {
  float lmagA = imath_log(complex_magnitude(*a));
  float argA = complex_argument(*a);
  float mag = imath_exp(b.r * lmagA - argA * b.i);
  float arg = argA * b.r + b.i * lmagA;
  complex_set_polar(a,mag,arg);
}
// return to string
inline void complex_append_string(String *str, const complex a) {
  string_append_char(str, '{');
  if (a.r || a.i) {
    if (a.r) string_append(str, "%.2f", a.r);
    if (a.i) {
      if (a.i >= 0.0f) string_append_char(str, '+');
      string_append(str, "%.2fi", a.i);
    }
  } else string_append_char(str, '0');
  string_append_char(str, '}');
}

