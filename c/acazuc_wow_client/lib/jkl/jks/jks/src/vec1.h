#ifndef JKS_VEC1_H
#define JKS_VEC1_H

#include <stdint.h>

#define VEC1_DEF(name, type) \
	struct name \
	{ \
		type x; \
	}

#define VEC1_SET(v, vx) \
	do \
	{ \
		(v).x = (vx); \
	} while (0)

#define VEC1_SETV(v, vv) \
	do \
	{ \
		(v).x = (vv); \
	} while (0)

#define VEC1_CPY(r, v) \
	do \
	{ \
		(r).x = (v).x; \
	} while (0)

#define VEC1_OPV(r, a, v, op) \
	do \
	{ \
		(r).x = (a).x op (v); \
	} while (0)

#define VEC1_OP(r, a, b, op) \
	do \
	{ \
		(r).x = (a).x op (b).x; \
	} while (0)

#define VEC1_FNV(r, a, v, fn) \
	do \
	{ \
		(r).x = fn((a).x, v); \
	} while (0)

#define VEC1_FN1(r, a, fn) \
	do \
	{ \
		(r).x = fn((a).x); \
	} while (0)

#define VEC1_FN2(r, a, b, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x); \
	} while (0)

#define VEC1_FN3(r, a, b, c, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x, (c).x); \
	} while (0)

#define VEC1_MIN(r, a, b) \
	do \
	{ \
		(r).x = (a).x < (b).x ? (a).x : (b).x; \
	} while (0)

#define VEC1_MAX(r, a, b) \
	do \
	{ \
		(r).x = (a).x > (b).x ? (a).x : (b).x; \
	} while (0)

#define VEC1_DOT(a, b) \
	((a).x * (b).x)

#define VEC1_NORM(a) \
	sqrt(VEC1_DOT(a, a))

#define VEC1_NORMALIZE(type, r, a) \
	do \
	{ \
		type norm = VEC1_NORM(a); \
		(r).x = (a).x / norm; \
	} while (0)

#define VEC1_ADDV(r, a, v) VEC1_OPV(r, a, v, +)
#define VEC1_SUBV(r, a, v) VEC1_OPV(r, a, v, -)
#define VEC1_MULV(r, a, v) VEC1_OPV(r, a, v, *)
#define VEC1_DIVV(r, a, v) VEC1_OPV(r, a, v, /)
#define VEC1_MODV(r, a, v) VEC1_OPV(r, a, v, %)
#define VEC1_ANDV(r, a, v) VEC1_OPV(r, a, v, &)
#define VEC1_ORV (r, a, v) VEC1_OPV(r, a, v, |)
#define VEC1_XORV(r, a, v) VEC1_OPV(r, a, v, ^)

#define VEC1_ADD(r, a, b) VEC1_OP(r, a, b, +)
#define VEC1_SUB(r, a, b) VEC1_OP(r, a, b, -)
#define VEC1_MUL(r, a, b) VEC1_OP(r, a, b, *)
#define VEC1_DIV(r, a, b) VEC1_OP(r, a, b, /)
#define VEC1_MOD(r, a, b) VEC1_OP(r, a, b, %)
#define VEC1_AND(r, a, b) VEC1_OP(r, a, b, &)
#define VEC1_OR (r, a, b) VEC1_OP(r, a, b, |)
#define VEC1_XOR(r, a, b) VEC1_OP(r, a, b, ^)

#define VEC1_EQ(a, b) ((a).x == (b).x)

typedef VEC1_DEF(vec1f, float) vec1f_t;
typedef VEC1_DEF(vec1d, double) vec1d_t;
typedef VEC1_DEF(vec1i, int32_t) vec1i_t;
typedef VEC1_DEF(vec1u, uint32_t) vec1u_t;
typedef VEC1_DEF(vec1s, int16_t) vec1s_t;
typedef VEC1_DEF(vec1b, uint8_t) vec1b_t;

#endif
