/* *****************************************************************************
 * matrix4.h v0.0.0000
 * 
 * Marrix 4x4 object
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _MATRIX_MATH_INCLUDED_
#define _MATRIX_MATH_INCLUDED_

#include "common.h"

#define MAT4_SIDE          4
#define MAT4_SIZE          16
#define MAT4_BYTES         (MAT4_SIZE * FZ)

typedef float *matrix4;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// initialize
matrix4 matrix4_idt  ();
matrix4 matrix4_dup  (const matrix4);
// init modify
void   matrix4_move (matrix4,matrix4);
void   matrix4_midt (matrix4);
void   matrix4_set  (matrix4,const matrix4);
// extra
void   matrix4_free (matrix4);
int    matrix4_equal(const matrix4, const matrix4);  // operaror compare
float  matrix4_det  (const matrix4);                // property
// operator duplicate return 0 sized matrix4 when error
matrix4 matrix4_trn  (const matrix4);                // transpose matrix4
matrix4 matrix4_mulf (const matrix4,float);
matrix4 matrix4_divf (const matrix4,float);
matrix4 matrix4_add  (const matrix4,const matrix4);
matrix4 matrix4_sub  (const matrix4,const matrix4);
matrix4 matrix4_mul  (const matrix4,const matrix4);
// operator modify
void matrix4_mtrn (matrix4);
void matrix4_mmulf(matrix4,float);
void matrix4_mdivf(matrix4,float);
void matrix4_madd (matrix4,const matrix4);
void matrix4_msub (matrix4,const matrix4);
void matrix4_mmul (matrix4,const matrix4);

void matrix4_append_string(String*,const matrix4);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_MATRIX_MATH_INCLUDED_
