#ifndef JKS_MAT3_H
#define JKS_MAT3_H

#include "vec3.h"

#define MAT3_DEF(name, type) \
	struct name \
	{ \
		VEC3_DEF(, type) x; \
		VEC3_DEF(, type) y; \
		VEC3_DEF(, type) z; \
	}

#define MAT3_MUL1(r, a, b, X, Y) (r).X.Y = (a).x.Y * (b).X.x + (a).y.Y * (b).X.y + (a).z.Y * (b).X.z

#define MAT3_MUL3(r, a, b, X) \
	MAT3_MUL1(r, a, b, X, x); \
	MAT3_MUL1(r, a, b, X, y); \
	MAT3_MUL1(r, a, b, X, z); \

#define MAT3_MUL(r, a, b) \
	do \
	{ \
		MAT3_MUL3(r, a, b, x); \
		MAT3_MUL3(r, a, b, y); \
		MAT3_MUL3(r, a, b, z); \
	} while (0)

#define MAT3_VEC3_MUL1(r, m, v, X) (r).X = (v).x * (m).x.X + (v).y * (m).y.X + (v).z * (m).z.X

#define MAT3_VEC3_MUL(r, m, v) \
	do \
	{ \
		MAT3_VEC3_MUL1(r, m, v, x); \
		MAT3_VEC3_MUL1(r, m, v, y); \
		MAT3_VEC3_MUL1(r, m, v, z); \
	} while (0)

#define MAT3_ROTATEX(t, r, m, v) \
	do \
	{ \
		t c = cos(v); \
		t s = sin(v); \
		VEC3_MULV((r).y, (m).y, c); \
		VEC3_MULV((r).z, (m).z, s); \
		VEC3_ADD((r).y, (r).y, (r).z); \
		VEC3_MULV((r).z, (m).y, -s); \
		VEC3_MULV((r).x, (m).z, c); \
		VEC3_ADD((r).z, (r).z, (r).x); \
		VEC3_CPY((r).x, (m).x); \
	} while (0)

#define MAT3_ROTATEY(t, r, m, v) \
	do \
	{ \
		t c = cos(v); \
		t s = sin(v); \
		VEC3_MULV((r).x, (m).x, c); \
		VEC3_MULV((r).y, (m).z, -s); \
		VEC3_ADD((r).x, (r).x, (r).y); \
		VEC3_MULV((r).z, (m).x, s); \
		VEC3_MULV((r).y, (m).z, c); \
		VEC3_ADD((r).z, (r).z, (r).y); \
		VEC3_CPY((r).y, (m).y); \
	} while (0)

#define MAT3_ROTATEZ(t, r, m, v) \
	do \
	{ \
		t c = cos(v); \
		t s = sin(v); \
		VEC3_MULV((r).x, (m).x, c); \
		VEC3_MULV((r).y, (m).y, s); \
		VEC3_ADD((r).x, (r).x, (r).y); \
		VEC3_MULV((r).y, (m).x, -s); \
		VEC3_MULV((r).z, (m).y, c); \
		VEC3_ADD((r).y, (r).y, (r).z); \
		VEC3_CPY((r).z, (m).z); \
	} while (0)

#define MAT3_SCALE(r, m, v) \
	do \
	{ \
		VEC3_MULV((r).x, (m).x, (v).x); \
		VEC3_MULV((r).y, (m).y, (v).y); \
		VEC3_MULV((r).z, (m).z, (v).z); \
	} while (0)

#define MAT3_SCALEV(r, m, v) \
	do \
	{ \
		VEC3_MULV((r).x, (m).x, v); \
		VEC3_MULV((r).y, (m).y, v); \
		VEC3_MULV((r).z, (m).z, v); \
	} while (0)

#define MAT3_IDENTITY(m) \
	do \
	{ \
		VEC3_SET((m).x, 1, 0, 0); \
		VEC3_SET((m).y, 0, 1, 0); \
		VEC3_SET((m).z, 0, 0, 1); \
	} while (0)

typedef MAT3_DEF(mat3f, float) mat3f_t;

#endif
