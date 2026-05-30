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

#include "util/imath.h"
#include "util/string.h"

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
int     complex_eq  (const complex, const complex);  
// operator duplicate 
complex complex_addf(const complex, const float);
complex complex_addfi(const complex, const float);
complex complex_subf(const complex, const float);
complex complex_subfi(const complex, const float);
complex complex_mulf(const complex, const float);
complex complex_mulfi(const complex, const float);
complex complex_divf(const complex, const float);
complex complex_divfi(const complex, const float);
complex complex_powf(const complex, const float);
complex complex_powfi(const complex, const float);
complex complex_add (const complex, const complex);
complex complex_sub (const complex, const complex);
complex complex_mul (const complex, const complex);
complex complex_div (const complex, const complex);
complex complex_pow (const complex, const complex);
// operator modify 
void    complex_addsf(complex*, const float);
void    complex_addsfi(complex*, const float);
void    complex_subsf(complex*, const float);
void    complex_subsfi(complex*, const float);
void    complex_mulsf(complex*, const float);
void    complex_mulsfi(complex*, const float);
void    complex_divsf(complex*, const float);
void    complex_divsfi(complex*, const float);
void    complex_powsf(complex*, const float);
void    complex_powsfi(complex*, const float);
void    complex_adds (complex*, const complex);
void    complex_subs (complex*, const complex);
void    complex_muls (complex*, const complex);
void    complex_divs (complex*, const complex);
void    complex_pows (complex*, const complex);
// return to string
void    complex_append_string(String *, const complex);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // _COMPLEX_MATH_INCLUDED_