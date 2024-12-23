#ifndef JKS_MAT2_H
#define JKS_MAT2_H

#include "vec2.h"

#define MAT2_DEF(name, type) \
	struct name \
	{ \
		VEC2_DEF(, type) x; \
		VEC2_DEF(, type) y; \
	}

#define MAT2_MUL1(r, a, b, X, Y) (r).X.Y = (a).x.Y * (b).X.x + (a).y.Y * (b).X.y

#define MAT2_MUL2(r, a, b, X) \
	MAT2_MUL1(r, a, b, X, x); \
	MAT2_MUL1(r, a, b, X, y); \

#define MAT2_MUL(r, a, b) \
	do \
	{ \
		MAT2_MUL2(r, a, b, x); \
		MAT2_MUL2(r, a, b, y); \
	} while (0)

#define MAT2_VEC2_MUL1(r, m, v, X) (r).X = (v).x * (m).x.X + (v).y * (m).y.X

#define MAT2_VEC2_MUL(r, m, v) \
	do \
	{ \
		MAT2_VEC2_MUL1(r, m, v, x); \
		MAT2_VEC2_MUL1(r, m, v, y); \
	} while (0)

#define MAT2_IDENTITY(m) \
	do \
	{ \
		VEC2_SET((m).x, 1, 0); \
		VEC2_SET((m).y, 0, 1); \
	} while (0)

typedef MAT2_DEF(mat2f, float) mat2f_t;

#endif
