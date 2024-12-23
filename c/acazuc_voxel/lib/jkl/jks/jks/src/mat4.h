#ifndef JKS_MAT4_H
#define JKS_MAT4_H

#include "vec4.h"

#define MAT4_DEF(name, type) \
	struct name \
	{ \
		VEC4_DEF(, type) x; \
		VEC4_DEF(, type) y; \
		VEC4_DEF(, type) z; \
		VEC4_DEF(, type) w; \
	}

#define MAT4_MUL1(r, a, b, X, Y) (r).X.Y = (a).x.Y * (b).X.x + (a).y.Y * (b).X.y + (a).z.Y * (b).X.z + (a).w.Y * (b).X.w

#define MAT4_MUL4(r, a, b, X) \
	MAT4_MUL1(r, a, b, X, x); \
	MAT4_MUL1(r, a, b, X, y); \
	MAT4_MUL1(r, a, b, X, z); \
	MAT4_MUL1(r, a, b, X, w)

#define MAT4_MUL(r, a, b) \
	do \
	{ \
		MAT4_MUL4(r, a, b, x); \
		MAT4_MUL4(r, a, b, y); \
		MAT4_MUL4(r, a, b, z); \
		MAT4_MUL4(r, a, b, w); \
	} while (0)

#define MAT4_VEC4_MUL1(r, m, v, X) (r).X = (v).x * (m).x.X + (v).y * (m).y.X + (v).z * (m).z.X + (v).w * (m).w.X;

#define MAT4_VEC4_MUL(r, m, v) \
	do \
	{ \
		MAT4_VEC4_MUL1(r, m, v, x); \
		MAT4_VEC4_MUL1(r, m, v, y); \
		MAT4_VEC4_MUL1(r, m, v, z); \
		MAT4_VEC4_MUL1(r, m, v, w); \
	} while (0)

#define VEC4_MAT4_MUL1(r, m, v, X) (r).X = (v).x * (m).X.x + (v).y * (m).X.y + (v).z * (m).X.z + (v).w * (m).X.w;

#define VEC4_MAT4_MUL(r, m, v) \
	do \
	{ \
		VEC4_MAT4_MUL1(r, m, v, x); \
		VEC4_MAT4_MUL1(r, m, v, y); \
		VEC4_MAT4_MUL1(r, m, v, z); \
		VEC4_MAT4_MUL1(r, m, v, w); \
	} while (0)

#define MAT4_MULV(r, m, v) \
	do \
	{ \
		VEC4_MULV((r).x, (m).x, v); \
		VEC4_MULV((r).y, (m).y, v); \
		VEC4_MULV((r).z, (m).z, v); \
		VEC4_MULV((r).w, (m).w, v); \
	} while (0)

#define MAT4_ADD(r, a, b) \
	do \
	{ \
		VEC4_ADD((r).x, (a).x, (b).x); \
		VEC4_ADD((r).y, (a).y, (b).y); \
		VEC4_ADD((r).z, (a).z, (b).z); \
		VEC4_ADD((r).w, (a).w, (b).w); \
	} while (0)

#define MAT4_TRANSLATE1(r, m, v, X) (r).X = (v).x * (m).x.X + (v).y * (m).y.X + (v).z * (m).z.X + (m).w.X

#define MAT4_TRANSLATE(r, m, v) \
	do \
	{ \
		VEC4_CPY((r).x, (m).x); \
		VEC4_CPY((r).y, (m).y); \
		VEC4_CPY((r).z, (m).z); \
		MAT4_TRANSLATE1((r).w, m, v, x); \
		MAT4_TRANSLATE1((r).w, m, v, y); \
		MAT4_TRANSLATE1((r).w, m, v, z); \
		MAT4_TRANSLATE1((r).w, m, v, w); \
	} while (0)

#define MAT4_ROTATEX(t, r, m, v) \
	do \
	{ \
		t c = cos(v); \
		t s = sin(v); \
		VEC4_CPY((r).x, (m).x); \
		VEC4_MULV((r).y, (m).y, c); \
		VEC4_MULV((r).z, (m).z, s); \
		VEC4_ADD((r).y, (r).y, (r).z); \
		VEC4_MULV((r).z, (m).y, -s); \
		VEC4_MULV((r).w, (m).z, c); \
		VEC4_ADD((r).z, (r).z, (r).w); \
		VEC4_CPY((r).w, (m).w); \
	} while (0)

