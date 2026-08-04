/* *****************************************************************************
 * vector.h v0.0.0000
 * 
 * vector object
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _VECTOR_MATH_INCLUDED_
#define _VECTOR_MATH_INCLUDED_

#include "common.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
#define __VECTOR_PACK__
#else
#define __VECTOR_PACK__ __attribute__((packed))
#endif

typedef union {
	uint u[2];
	struct { uint x, y; };
} __VECTOR_PACK__ uvec2;

typedef union {
	float v[2];
	struct { float x, y; };
} __VECTOR_PACK__ vec2;
typedef union {
	float v[3];
	struct { float x, y, z; };
	struct { vec2 xy; float __pad1; };
	struct { float __pad2; vec2 yz; };
} __VECTOR_PACK__ vec3;
typedef union {
	float v[4];
	struct { float x, y, z, w; };
	struct { vec2 xy, zw; };
	struct { float __pad1; vec2 yz; float __pad2; };
	struct { vec3 xyz; float __pad3; };
	struct {  float __pad4; vec3 yzw; };
} __VECTOR_PACK__ vec4;

#undef __VECTOR_PACK__
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#define UVECTOR2_INIT(F) CLIT(uvec2){.x = (F), .y = (F)}
#define UVECTOR2(X,Y)    CLIT(uvec2){.x = (X), .y = (Y)}

#define VECTOR2_ZERO    CLIT(vec2){0.0f, 0.0f}
#define VECTOR2_ONE     CLIT(vec2){1.0f, 1.0f}
#define VECTOR2_INIT(F) CLIT(vec2){ (F),  (F)}
#define VECTOR2(X,Y)    CLIT(vec2){ (X),  (Y)}

#define VECTOR3_ZERO    CLIT(vec3){0.0f, 0.0f, 0.0f}
#define VECTOR3_ONE     CLIT(vec3){1.0f, 1.0f, 1.0f}
#define VECTOR3_INIT(F) CLIT(vec3){ (F),  (F),  (F)}
#define VECTOR3(X,Y,Z)  CLIT(vec3){ (X),  (Y),  (Z)}

#define VECTOR4_ZERO     CLIT(vec4){0.0f, 0.0f, 0.0f, 0.0f}
#define VECTOR4_ONE      CLIT(vec4){1.0f, 1.0f, 1.0f, 1.0f}
#define VECTOR4_INIT(F)  CLIT(vec4){ (F),  (F),  (F),  (F)}
#define VECTOR4(X,Y,Z,W) CLIT(vec4){ (X),  (Y),  (Z),  (W)}

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

vec2 vec2_add(vec2, vec2);
vec3 vec3_add(vec3, vec3);
vec4 vec4_add(vec4, vec4);

vec2 vec2_sub(vec2, vec2);
vec3 vec3_sub(vec3, vec3);
vec4 vec4_sub(vec4, vec4);

vec2 vec2_diff(vec2, vec2);
vec3 vec3_diff(vec3, vec3);
vec4 vec4_diff(vec4, vec4);

vec2 vec2_mul(vec2, float);
vec3 vec3_mul(vec3, float);
vec4 vec4_mul(vec4, float);

vec2 vec2_div(vec2, float);
vec3 vec3_div(vec3, float);
vec4 vec4_div(vec4, float);

float vec2_dist(vec2, vec2);
float vec3_dist(vec3, vec3);
float vec4_dist(vec4, vec4);

vec2 vec2_norm(vec2);
vec3 vec3_norm(vec3);
vec4 vec4_norm(vec4);

float vec2_dot(vec2, vec2);
float vec3_dot(vec3, vec3);
float vec4_dot(vec4, vec4);

float vec2_rad(vec2, vec2);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_VECTOR_MATH_INCLUDED_
