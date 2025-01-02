#ifndef JKS_VEC2_H
#define JKS_VEC2_H

#include <stdint.h>

#define VEC2_DEF(name, type) \
	struct name \
	{ \
		type x; \
		type y; \
	}

#define VEC2_SET(v, vx, vy) \
	do \
	{ \
		(v).x = (vx); \
		(v).y = (vy); \
	} while (0)

#define VEC2_SETV(v, vv) \
	do \
	{ \
		(v).x = (vv); \
		(v).y = (vv); \
	} while (0)

#define VEC2_CPY(r, v) \
	do \
	{ \
		(r).x = (v).x; \
		(r).y = (v).y; \
	} while (0)

#define VEC2_OPV(r, a, v, op) \
	do \
	{ \
		(r).x = (a).x op (v); \
		(r).y = (a).y op (v); \
	} while (0)

#define VEC2_OP(r, a, b, op) \
	do \
	{ \
		(r).x = (a).x op (b).x; \
		(r).y = (a).y op (b).y; \
	} while (0)

#define VEC2_FNV(r, a, v, fn) \
	do \
	{ \
		(r).x = fn((a).x, v); \
		(r).y = fn((a).y, v); \
	} while (0)

#define VEC2_FN1(r, a, fn) \
	do \
	{ \
		(r).x = fn((a).x); \
		(r).y = fn((a).y); \
	} while (0)

#define VEC2_FN2(r, a, b, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x); \
		(r).y = fn((a).y, (b).y); \
	} while (0)

#define VEC2_FN3(r, a, b, c, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x, (c).x); \
		(r).y = fn((a).y, (b).y, (c).y); \
	} while (0)

#define VEC2_MIN(r, a, b) \
	do \
	{ \
		(r).x = (a).x < (b).x ? (a).x : (b).x; \
		(r).y = (a).y < (b).y ? (a).y : (b).y; \
	} while (0)

#define VEC2_MAX(r, a, b) \
	do \
	{ \
		(r).x = (a).x > (b).x ? (a).x : (b).x; \
		(r).y = (a).y > (b).y ? (a).y : (b).y; \
	} while (0)

#define VEC2_DOT(a, b) \
	((a).x * (b).x + (a).y * (b).y)

#define VEC2_NORM(a) \
	sqrt(VEC2_DOT(a))

#define VEC2_NORMALIZE(type, r, a) \
	do \
	{ \
		type norm = VEC2_NORM(a); \
		(r).x = (a).x / norm; \
		(r).y = (a).y / norm; \
	} while (0)

#define VEC2_ADDV(r, a, v) VEC2_OPV(r, a, v, +)
#define VEC2_SUBV(r, a, v) VEC2_OPV(r, a, v, -)
#define VEC2_MULV(r, a, v) VEC2_OPV(r, a, v, *)
#define VEC2_DIVV(r, a, v) VEC2_OPV(r, a, v, /)
#define VEC2_MODV(r, a, v) VEC2_OPV(r, a, v, %)
#define VEC2_ANDV(r, a, v) VEC2_OPV(r, a, v, &)
#define VEC2_ORV (r, a, v) VEC2_OPV(r, a, v, |)
#define VEC2_XORV(r, a, v) VEC2_OPV(r, a, v, ^)

#define VEC2_ADD(r, a, b) VEC2_OP(r, a, b, +)
#define VEC2_SUB(r, a, b) VEC2_OP(r, a, b, -)
#define VEC2_MUL(r, a, b) VEC2_OP(r, a, b, *)
#define VEC2_DIV(r, a, b) VEC2_OP(r, a, b, /)
#define VEC2_MOD(r, a, b) VEC2_OP(r, a, b, %)
#define VEC2_AND(r, a, b) VEC2_OP(r, a, b, &)
#define VEC2_OR (r, a, b) VEC2_OP(r, a, b, |)
#define VEC2_XOR(r, a, b) VEC2_OP(r, a, b, ^)

#define VEC2_EQ(a, b) ((a).x == (b).x && (a).y == (b).y)

typedef VEC2_DEF(vec2f, float) vec2f_t;
typedef VEC2_DEF(vec2d, double) vec2d_t;
typedef VEC2_DEF(vec2i, int32_t) vec2i_t;
typedef VEC2_DEF(vec2u, uint32_t) vec2u_t;
typedef VEC2_DEF(vec2s, int16_t) vec2s_t;
typedef VEC2_DEF(vec2b, uint8_t) vec2b_t;

#endif
