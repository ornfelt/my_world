#include "frustum.h"
#include "aabb.h"
#include <string.h>
#include <math.h>

void frustum_init(frustum_t *frustum)
{
	jks_array_init(&frustum->planes, sizeof(vec4f_t), NULL, NULL);
}

void frustum_destroy(frustum_t *frustum)
{
	jks_array_destroy(&frustum->planes);
}

bool frustum_copy(frustum_t *dst, frustum_t *src)
{
	if (!jks_array_resize(&dst->planes, src->planes.size))
		return false;
	memcpy(dst->planes.data, src->planes.data, src->planes.size * sizeof(vec4f_t));
	return true;
}

bool frustum_update(frustum_t *frustum, const mat4f_t *mat)
{
	if (!jks_array_resize(&frustum->planes, 6))
		return false;
	vec4f_t *planes = (vec4f_t*)jks_array_get(&frustum->planes, 0);
	VEC4_SET(planes[0], mat->x.w - mat->x.x, mat->y.w - mat->y.x, mat->z.w - mat->z.x, mat->w.w - mat->w.x);
	VEC4_SET(planes[1], mat->x.w + mat->x.x, mat->y.w + mat->y.x, mat->z.w + mat->z.x, mat->w.w + mat->w.x);
	VEC4_SET(planes[2], mat->x.w - mat->x.z, mat->y.w - mat->y.z, mat->z.w - mat->z.z, mat->w.w - mat->w.z);
	VEC4_SET(planes[3], mat->x.w + mat->x.z, mat->y.w + mat->y.z, mat->z.w + mat->z.z, mat->w.w + mat->w.z);
	VEC4_SET(planes[4], mat->x.w - mat->x.y, mat->y.w - mat->y.y, mat->z.w - mat->z.y, mat->w.w - mat->w.y);
	VEC4_SET(planes[5], mat->x.w + mat->x.y, mat->y.w + mat->y.y, mat->z.w + mat->z.y, mat->w.w + mat->w.y);
	return true;
}

bool frustum_add_plane(frustum_t *frustum, vec3f_t eye, vec3f_t v1, vec3f_t v2)
{
	vec4f_t *plane = (vec4f_t*)jks_array_grow(&frustum->planes, 1);
	if (!plane)
		return false;
	vec3f_t tmp1;
	vec3f_t tmp2;
	VEC3_SUB(tmp1, v1, eye);
	VEC3_SUB(tmp2, v2, eye);
	VEC3_CROSS(*plane, tmp1, tmp2);
	VEC3_NORMALIZE(float, *plane, *plane);
	plane->w = -VEC3_DOT(*plane, eye);
	return true;
}

void frustum_transform(frustum_t *frustum, const mat4f_t *mat)
{
	vec4f_t *planes = (vec4f_t*)jks_array_get(&frustum->planes, 0);
	for (uint32_t i = 0; i < frustum->planes.size; ++i)
	{
		vec4f_t r;
		MAT4_VEC4_MUL(r, *mat, planes[i]);
		planes[i] = r;
	}
}

void frustum_clear(frustum_t *frustum)
{
	jks_array_resize(&frustum->planes, 0);
}

