#include "aabb.h"
#include "mat3.h"
#include <math.h>

void aabb_set_min_max(aabb_t *aabb, const aabb_t *min_max)
{
	VEC3_MIN(aabb->p0, min_max->p0, min_max->p1);
	VEC3_MAX(aabb->p1, min_max->p0, min_max->p1);
}

void aabb_add_min_max(aabb_t *aabb, const aabb_t *min_max)
{
	vec3f_t tmp;
	VEC3_MIN(tmp, min_max->p0, min_max->p1);
	VEC3_MIN(aabb->p0, aabb->p0, tmp);
	VEC3_MAX(tmp, min_max->p0, min_max->p1);
	VEC3_MAX(aabb->p1, aabb->p1, tmp);
}

void aabb_sub_min_max(aabb_t *aabb, const aabb_t *min_max)
{
	vec3f_t tmp;
	VEC3_MIN(tmp, min_max->p0, min_max->p1);
	VEC3_MAX(aabb->p0, aabb->p0, tmp);
	VEC3_MAX(tmp, min_max->p0, min_max->p1);
	VEC3_MIN(aabb->p1, aabb->p1, tmp);
}

void aabb_transform(aabb_t *aabb, const mat4f_t *mat)
{
	vec4f_t center;
	VEC3_ADD(center, aabb->p0, aabb->p1);
	VEC3_MULV(center, center, .5f);
	center.w = 1;
	vec3f_t extents;
	VEC3_SUB(extents, aabb->p1, center);
	vec4f_t new_center;
	MAT4_VEC4_MUL(new_center, *mat, center);
	mat3f_t abs_mat;
	VEC3_FN1(abs_mat.x, mat->x, fabs);
	VEC3_FN1(abs_mat.y, mat->y, fabs);
	VEC3_FN1(abs_mat.z, mat->z, fabs);
	vec3f_t new_extents;
	MAT3_VEC3_MUL(new_extents, abs_mat, extents);
	struct vec3f p0;
	struct vec3f p1;
	VEC3_SUB(p0, new_center, new_extents);
	VEC3_ADD(p1, new_center, new_extents);
	VEC3_MIN(aabb->p0, p0, p1);
	VEC3_MAX(aabb->p1, p0, p1);
}

void aabb_move(aabb_t *aabb, vec3f_t dst)
{
	VEC3_ADD(aabb->p0, aabb->p0, dst);
	VEC3_ADD(aabb->p1, aabb->p1, dst);
}

bool aabb_contains(const aabb_t *aabb, vec3f_t point)
{
	if (point.x < aabb->p0.x)
		return false;
	if (point.x > aabb->p1.x)
		return false;
	if (point.y < aabb->p0.y)
		return false;
	if (point.y > aabb->p1.y)
		return false;
	if (point.z < aabb->p0.z)
		return false;
	if (point.z > aabb->p1.z)
		return false;
	return true;
}

bool aabb_intersect(const aabb_t *aabb, vec3f_t pos, vec3f_t dir, float *t)
{
	float t1 = (aabb->p0.x - pos.x) / dir.x;
	float t2 = (aabb->p1.x - pos.x) / dir.x;
	float t3 = (aabb->p0.y - pos.y) / dir.y;
	float t4 = (aabb->p1.y - pos.y) / dir.y;
	float t5 = (aabb->p0.z - pos.z) / dir.z;
	float t6 = (aabb->p1.z - pos.z) / dir.z;
	float tmin = t1;
	tmin = tmin < t2 ? tmin : t2;
	tmin = tmin < t3 ? tmin : t3;
	tmin = tmin < t4 ? tmin : t4;
	tmin = tmin < t5 ? tmin : t5;
	tmin = tmin < t6 ? tmin : t6;
	float tmax = t1;
	tmax = tmax > t2 ? tmax : t2;
	tmax = tmax > t3 ? tmax : t3;
	tmax = tmax > t4 ? tmax : t4;
	tmax = tmax > t5 ? tmax : t5;
	tmax = tmax > t6 ? tmax : t6;
	if (tmax < 0)
		return false;
	if (tmin > tmax)
		return false;
	*t = tmin;
	return true;
}