#define MAT4_ROTATEY(t, r, m, v) \
	do \
	{ \
		t c = cos(v); \
		t s = sin(v); \
		VEC4_MULV((r).x, (m).x, c); \
		VEC4_MULV((r).y, (m).z, -s); \
		VEC4_ADD((r).x, (r).x, (r).y); \
		VEC4_CPY((r).y, (m).y); \
		VEC4_MULV((r).z, (m).x, s); \
		VEC4_MULV((r).w, (m).z, c); \
		VEC4_ADD((r).z, (r).z, (r).w); \
		VEC4_CPY((r).w, (m).w); \
	} while (0)

#define MAT4_ROTATEZ(t, r, m, v) \
	do \
	{ \
		t c = cos(v); \
		t s = sin(v); \
		VEC4_MULV((r).x, (m).x, c); \
		VEC4_MULV((r).y, (m).y, s); \
		VEC4_ADD((r).x, (r).x, (r).y); \
		VEC4_MULV((r).y, (m).x, -s); \
		VEC4_MULV((r).z, (m).y, c); \
		VEC4_ADD((r).y, (r).y, (r).z); \
		VEC4_CPY((r).z, (m).z); \
		VEC4_CPY((r).w, (m).w); \
	} while (0)

#define MAT4_SCALE(r, m, v) \
	do \
	{ \
		VEC4_MULV((r).x, (m).x, (v).x); \
		VEC4_MULV((r).y, (m).y, (v).y); \
		VEC4_MULV((r).z, (m).z, (v).z); \
		VEC4_CPY((r).w, (m).w); \
	} while (0)

#define MAT4_SCALEV(r, m, v) \
	do \
	{ \
		VEC4_MULV((r).x, (m).x, v); \
		VEC4_MULV((r).y, (m).y, v); \
		VEC4_MULV((r).z, (m).z, v); \
		VEC4_CPY((r).w, (m).w); \
	} while (0)

#define MAT4_PERSPECTIVE(m, fov, aspect, znear, zfar) \
	do \
	{ \
		VEC4_SET((m).y, 0, 1 / tan((fov) / 2.), 0, 0); \
		VEC4_SET((m).x, (m).y.y / (aspect), 0, 0, 0); \
		VEC4_SET((m).z, 0, 0, ((zfar) + (znear)) / ((znear) - (zfar)), -1); \
		VEC4_SET((m).w, 0, 0, (2. * (zfar) * (znear)) / ((znear) - (zfar)), 0); \
	} while (0)

#define MAT4_ORTHO(t, m, left, right, bottom, top, near, far) \
	do \
	{ \
		t rml = (right) - (left); \
		t tmb = (top) - (bottom); \
		t fmn = (far) - (near); \
		VEC4_SET((m).x, 2 / rml, 0, 0, 0); \
		VEC4_SET((m).y, 0, 2 / tmb, 0, 0); \
		VEC4_SET((m).z, 0, 0, -2 / fmn, 0); \
		VEC4_SET((m).w, -((right) + (left)) / rml, -((top) + (bottom)) / tmb, -((far) + (near)) / fmn, 1); \
	} while (0)

#define MAT4_LOOKAT(t, m, eye, center, up) \
	do \
	{ \
		MAT4_DEF(, t) tmp; \
		VEC3_DEF(, t) up_n; \
		VEC3_DEF(, t) f; \
		VEC3_DEF(, t) f_n; \
		VEC3_DEF(, t) s; \
		VEC3_DEF(, t) u; \
		VEC3_DEF(, t) s_n; \
		VEC3_DEF(, t) neg_eye; \
		VEC3_NORMALIZE(t, up_n, up); \
		VEC3_SUB(f, center, eye); \
		VEC3_NORMALIZE(t, f_n, f); \
		VEC3_CROSS(s, f_n, up_n); \
		VEC3_NORMALIZE(t, s_n, s); \
		VEC3_CROSS(u, s_n, f_n); \
		VEC3_NEGATE(f_n, f_n); \
		VEC4_SET(tmp.x, s.x, u.x, f_n.x, 0); \
		VEC4_SET(tmp.y, s.y, u.y, f_n.y, 0); \
		VEC4_SET(tmp.z, s.z, u.z, f_n.z, 0); \
		VEC4_SET(tmp.w, 0, 0, 0, 1); \
		VEC3_NEGATE(neg_eye, eye); \
		MAT4_TRANSLATE(m, tmp, neg_eye); \
	} while (0)

#define MAT4_IDENTITY(m) \
	do \
	{ \
		VEC4_SET((m).x, 1, 0, 0, 0); \
		VEC4_SET((m).y, 0, 1, 0, 0); \
		VEC4_SET((m).z, 0, 0, 1, 0); \
		VEC4_SET((m).w, 0, 0, 0, 1); \
	} while (0)

