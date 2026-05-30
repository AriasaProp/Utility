/* *****************************************************************************
 * matrix.h v0.0.0000
 * 
 * Marrix(x,y) object
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _MATRIX_MATH_INCLUDED_
#define _MATRIX_MATH_INCLUDED_

#include "common.h"

typedef struct {
  ubyte cols, rows;
  float *data;
} matrix;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// initialize
matrix matrix_idt  (iter,iter);
matrix matrix_dup  (const matrix);
void   matrix_midt (matrix*);
void   matrix_set  (matrix*,const matrix);
void   matrix_free (matrix *);                    // free
int    matrix_equal(const matrix, const matrix);  // operaror compare
float  matrix_det  (const matrix);                // property
matrix matrix_minor(const matrix,iter,iter);
// operator duplicate return 0 sized matrix when error
matrix matrix_trn  (const matrix);                // transpose matrix
matrix matrix_cof  (const matrix);                // transpose matrix
matrix matrix_adj  (const matrix);
matrix matrix_inv  (const matrix);
matrix matrix_mulf (const matrix,float);
matrix matrix_divf (const matrix,float);
matrix matrix_add  (const matrix,const matrix);
matrix matrix_sub  (const matrix,const matrix);
matrix matrix_mul  (const matrix,const matrix);
matrix matrix_div  (const matrix,const matrix);
// operator modify
void matrix_mtrn (matrix*);
void matrix_mcof (matrix*);
int  matrix_madj (matrix*);
int  matrix_minv (matrix*);
void matrix_mmulf(matrix*,float);
void matrix_mdivf(matrix*,float);
int  matrix_madd (matrix*,const matrix);
int  matrix_msub (matrix*,const matrix);
int  matrix_mmul (matrix*,const matrix);
int  matrix_mdiv (matrix*,const matrix);

void matrix_append_string(String*,const matrix);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_MATRIX_MATH_INCLUDED_