static float aabb_point_dist(float p, float min, float max)
{
	float ret = 0;
	if (p < min)
	{
		float tmp = min - p;
		ret += tmp * tmp;
	}
	if (p > max)
	{
		float tmp = p - max;
		ret += tmp * tmp;
	}
	return ret;
}

bool aabb_intersect_sphere(const aabb_t *aabb, vec3f_t center, float radius)
{
	float d = aabb_point_dist(center.x, aabb->p0.x, aabb->p1.x)
	        + aabb_point_dist(center.y, aabb->p0.y, aabb->p1.y)
	        + aabb_point_dist(center.z, aabb->p0.z, aabb->p1.z);
	return d <= radius * radius;
}

bool aabb_intersect_aabb(const aabb_t *aabb1, const aabb_t *aabb2)
{
	return  aabb1->p0.x <= aabb2->p1.x
	     && aabb1->p1.x >= aabb2->p0.x
	     && aabb1->p0.y <= aabb2->p1.y
	     && aabb1->p1.y >= aabb2->p0.y
	     && aabb1->p0.z <= aabb2->p1.z
	     && aabb1->p1.z >= aabb2->p0.z;
}

float aabb_collide_x(const aabb_t *aabb1, const aabb_t *aabb2, float x)
{
	if (aabb1->p1.y <= aabb2->p0.y || aabb1->p0.y >= aabb2->p1.y)
		return x;
	if (aabb1->p1.z <= aabb2->p0.z || aabb1->p0.z >= aabb2->p1.z)
		return x;
	if (x > 0 && aabb1->p0.x >= aabb2->p1.x)
	{
		float max = aabb1->p0.x - aabb2->p1.x;
		if (max < x)
			return max;
	}
	else if (x < 0 && aabb1->p1.x <= aabb2->p0.x)
	{
		float max = aabb1->p1.x - aabb2->p0.x;
		if (max > x)
			return max;
	}
	return x;
}

float aabb_collide_y(const aabb_t *aabb1, const aabb_t *aabb2, float y)
{
	if (aabb1->p1.x <= aabb2->p0.x || aabb1->p0.x >= aabb2->p1.x)
		return y;
	if (aabb1->p1.z <= aabb2->p0.z || aabb1->p0.z >= aabb2->p1.z)
		return y;
	if (y > 0 && aabb1->p0.y >= aabb2->p1.y)
	{
		float max = aabb1->p0.y - aabb2->p1.y;
		if (max < y)
			return max;
	}
	else if (y < 0 && aabb1->p1.y <= aabb2->p0.y)
	{
		float max = aabb1->p1.y - aabb2->p0.y;
		if (max > y)
			return max;
	}
	return y;
}

float aabb_collide_z(const aabb_t *aabb1, const aabb_t *aabb2, float z)
{
	if (aabb1->p1.x <= aabb2->p0.x || aabb1->p0.x >= aabb2->p1.x)
		return z;
	if (aabb1->p1.y <= aabb2->p0.y || aabb1->p0.y >= aabb2->p1.y)
		return z;
	if (z > 0 && aabb1->p0.z >= aabb2->p1.z)
	{
		float max = aabb1->p0.z - aabb2->p1.z;
		if (max < z)
			return max;
	}
	else if (z < 0 && aabb1->p1.z <= aabb2->p0.z)
	{
		float max = aabb1->p1.z - aabb2->p0.z;
		if (max > z)
			return max;
	}
	return z;
}

aabb_t aabb_expand(const aabb_t *aabb, vec3f_t size)
{
	aabb_t ret;
	ret.p0 = aabb->p0;
	ret.p1 = aabb->p1;
	if (size.x < 0)
		ret.p0.x += size.x;
	else if (size.x > 0)
		ret.p1.x += size.x;
	if (size.y < 0)
		ret.p0.y += size.y;
	else if (size.y > 0)
		ret.p1.y += size.y;
	if (size.z < 0)
		ret.p0.z += size.z;
	else if (size.z > 0)
		ret.p1.z += size.z;
	return ret;
}