#define MAT4_INVERSE(t, r, m) \
	do \
	{ \
		t v2323 = (m).z.z * (m).w.w - (m).w.z * (m).z.w; \
		t v1323 = (m).y.z * (m).w.w - (m).w.z * (m).y.w; \
		t v1223 = (m).y.z * (m).z.w - (m).z.z * (m).y.w; \
		t v0323 = (m).x.z * (m).w.w - (m).w.z * (m).x.w; \
		t v0223 = (m).x.z * (m).z.w - (m).z.z * (m).x.w; \
		t v0123 = (m).x.z * (m).y.w - (m).y.z * (m).x.w; \
		t v2313 = (m).z.y * (m).w.w - (m).w.y * (m).z.w; \
		t v1313 = (m).y.y * (m).w.w - (m).w.y * (m).y.w; \
		t v1213 = (m).y.y * (m).z.w - (m).z.y * (m).y.w; \
		t v2312 = (m).z.y * (m).w.z - (m).w.y * (m).z.z; \
		t v1312 = (m).y.y * (m).w.z - (m).w.y * (m).y.z; \
		t v1212 = (m).y.y * (m).z.z - (m).z.y * (m).y.z; \
		t v0313 = (m).x.y * (m).w.w - (m).w.y * (m).x.w; \
		t v0213 = (m).x.y * (m).z.w - (m).z.y * (m).x.w; \
		t v0312 = (m).x.y * (m).w.z - (m).w.y * (m).x.z; \
		t v0212 = (m).x.y * (m).z.z - (m).z.y * (m).x.z; \
		t v0113 = (m).x.y * (m).y.w - (m).y.y * (m).x.w; \
		t v0112 = (m).x.y * (m).y.z - (m).y.y * (m).x.z; \
		t inv_deter = 1. / ( \
			+(m).x.x * ((m).y.y * v2323 - (m).z.y * v1323 + (m).w.y * v1223) \
			-(m).y.x * ((m).x.y * v2323 - (m).z.y * v0323 + (m).w.y * v0223) \
			+(m).z.x * ((m).x.y * v1323 - (m).y.y * v0323 + (m).w.y * v0123) \
			-(m).w.x * ((m).x.y * v1223 - (m).y.y * v0223 + (m).z.y * v0123)); \
		VEC4_SET((r).x, \
			+((m).y.y * v2323 - (m).z.y * v1323 + (m).w.y * v1223) * inv_deter, \
			-((m).x.y * v2323 - (m).z.y * v0323 + (m).w.y * v0223) * inv_deter, \
			+((m).x.y * v1323 - (m).y.y * v0323 + (m).w.y * v0123) * inv_deter, \
			-((m).x.y * v1223 - (m).y.y * v0223 + (m).z.y * v0123) * inv_deter); \
		VEC4_SET((r).y, \
			-((m).y.x * v2323 - (m).z.x * v1323 + (m).w.x * v1223) * inv_deter, \
			+((m).x.x * v2323 - (m).z.x * v0323 + (m).w.x * v0223) * inv_deter, \
			-((m).x.x * v1323 - (m).y.x * v0323 + (m).w.x * v0123) * inv_deter, \
			+((m).x.x * v1223 - (m).y.x * v0223 + (m).z.x * v0123) * inv_deter); \
		VEC4_SET((r).z, \
			+((m).y.x * v2313 - (m).z.x * v1313 + (m).w.x * v1213) * inv_deter, \
			-((m).x.x * v2313 - (m).z.x * v0313 + (m).w.x * v0213) * inv_deter, \
			+((m).x.x * v1313 - (m).y.x * v0313 + (m).w.x * v0113) * inv_deter, \
			-((m).x.x * v1213 - (m).y.x * v0213 + (m).z.x * v0113) * inv_deter); \
		VEC4_SET((r).w, \
			-((m).y.x * v2312 - (m).z.x * v1312 + (m).w.x * v1212) * inv_deter, \
			+((m).x.x * v2312 - (m).z.x * v0312 + (m).w.x * v0212) * inv_deter, \
			-((m).x.x * v1312 - (m).y.x * v0312 + (m).w.x * v0112) * inv_deter, \
			+((m).x.x * v1212 - (m).y.x * v0212 + (m).z.x * v0112) * inv_deter); \
	} while (0)

typedef MAT4_DEF(mat4f, float) mat4f_t;

#endif
