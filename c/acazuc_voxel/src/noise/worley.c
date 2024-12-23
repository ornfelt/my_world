#include "worley.h"

#include <jks/vec3.h>
#include <jks/vec2.h>

#include <math.h>

#define K 0.142857142857f
#define Ko 0.428571428571f
#define JITTER 1.f

static struct vec3f permute(struct vec3f x)
{
	struct vec3f a;
	VEC3_MULV(a, x, 34);
	VEC3_ADDV(a, a, 1);
	VEC3_MUL(a, a, x);
	VEC3_FNV(a, a, 10, fmod);
	return a;
}

static struct vec3f dist(struct vec3f x, struct vec3f y)
{
	VEC3_MUL(x, x, x);
	VEC3_MUL(y, y, y);
	struct vec3f ret;
	VEC3_ADD(ret, x, y);
	return ret;
}

static float fract(float f)
{
	return fmod(f, 1);
}

double worley_noise_get2(double x, double y)
{
	struct vec2f P;
	struct vec2f Pi;
	struct vec2f Pf;
	struct vec3f oi;
	struct vec3f of;
	struct vec3f px;
	struct vec3f p;
	struct vec3f ox;
	struct vec3f a;
	struct vec3f oy;
	struct vec3f dx;
	struct vec3f dy;
	struct vec3f d1;
	struct vec3f d2;
	struct vec3f d3;
	struct vec3f d1a;
	VEC2_SET(P, x, y);
	VEC2_FN1(Pi, P, floor);
	VEC2_FNV(Pi, Pi, 10, fmod);
	VEC2_FN1(Pf, P, fract);
	VEC3_SET(oi, -1, 0, 1);
	VEC3_SET(of, -.5, .5, 1.5);
	VEC3_ADDV(px, oi, Pi.x);
	px = permute(px);
	VEC3_ADDV(p, oi, px.x);
	VEC3_ADDV(p, p, Pi.y);
	p = permute(p);
	VEC3_MULV(ox, p, K);
	VEC3_FN1(ox, ox, fract);
	VEC3_SUBV(ox, ox, Ko);
	VEC3_MULV(a, p, K);
	VEC3_FN1(oy, a, floor);
	VEC3_FNV(oy, oy, 7, fmod);
	VEC3_MULV(oy, oy, K);
	VEC3_SUBV(oy, oy, Ko);
	VEC3_MULV(dx, ox, JITTER);
	VEC3_ADDV(dx, dx, .5);
	VEC3_ADDV(dx, dx, Pf.x);
	VEC3_MULV(dy, oy, JITTER);
	VEC3_ADDV(dy, dy, Pf.y);
	VEC3_SUB(dy, dy, of);
	d1 = dist(dx, dy);
	VEC3_ADDV(p, oi, Pi.y);
	VEC3_ADDV(p, p, px.y);
	p = permute(p);
	VEC3_MULV(ox, p, K);
	VEC3_FN1(ox, ox, fract);
	VEC3_SUBV(ox, ox, Ko);
	VEC3_MULV(a, p, K);
	VEC3_FN1(oy, a, floor);
	VEC3_FNV(oy, oy, 7, fmod);
	VEC3_MULV(oy, oy, K);
	VEC3_SUBV(oy, oy, Ko);
	VEC3_MULV(dx, ox, JITTER);
	VEC3_SUBV(dx, dx, .5);
	VEC3_ADDV(dx, dx, Pf.x);
	VEC3_MULV(dy, oy, JITTER);
	VEC3_SUB(dy, dy, of);
	VEC3_ADDV(dy, dy, Pf.y);
	d2 = dist(dx, dy);
	VEC3_ADDV(p, oi, px.z);
	VEC3_ADDV(p, p, Pi.y);
	p = permute(p);
	VEC3_MULV(ox, p, K);
	VEC3_FN1(ox, ox, fract);
	VEC3_SUBV(ox, ox, Ko);
	VEC3_MULV(a, p, K);
	VEC3_FN1(oy, a, floor);
	VEC3_FNV(oy, oy, 7, fmod);
	VEC3_MULV(oy, oy, K);
	VEC3_SUBV(oy, oy, Ko);
	VEC3_MULV(dx, ox, JITTER);
	VEC3_ADDV(dx, dx, Pf.x);
	VEC3_SUBV(dx, dx, 1.5);
	VEC3_MULV(dy, oy, JITTER);
	VEC3_ADDV(dy, dy, Pf.y);
	VEC3_SUB(dy, dy, of);
	d3 = dist(dx,dy);
	VEC3_MIN(d1a, d1, d2);
	VEC3_MAX(d2, d1, d2);
	VEC3_MIN(d2, d2, d3);
	VEC3_MIN(d1, d1a, d2);
	VEC3_MAX(d2, d1a, d2);
	if (d1.x >= d1.y)
		VEC3_SET(d1, d1.y, d1.x, d1.z);
	if (d1.x >= d1.z)
		VEC3_SET(d1, d1.z, d1.y, d1.x);
	struct vec2f tmp;
	struct vec2f d1yz = {d1.y, d1.z};
	struct vec2f d2yz = {d2.y, d2.z};
	VEC2_MIN(tmp, d1yz, d2yz);
	d1.y = tmp.x;
	d1.z = tmp.y;
	if (d1.z < d1.y)
		d1.y = d1.z;
	if (d2.x < d1.y)
		d1.y = d2.x;
	return sqrtf(d1.x);
}
