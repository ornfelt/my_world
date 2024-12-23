#ifndef JKS_FRUSTUM_H
#define JKS_FRUSTUM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "array.h"
#include "mat4.h"
#include "vec3.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct aabb aabb_t;

enum frustum_result
{
	FRUSTUM_OUTSIDE,
	FRUSTUM_INSIDE,
	FRUSTUM_COLLIDE
};

typedef struct frustum
{
	jks_array_t planes; /* vec4f_t */
} frustum_t;

void frustum_init(frustum_t *frustum);
void frustum_destroy(frustum_t *frustum);
void frustum_clear(frustum_t *frustum);
bool frustum_copy(frustum_t *dst, frustum_t *src);
bool frustum_update(frustum_t *frustum, const mat4f_t *mat);
bool frustum_add_plane(frustum_t *frustum, vec3f_t eye, vec3f_t v1, vec3f_t v2);
void frustum_transform(frustum_t *frustum, const mat4f_t *mat);
enum frustum_result frustum_check(const frustum_t *frustum, const aabb_t *aabb);
bool frustum_check_fast(const frustum_t *frustum, const aabb_t *aabb);
bool frustum_check_sphere(const frustum_t *frustum, vec3f_t center, float radius);
bool frustum_check_point(const frustum_t *frustum, vec4f_t point);
bool frustum_check_points(const frustum_t *frustum, const vec4f_t *points, uint32_t points_nb);

#ifdef __cplusplus
}
#endif

#endif