enum frustum_result frustum_check(const frustum_t *frustum, const aabb_t *aabb)
{
	vec4f_t points[8] =
	{
		{aabb->p0.x, aabb->p0.y, aabb->p0.z, 1},
		{aabb->p1.x, aabb->p0.y, aabb->p0.z, 1},
		{aabb->p0.x, aabb->p1.y, aabb->p0.z, 1},
		{aabb->p1.x, aabb->p1.y, aabb->p0.z, 1},
		{aabb->p0.x, aabb->p0.y, aabb->p1.z, 1},
		{aabb->p1.x, aabb->p0.y, aabb->p1.z, 1},
		{aabb->p0.x, aabb->p1.y, aabb->p1.z, 1},
		{aabb->p1.x, aabb->p1.y, aabb->p1.z, 1}
	};
	enum frustum_result result = FRUSTUM_INSIDE;
	vec4f_t *planes = (vec4f_t*)jks_array_get(&frustum->planes, 0);
	for (uint32_t i = 0; i < frustum->planes.size; ++i)
	{
		vec4f_t plane = planes[i];
		uint8_t outside_points = 0;
		if (VEC4_DOT(plane, points[0]) < 0)
			outside_points++;
		if (VEC4_DOT(plane, points[1]) < 0)
			outside_points++;
		if (VEC4_DOT(plane, points[2]) < 0)
			outside_points++;
		if (VEC4_DOT(plane, points[3]) < 0)
			outside_points++;
		if (VEC4_DOT(plane, points[4]) < 0)
			outside_points++;
		if (VEC4_DOT(plane, points[5]) < 0)
			outside_points++;
		if (VEC4_DOT(plane, points[6]) < 0)
			outside_points++;
		if (VEC4_DOT(plane, points[7]) < 0)
			outside_points++;
		if (outside_points == 8)
			return FRUSTUM_OUTSIDE;
		if (outside_points)
			result = FRUSTUM_COLLIDE;
	}
	return result;
}

bool frustum_check_fast(const frustum_t *frustum, const aabb_t *aabb)
{
	vec4f_t points[8] =
	{
		{aabb->p0.x, aabb->p0.y, aabb->p0.z, 1},
		{aabb->p1.x, aabb->p0.y, aabb->p0.z, 1},
		{aabb->p0.x, aabb->p1.y, aabb->p0.z, 1},
		{aabb->p1.x, aabb->p1.y, aabb->p0.z, 1},
		{aabb->p0.x, aabb->p0.y, aabb->p1.z, 1},
		{aabb->p1.x, aabb->p0.y, aabb->p1.z, 1},
		{aabb->p0.x, aabb->p1.y, aabb->p1.z, 1},
		{aabb->p1.x, aabb->p1.y, aabb->p1.z, 1}
	};
	vec4f_t *planes = (vec4f_t*)jks_array_get(&frustum->planes, 0);
	for (uint32_t i = 0; i < frustum->planes.size; ++i)
	{
		vec4f_t plane = planes[i];
		if (VEC4_DOT(plane, points[0]) < 0
		 && VEC4_DOT(plane, points[1]) < 0
		 && VEC4_DOT(plane, points[2]) < 0
		 && VEC4_DOT(plane, points[3]) < 0
		 && VEC4_DOT(plane, points[4]) < 0
		 && VEC4_DOT(plane, points[5]) < 0
		 && VEC4_DOT(plane, points[6]) < 0
		 && VEC4_DOT(plane, points[7]) < 0)
			return false;
	}
	return true;
}

bool frustum_check_sphere(const frustum_t *frustum, vec3f_t center, float radius)
{
	vec4f_t point = {center.x, center.y, center.z, 1};
	vec4f_t *planes = (vec4f_t*)jks_array_get(&frustum->planes, 0);
	for (uint32_t i = 0; i < frustum->planes.size; ++i)
	{
		vec4f_t plane = planes[i];
		if (VEC4_DOT(plane, point) < -radius)
			return false;
	}
	return true;
}

bool frustum_check_point(const frustum_t *frustum, vec4f_t point)
{
	vec4f_t *planes = (vec4f_t*)jks_array_get(&frustum->planes, 0);
	for (uint32_t i = 0; i < frustum->planes.size; ++i)
	{
		vec4f_t plane = planes[i];
		if (VEC4_DOT(plane, point) < 0)
			return false;
	}
	return true;
}

bool frustum_check_points(const frustum_t *frustum, const vec4f_t *points, uint32_t points_nb)
{
	vec4f_t *planes = (vec4f_t*)jks_array_get(&frustum->planes, 0);
	for (uint32_t i = 0; i < frustum->planes.size; ++i)
	{
		vec4f_t plane = planes[i];
		for (uint32_t j = 0; j < points_nb; ++j)
		{
			if (VEC4_DOT(plane, points[j]) >= 0)
				goto next_plane;
		}
		return false;
next_plane:
		continue;
	}
	return true;
}
