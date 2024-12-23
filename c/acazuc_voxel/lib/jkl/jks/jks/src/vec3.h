#ifndef JKS_VEC3_H
#define JKS_VEC3_H

#include <stdint.h>

#define VEC3_DEF(name, type) \
	struct name \
	{ \
		type x; \
		type y; \
		type z; \
	}

#define VEC3_SET(v, vx, vy, vz) \
	do \
	{ \
		(v).x = (vx); \
		(v).y = (vy); \
		(v).z = (vz); \
	} while (0)

#define VEC3_SETV(v, vv) \
	do \
	{ \
		(v).x = (vv); \
		(v).y = (vv); \
		(v).z = (vv); \
	} while (0)

#define VEC3_CPY(r, v) \
	do \
	{ \
		(r).x = (v).x; \
		(r).y = (v).y; \
		(r).z = (v).z; \
	} while (0)

#define VEC3_OPV(r, a, v, op) \
	do \
	{ \
		(r).x = (a).x op (v); \
		(r).y = (a).y op (v); \
		(r).z = (a).z op (v); \
	} while (0)

#define VEC3_OP(r, a, b, op) \
	do \
	{ \
		(r).x = (a).x op (b).x; \
		(r).y = (a).y op (b).y; \
		(r).z = (a).z op (b).z; \
	} while (0)

#define VEC3_FNV(r, a, v, fn) \
	do \
	{ \
		(r).x = fn((a).x, v); \
		(r).y = fn((a).y, v); \
		(r).z = fn((a).z, v); \
	} while (0)

#define VEC3_FN1(r, a, fn) \
	do \
	{ \
		(r).x = fn((a).x); \
		(r).y = fn((a).y); \
		(r).z = fn((a).z); \
	} while (0)

#define VEC3_FN2(r, a, b, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x); \
		(r).y = fn((a).y, (b).y); \
		(r).z = fn((a).z, (b).z); \
	} while (0)

#define VEC3_FN3(r, a, b, c, fn) \
	do \
	{ \
		(r).x = fn((a).x, (b).x, (c).x); \
		(r).y = fn((a).y, (b).y, (c).y); \
		(r).z = fn((a).z, (b).z, (c).z); \
	} while (0)

#define VEC3_MIN(r, a, b) \
	do \
	{ \
		(r).x = (a).x < (b).x ? (a).x : (b).x; \
		(r).y = (a).y < (b).y ? (a).y : (b).y; \
		(r).z = (a).z < (b).z ? (a).z : (b).z; \
	} while (0)

#define VEC3_MAX(r, a, b) \
	do \
	{ \
		(r).x = (a).x > (b).x ? (a).x : (b).x; \
		(r).y = (a).y > (b).y ? (a).y : (b).y; \
		(r).z = (a).z > (b).z ? (a).z : (b).z; \
	} while (0)

#define VEC3_DOT(a, b) \
	((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)

#define VEC3_NORM(a) \
	sqrt(VEC3_DOT(a, a))

#define VEC3_NORMALIZE(type, r, a) \
	do \
	{ \
		type v_norm = VEC3_NORM(a); \
		(r).x = (a).x / v_norm; \
		(r).y = (a).y / v_norm; \
		(r).z = (a).z / v_norm; \
	} while (0)

#define VEC3_CROSS(r, a, b) \
	do \
	{ \
		(r).x = (a).y * (b).z - (a).z * (b).y; \
		(r).y = (a).z * (b).x - (a).x * (b).z; \
		(r).z = (a).x * (b).y - (a).y * (b).x; \
	} while (0)

#define VEC3_NEGATE(r, v) \
	do \
	{ \
		(r).x = -(v).x; \
		(r).y = -(v).y; \
		(r).z = -(v).z; \
	} while (0)

#define VEC3_ADDV(r, a, v) VEC3_OPV(r, a, v, +)
#define VEC3_SUBV(r, a, v) VEC3_OPV(r, a, v, -)
#define VEC3_MULV(r, a, v) VEC3_OPV(r, a, v, *)
#define VEC3_DIVV(r, a, v) VEC3_OPV(r, a, v, /)
#define VEC3_MODV(r, a, v) VEC3_OPV(r, a, v, %)
#define VEC3_ANDV(r, a, v) VEC3_OPV(r, a, v, &)
#define VEC3_ORV (r, a, v) VEC3_OPV(r, a, v, |)
#define VEC3_XORV(r, a, v) VEC3_OPV(r, a, v, ^)

#define VEC3_ADD(r, a, b) VEC3_OP(r, a, b, +)
#define VEC3_SUB(r, a, b) VEC3_OP(r, a, b, -)
#define VEC3_MUL(r, a, b) VEC3_OP(r, a, b, *)
#define VEC3_DIV(r, a, b) VEC3_OP(r, a, b, /)
#define VEC3_MOD(r, a, b) VEC3_OP(r, a, b, %)
#define VEC3_AND(r, a, b) VEC3_OP(r, a, b, &)
#define VEC3_OR (r, a, b) VEC3_OP(r, a, b, |)
#define VEC3_XOR(r, a, b) VEC3_OP(r, a, b, ^)

#define VEC3_EQ(a, b) ((a).x == (b).x && (a).y == (b).y && (a).z == (b).z)

typedef VEC3_DEF(vec3f, float) vec3f_t;
typedef VEC3_DEF(vec3d, double) vec3d_t;
typedef VEC3_DEF(vec3i, int32_t) vec3i_t;
typedef VEC3_DEF(vec3u, uint32_t) vec3u_t;
typedef VEC3_DEF(vec3s, int16_t) vec3s_t;
typedef VEC3_DEF(vec3b, uint8_t) vec3b_t;

#endif
