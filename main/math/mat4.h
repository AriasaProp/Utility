/* *****************************************************************************
 * mat4.h v0.0.0000
 * 
 * Marrix 4x4 object
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _MATRIX4_MATH_INCLUDED_
#define _MATRIX4_MATH_INCLUDED_

#include "common.h"
#include "array/dstring.h"

#define MAT4_IDT CLIT(mat4){1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f}
#define MAT4(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) CLIT(mat4){(A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P)}

#ifdef _MSC_VER
#pragma pack(push, 1)
#define __MATRIX4_PACK__
#else
#define __MATRIX4_PACK__ __attribute__((packed))
#endif

typedef union {
	float v[16];
	float m[4][4];
	struct {
		float m00, m10, m20, m30;
		float m01, m11, m21, m31;
		float m02, m12, m22, m32;
		float m03, m13, m23, m33;
	};
} __MATRIX4_PACK__ mat4;

#undef __MATRIX4_PACK__
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// extra
bool   mat4_equal(const mat4, const mat4);  // operaror compare
float  mat4_det  (const mat4);                // property
// operator duplicate return 0 sized mat4 when error
mat4 mat4_trn  (const mat4);                // transpose mat4
mat4 mat4_mulf (const mat4,float);
mat4 mat4_divf (const mat4,float);
mat4 mat4_add  (const mat4,const mat4);
mat4 mat4_sub  (const mat4,const mat4);
mat4 mat4_mul  (const mat4,const mat4);
// operator modify
void mat4_mtrn (mat4);
void mat4_mmulf(mat4,float);
void mat4_mdivf(mat4,float);
void mat4_madd (mat4,const mat4);
void mat4_msub (mat4,const mat4);
void mat4_mmul (mat4,const mat4);

void mat4_append_dstring(dstring*,const mat4);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_MATRIX4_MATH_INCLUDED_
