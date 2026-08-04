#ifndef __RAYTRACING_INCLUDED__
#define __RAYTRACING_INCLUDED__

#include "common.h"
#include "math/mat4.h"
#include "math/vector.h"
#include "array/darray.h"

typedef struct {
	float x;
} rayTracing_Opt;

typedef struct {
	iter cap, count;
	vec3 *items;
} Vertices;
typedef struct {
	iter cap, count;
	ushrt *items;
} Indices;
typedef struct {
	Vertices vert;
	Indices idx;
	mat4 trans;
} Object;
typedef struct {
	Object *items;
	iter cap, count;
} Objects;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool rayTracing_render_opt(ubyte*,const Objects, const uvec2, rayTracing_Opt);

#define rayTracing_render(b, o, s, ...) rayTracing_render_opt((b), (o), (s), CLIT(rayTracing_Opt){__VA_ARGS__})



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __RAYTRACING_INCLUDED__