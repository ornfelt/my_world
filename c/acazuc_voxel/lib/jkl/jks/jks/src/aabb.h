#ifndef JKS_AABB_H
#define JKS_AABB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mat4.h"
#include "vec3.h"

#include <stdbool.h>

typedef struct aabb
{
	vec3f_t p0;
	vec3f_t p1;
} aabb_t;

void aabb_set_min_max(aabb_t *aabb, const aabb_t *min_max);
void aabb_add_min_max(aabb_t *aabb, const aabb_t *min_max);
void aabb_sub_min_max(aabb_t *aabb, const aabb_t *min_max);
void aabb_transform(aabb_t *aabb, const mat4f_t *mat);
void aabb_move(aabb_t *aabb, vec3f_t dst);
bool aabb_contains(const aabb_t *aabb, vec3f_t point);
bool aabb_intersect(const aabb_t *aabb, vec3f_t pos, vec3f_t dir, float *t);
bool aabb_intersect_sphere(const aabb_t *aabb, vec3f_t center, float radius);
bool aabb_intersect_aabb(const aabb_t *aabb1, const aabb_t *aabb2);
float aabb_collide_x(const aabb_t *aabb1, const aabb_t *aabb2, float x);
float aabb_collide_y(const aabb_t *aabb1, const aabb_t *aabb2, float y);
float aabb_collide_z(const aabb_t *aabb1, const aabb_t *aabb2, float z);
aabb_t aabb_expand(const aabb_t *aabb, vec3f_t size);

#ifdef __cplusplus
}
#endif

#endif
