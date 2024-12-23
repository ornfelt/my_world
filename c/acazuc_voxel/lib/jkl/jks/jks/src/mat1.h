#ifndef JKS_MAT1_H
#define JKS_MAT1_H

#include "vec1.h"

#define MAT1_DEF(name, type) \
	struct name \
	{ \
		VEC1_DEF(, type) x; \
	}

#define MAT1_MUL(r, a, b, X) \
	do \
	{ \
		(r).x.x = (a).x.x * (b).x.x; \
	} while (0)

#define MAT1_VEC1_MUL(r, m, v) \
	do \
	{ \
		(r).x.x = (v).x * (m).x.x; \
	} while (0)

#define MAT1_IDENTITY(m) \
	do \
	{ \
		VEC1_SET((m).x, 1); \
	} while (0)

typedef MAT1_DEF(mat1f, float) mat1f_t;

#endif
