/* *****************************************************************************
 * complex.h v0.0.0000
 * 
 * Complex Number object
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _COMPLEX_MATH_INCLUDED_
#define _COMPLEX_MATH_INCLUDED_

#include "common.h"

// store into cartesian form
typedef struct { float r, i; } complex;

// initialize
#define complex_cartesian(R,I) (complex){.r = (R), .i = (I)}
#define complex_polar(M,A) (complex){ .r = (M) * imath_cos(A), .i = (M) * imath_sin(A)}

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// initialize
void    complex_set_cartesian(complex*,const float, const float);
void    complex_set_polar    (complex*,const float, const float);
// attribute 
float   complex_real     (const complex);
float   complex_imaginary(const complex);
float   complex_argument (const complex);
float   complex_magnitude(const complex);
// compare
int     complex_equal(const complex, const complex);  
// extra
complex complex_inv(const complex);
// operator duplicate
complex complex_addf (const complex, const float);
complex complex_addfi(const complex, const float);
complex complex_subf (const complex, const float);
complex complex_subfi(const complex, const float);
complex complex_mulf (const complex, const float);
complex complex_mulfi(const complex, const float);
complex complex_divf (const complex, const float);
complex complex_divfi(const complex, const float);
complex complex_powf (const complex, const float);
complex complex_rootf(const complex, const float);
complex complex_powfi(const complex, const float);
complex complex_rootfi(const complex, const float);
complex complex_add  (const complex, const complex);
complex complex_sub  (const complex, const complex);
complex complex_mul  (const complex, const complex);
complex complex_div  (const complex, const complex);
complex complex_pow  (const complex, const complex);
complex complex_root (const complex, const complex);
// extra modify 
void    complex_minv  (complex*);
// operator modify 
void    complex_maddf (complex*, const float);
void    complex_maddfi(complex*, const float);
void    complex_msubf (complex*, const float);
void    complex_msubfi(complex*, const float);
void    complex_mmulf (complex*, const float);
void    complex_mmulfi(complex*, const float);
void    complex_mdivf (complex*, const float);
void    complex_mdivfi(complex*, const float);
void    complex_mpowf (complex*, const float);
void    complex_mrootf(complex*, const float);
void    complex_mpowfi(complex*, const float);
void    complex_mrootfi(complex*, const float);
void    complex_madd  (complex*, const complex);
void    complex_msub  (complex*, const complex);
void    complex_mmul  (complex*, const complex);
void    complex_mdiv  (complex*, const complex);
void    complex_mpow  (complex*, const complex);
void    complex_mroot (complex*, const complex);
// return to string
void    complex_append_string(String *, const complex);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // _COMPLEX_MATH_INCLUDED_
