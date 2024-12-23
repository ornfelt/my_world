#ifndef JKS_VEC4_H
#define JKS_VEC4_H

#include <stdint.h>

#define VEC4_DEF(name, type) \
	struct name \
	{ \
		type x; \
		type y; \
		type z; \
		type w; \
	}

#define VEC4_SET(v, vx, vy, vz, vw) \
	do \
	{ \
		(v).x = (vx); \
		(v).y = (vy); \
		(v).z = (vz); \
		(v).w = (vw); \
	} while (0)

#define VEC4_SETV(v, vv) \
	do \
	{ \
		(v).x = (vv); \
		(v).y = (vv); \
		(v).z = (vv); \
		(v).w = (vv); \
	} while (0)

#define VEC4_CPY(r, v) \
	do \
	{ \
		(r).x = (v).x; \
		(r).y = (v).y; \
		(r).z = (v).z; \
		(r).w = (v).w; \
	} while (0)

#define VEC4_OPV(r, a, v, op) \
	do \
	{ \
		(r).x = (a).x op (v); \
		(r).y = (a).y op (v); \
		(r).z = (a).z op (v); \
		(r).w = (a).w op (v); \
	} while (0)

#define VEC4_OP(r, a, b, op) \
	do \
	{ \
		(r).x = (a).x op (b).x; \
		(r).y = (a).y op (b).y; \
		(r).z = (a).z op (b).z; \
		(r).w = (a).w op (b).w; \
	} while (0)

#define VEC4_FNV(r, a, v, fn) \
	do \
	{ \
		(r).x = fn((a).x, v); \
		(r).y = fn((a).y, v); \
		(r).z = fn((a).z, v); \
		(r).w = fn((a).w, v); \
	} while (0)

#define VEC4_FN1(r, a, fn) \
	do \
	{ \
		(r).x = fn((a).x); \
		(r).y = fn((a).y); \
		(r).z = fn((a).z); \
		(r).w = fn((a).w); \
	} while (0)

#define VEC4_FN2(r, a, b, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x); \
		(r).y = fn((a).y, (b).y); \
		(r).z = fn((a).z, (b).z); \
		(r).w = fn((a).w, (b).w); \
	} while (0)

#define VEC4_FN3(r, a, b, c, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x, (c).x); \
		(r).y = fn((a).y, (b).y, (c).y); \
		(r).z = fn((a).z, (b).z, (c).z); \
		(r).w = fn((a).w, (b).w, (c).w); \
	} while (0)

#define VEC4_MIN(r, a, b) \
	do \
	{ \
		(r).x = (a).x < (b).x ? (a).x : (b).x; \
		(r).y = (a).y < (b).y ? (a).y : (b).y; \
		(r).z = (a).z < (b).z ? (a).z : (b).z; \
		(r).w = (a).w < (b).w ? (a).w : (b).w; \
	} while (0)

#define VEC4_MAX(r, a, b) \
	do \
	{ \
		(r).x = (a).x > (b).x ? (a).x : (b).x; \
		(r).y = (a).y > (b).y ? (a).y : (b).y; \
		(r).z = (a).z > (b).z ? (a).z : (b).z; \
		(r).w = (a).w > (b).w ? (a).w : (b).w; \
	} while (0)

#define VEC4_DOT(a, b) \
	((a).x * (b).x + (a).y * (b).y + (a).z * (b).z + (a).w * (b).w)

#define VEC4_NORM(a) \
	sqrt(VEC4_DOT(a, a))

#define VEC4_NORMALIZE(type, r, a) \
	do \
	{ \
		type norm = VEC4_NORM(a); \
		(r).x = (a).x / norm; \
		(r).y = (a).y / norm; \
		(r).z = (a).z / norm; \
		(r).w = (a).w / norm; \
	} while (0)

#define VEC4_NEGATE(r, v) \
	do \
	{ \
		(r).x = -(v).x; \
		(r).y = -(v).y; \
		(r).z = -(v).z; \
		(r).w = -(v).w; \
	} while (0)

#define VEC4_ADDV(r, a, v) VEC4_OPV(r, a, v, +)
#define VEC4_SUBV(r, a, v) VEC4_OPV(r, a, v, -)
#define VEC4_MULV(r, a, v) VEC4_OPV(r, a, v, *)
#define VEC4_DIVV(r, a, v) VEC4_OPV(r, a, v, /)
#define VEC4_MODV(r, a, v) VEC4_OPV(r, a, v, %)
#define VEC4_ANDV(r, a, v) VEC4_OPV(r, a, v, &)
#define VEC4_ORV (r, a, v) VEC4_OPV(r, a, v, |)
#define VEC4_XORV(r, a, v) VEC4_OPV(r, a, v, ^)

#define VEC4_ADD(r, a, b) VEC4_OP(r, a, b, +)
#define VEC4_SUB(r, a, b) VEC4_OP(r, a, b, -)
#define VEC4_MUL(r, a, b) VEC4_OP(r, a, b, *)
#define VEC4_DIV(r, a, b) VEC4_OP(r, a, b, /)
#define VEC4_MOD(r, a, b) VEC4_OP(r, a, b, %)
#define VEC4_AND(r, a, b) VEC4_OP(r, a, b, &)
#define VEC4_OR (r, a, b) VEC4_OP(r, a, b, |)
#define VEC4_XOR(r, a, b) VEC4_OP(r, a, b, ^)

#define VEC4_EQ(a, b) ((a).x == (b).x && (a).y == (b).y && (a).z == (b).z && (a).w == (b).w)

typedef VEC4_DEF(vec4f, float) vec4f_t;
typedef VEC4_DEF(vec4d, double) vec4d_t;
typedef VEC4_DEF(vec4i, int32_t) vec4i_t;
typedef VEC4_DEF(vec4u, uint32_t) vec4u_t;
typedef VEC4_DEF(vec4s, int16_t) vec4s_t;
typedef VEC4_DEF(vec4b, uint8_t) vec4b_t;

#endif
