#include "camera.h"

#include "net/opcode.h"

#include "obj/unit.h"

#ifdef WITH_DEBUG_RENDERING
# include "gx/collisions.h"
# include "gx/frame.h"
#endif
#include "gx/m2.h"

#include "map/map.h"

#include "phys/physics.h"

#include "performance.h"
#include "memory.h"
#include "const.h"
#include "wow.h"
#include "log.h"

#include <jks/mat3.h>

#include <gfx/window.h>

#include <math.h>

#define WDL_VIEW_ZFAR  2.0f
#define WDL_VIEW_ZNEAR 0.5f

#define MOVE_SPEED 10

#define SHADOW_WIDTH 300.0f

#define FLYING(camera) (((camera)->worldobj->movement_data.flags & MOVEFLAG_FLYING))
#define GROUNDED(camera) (!FLYING(camera) && !((camera)->worldobj->movement_data.flags & MOVEFLAG_FALLING))

struct camera *
camera_new(void)
{
	struct camera *camera;

	camera = mem_malloc(MEM_GENERIC, sizeof(*camera));
	if (!camera)
		return NULL;
	camera->worldobj = NULL;
	MAT4_IDENTITY(camera->shadow_vp);
	MAT4_IDENTITY(camera->shadow_p);
	MAT4_IDENTITY(camera->shadow_v);
	MAT4_IDENTITY(camera->wdl_vp);
	MAT4_IDENTITY(camera->wdl_p);
	MAT4_IDENTITY(camera->vp);
	MAT4_IDENTITY(camera->p);
	MAT4_IDENTITY(camera->v);
	VEC3_SETV(camera->pos, 0);
	VEC3_SETV(camera->rot, 0);
	camera->mouse_x = 0;
	camera->mouse_y = 0;
	camera->unit_max_distance = 30;
	camera->unit_distance = 100;
	camera->unit_velocity = 0;
	camera->view_distance = CHUNK_WIDTH * 32;
	camera->fov = 60.0f / 180.0f * M_PI;
	camera->move_unit = false;
	frustum_init(&camera->shadow_frustum);
	frustum_init(&camera->wdl_frustum);
	frustum_init(&camera->frustum);
	camera->last_move_flags = 0;
	camera->autorun = false;
	return camera;
}

void
camera_delete(struct camera *camera)
{
	if (!camera)
		return;
	frustum_destroy(&camera->wdl_frustum);
	frustum_destroy(&camera->frustum);
	mem_free(MEM_GENERIC, camera);
}

void
camera_handle_mouse(struct camera *camera)
{
	int32_t mouse_x = gfx_get_mouse_x(g_wow->window);
	int32_t mouse_y = gfx_get_mouse_y(g_wow->window);
	int32_t diff_x = mouse_x - camera->mouse_x;
	int32_t diff_y = mouse_y - camera->mouse_y;
	camera->mouse_x = mouse_x;
	camera->mouse_y = mouse_y;
	if (g_wow->wow_opt & WOW_OPT_FOCUS_3D)
	{
		if (diff_x || diff_y)
		{
			if (camera->worldobj && camera->move_unit)
			{
				worldobj_set_orientation(camera->worldobj, camera->worldobj->orientation - diff_x / 10000.0f * M_PI);
				if (FLYING(camera))
					worldobj_set_slope(camera->worldobj, camera->worldobj->slope - diff_y / 10000.0f * M_PI);
				else
					worldobj_set_slope(camera->worldobj, 0);
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_SET_FACING);
			}
			else
			{
				camera->rot.x += diff_y / 10000.0f * M_PI;
				camera->rot.y += diff_x / 10000.0f * M_PI;
				if (camera->rot.x > M_PI / 2.0f)
					camera->rot.x = M_PI / 2.0f;
				if (camera->rot.x < -M_PI / 2.0f)
					camera->rot.x = -M_PI / 2.0f;
			}
		}
	}
	if (camera->worldobj)
	{
		struct vec3f base = camera->worldobj->position;
		base.y += 1.6;
		if (camera->unit_distance != camera->unit_max_distance)
		{
			float dt = (g_wow->frametime - g_wow->lastframetime) / 1000000000.0f;
			camera->unit_velocity += GRAVITY * dt;
			camera->unit_distance += dt * (camera->unit_velocity - dt * GRAVITY / 2);
			if (camera->unit_distance > camera->unit_max_distance)
			{
				camera->unit_distance = camera->unit_max_distance;
				camera->unit_velocity = 0;
			}
		}
		if (camera->unit_distance < 1)
		{
			camera->pos = base;
			return;
		}
		struct vec3f dir_base = {0, 0, camera->unit_distance};
		if (camera->move_unit)
		{
			camera->rot.x = -camera->worldobj->slope;
			camera->rot.y = -camera->worldobj->orientation;
		}
		struct mat3f tmp1;
		struct mat3f tmp2;
		MAT3_IDENTITY(tmp1);
		MAT3_ROTATEZ(float, tmp2, tmp1, -camera->rot.z);
		MAT3_ROTATEY(float, tmp1, tmp2, -camera->rot.y);
		MAT3_ROTATEX(float, tmp2, tmp1, -camera->rot.x);
		struct vec3f dst;
		struct vec3f dir;
		MAT3_VEC3_MUL(dir, tmp2, dir_base);
		VEC3_ADD(dst, dir, base);
		{
			PERFORMANCE_BEGIN(COLLISIONS);
			struct collision_params params;
			params.wmo_cam = true;
			VEC3_MIN(params.aabb.p0, dst, base);
			VEC3_MAX(params.aabb.p1, dst, base);
			params.aabb.p0.x -= SPHERE_RADIUS;
			params.aabb.p0.y -= SPHERE_RADIUS;
			params.aabb.p0.z -= SPHERE_RADIUS;
			params.aabb.p1.x += SPHERE_RADIUS;
			params.aabb.p1.y += SPHERE_RADIUS;
			params.aabb.p1.z += SPHERE_RADIUS;
			struct vec3f delta;
			VEC3_SUB(delta, params.aabb.p1, params.aabb.p0);
			params.radius = VEC3_NORM(delta);
			VEC3_MULV(params.center, delta, 0.5f);
			VEC3_ADD(params.center, params.center, params.aabb.p0);
			struct jks_array triangles;
			jks_array_init(&triangles, sizeof(struct collision_triangle), NULL, NULL);
			map_collect_collision_triangles(g_wow->map, &params, &triangles);
			float t;
			float dir_len = VEC3_NORM(dir);
			VEC3_DIVV(dir, dir, dir_len);
			if (find_nearest_triangle(&triangles, base, dir, dir_len, NULL, NULL, &t) && t >= 0)
			{
				camera->unit_distance = t;
				if (t > 1)
					t -= 1;
				else
					t = 0;
				VEC3_MULV(dir, dir, t);
				VEC3_ADD(dst, dir, base);
				camera->unit_velocity = 0;
			}
			else
			{
				VEC3_SUB(dst, dst, dir);
			}
			jks_array_destroy(&triangles);
			PERFORMANCE_END(COLLISIONS);
		}
		camera->pos = dst;
	}
}

bool
camera_update_matrixes(struct camera *camera)
{
	float ratio = (float)g_wow->render_width / g_wow->render_height;
	MAT4_PERSPECTIVE(camera->wdl_p, camera->fov, ratio, camera->view_distance * WDL_VIEW_ZNEAR, camera->view_distance * WDL_VIEW_ZFAR);
	MAT4_PERSPECTIVE(camera->p, camera->fov, ratio, 0.5f, camera->view_distance);
	struct mat4f tmp1;
	struct mat4f tmp2;
	MAT4_IDENTITY(tmp1);
	MAT4_ROTATEX(float, tmp2, tmp1, camera->rot.x);
	MAT4_ROTATEY(float, tmp1, tmp2, camera->rot.y);
	MAT4_ROTATEZ(float, tmp2, tmp1, camera->rot.z);
	{
		struct vec3f tmp;
		VEC3_NEGATE(tmp, camera->pos);
		MAT4_TRANSLATE(camera->v, tmp1, tmp);
	}
	MAT4_MUL(camera->vp, camera->p, camera->v);
	MAT4_MUL(camera->wdl_vp, camera->wdl_p, camera->v);
	MAT4_ORTHO(float, camera->shadow_p, -SHADOW_WIDTH / 2.0f, SHADOW_WIDTH / 2.0f, -SHADOW_WIDTH / 2.0f, SHADOW_WIDTH / 2.0f, 1.0f, 3000.0f);
	struct vec3f shadow_center = camera->pos;
	struct vec3f shadow_eye = {1000.0f, 1500.0f, -1000.0f};
	VEC3_ADD(shadow_eye, shadow_eye, shadow_center);
	struct vec3f shadow_up = {0.0f, 1.0f, 0.0f};
	MAT4_LOOKAT(float, camera->shadow_v, shadow_eye, shadow_center, shadow_up);
	MAT4_MUL(camera->shadow_vp, camera->shadow_p, camera->shadow_v);
	if (!frustum_update(&camera->frustum, &camera->vp))
	{
		LOG_ERROR("failed to update frustum");
		return false;
	}
	if (!frustum_update(&camera->wdl_frustum, &camera->wdl_vp))
	{
		LOG_ERROR("failed to update wdl frustum");
		return false;
	}
	if (!frustum_update(&camera->shadow_frustum, &camera->shadow_vp))
	{
		LOG_ERROR("failed to update shadow frustum");
		return false;
	}
	return true;
}

#if 1
#else
static struct vec3f
edge_collision(struct vec3f a, struct vec3f b, struct vec3f p)
{
	struct vec3f d[2];
	VEC3_SUB(d[0], b, a);
	VEC3_SUB(d[1], p, a);
	float t = VEC3_DOT(d[0], d[1]);
	if (t < 0)
		t = 0;
	if (t > 1)
		t = 1;
	struct vec3f ret;
	VEC3_MULV(ret, d[0], t);
	VEC3_ADD(ret, ret, a);
	return ret;
}

static bool
get_closest_triangle_point(struct vec3f p,
                           struct collision_triangle *triangle,
                           float *distp,
                           struct vec3f *intersectionp,
                           struct vec3f *normp)
{
	struct vec3f e0;
	struct vec3f e1;
	struct vec3f norm;
	struct vec3f delta[3];
	VEC3_SUB(e0, triangle->points[1], triangle->points[0]);
	VEC3_SUB(e1, triangle->points[2], triangle->points[0]);
	VEC3_CROSS(norm, e0, e1);
	VEC3_NORMALIZE(float, norm, norm);
	VEC3_SUB(delta[0], p, triangle->points[0]);
	float dist = VEC3_DOT(delta[0], norm);
	if (dist < -SPHERE_RADIUS || dist > SPHERE_RADIUS)
		return false;
	struct vec3f projected;
	VEC3_MULV(projected, norm, dist);
	VEC3_SUB(projected, p, projected);
	struct vec3f edges[3];
	struct vec3f crossed[3];
	VEC3_SUB(edges[0], triangle->points[1], triangle->points[0]);
	VEC3_SUB(edges[1], triangle->points[2], triangle->points[1]);
	VEC3_SUB(edges[2], triangle->points[0], triangle->points[2]);
	VEC3_SUB(delta[1], projected, triangle->points[1]);
	VEC3_SUB(delta[2], projected, triangle->points[2]);
	VEC3_CROSS(crossed[0], delta[0], edges[0]);
	VEC3_CROSS(crossed[1], delta[1], edges[1]);
	VEC3_CROSS(crossed[2], delta[2], edges[2]);
	bool inside = VEC3_DOT(crossed[0], norm) <= EPSILON && VEC3_DOT(crossed[1], norm) <= EPSILON && VEC3_DOT(crossed[2], norm) <= EPSILON;
	if (inside)
	{
		*distp = dist;
		*intersectionp = projected;
		*normp = norm;
		return true;
	}
	float radius2 = SPHERE_RADIUS * SPHERE_RADIUS;
	struct vec3f point;
	struct vec3f v;
	float best = INFINITY;
	float edge_dist;
	point = edge_collision(triangle->points[0], triangle->points[1], p);
	VEC3_SUB(v, p, point);
	edge_dist = VEC3_NORM(v);
	if (edge_dist <= radius2 && edge_dist < best)
	{
		best = edge_dist;
		projected = v;
	}
	point = edge_collision(triangle->points[1], triangle->points[2], p);
	VEC3_SUB(v, p, point);
	edge_dist = VEC3_NORM(v);
	if (edge_dist <= radius2 && edge_dist < best)
	{
		best = edge_dist;
		projected = v;
	}
	point = edge_collision(triangle->points[2], triangle->points[0], p);
	VEC3_SUB(v, p, point);
	edge_dist = VEC3_NORM(v);
	if (edge_dist <= radius2 && edge_dist < best)
	{
		best = edge_dist;
		projected = v;
	}
	if (best == INFINITY)
		return false;
	VEC3_SUB(*normp, p, projected);
	VEC3_NORMALIZE(float, *normp, *normp);
	*distp = dist;
	*intersectionp = projected;
	return true;
}

static bool
get_closest_triangle_point2(struct vec3f p,
                            struct collision_triangle *triangle,
                            struct vec3f norm,
                            struct vec3f *reference_point)
{
	struct vec3f edges[3];
	struct vec3f crossed[3];
	struct vec3f delta[3];
	VEC3_SUB(edges[0], triangle->points[1], triangle->points[0]);
	VEC3_SUB(edges[1], triangle->points[2], triangle->points[1]);
	VEC3_SUB(edges[2], triangle->points[0], triangle->points[2]);
	VEC3_SUB(delta[0], p, triangle->points[0]);
	VEC3_SUB(delta[1], p, triangle->points[1]);
	VEC3_SUB(delta[2], p, triangle->points[2]);
	VEC3_CROSS(crossed[0], delta[0], edges[0]);
	VEC3_CROSS(crossed[1], delta[1], edges[1]);
	VEC3_CROSS(crossed[2], delta[2], edges[2]);
	bool inside = VEC3_DOT(crossed[0], norm) <= EPSILON && VEC3_DOT(crossed[1], norm) <= EPSILON && VEC3_DOT(crossed[2], norm) <= EPSILON;
	if (inside)
	{
		*reference_point = p;
		return true;
	}
	struct vec3f point;
	struct vec3f v;
	float best = INFINITY;
	float edge_dist;
	point = edge_collision(triangle->points[0], triangle->points[1], p);
	VEC3_SUB(v, p, point);
	edge_dist = VEC3_NORM(v);
	if (edge_dist < best)
	{
		best = edge_dist;
		*reference_point = v;
	}
	point = edge_collision(triangle->points[1], triangle->points[2], p);
	VEC3_SUB(v, p, point);
	edge_dist = VEC3_NORM(v);
	if (edge_dist < best)
	{
		best = edge_dist;
		*reference_point = v;
	}
	point = edge_collision(triangle->points[2], triangle->points[0], p);
	VEC3_SUB(v, p, point);
	edge_dist = VEC3_NORM(v);
	if (edge_dist < best)
	{
		best = edge_dist;
		*reference_point = v;
	}
	return best <= SPHERE_RADIUS;
}

static struct vec3f
update_position_collision(struct camera *camera,
                          struct jks_array *triangles,
                          struct vec3f src,
                          struct vec3f dst,
                          size_t recursion,
                          struct vec3f *normp,
                          bool *ground_touched)
{
	if (recursion >= 10)
		return src;
	float best_dist = INFINITY;
	struct collision_triangle *best_triangle = NULL;
	struct vec3f best_norm;
	struct vec3f best_point;

	// Compute capsule line endpoints A, B like before in capsule-capsule case:
	struct vec3f capsule_normal;
	struct vec3f capsule_diff;
	struct vec3f line_end_offset;
	struct vec3f a;
	struct vec3f b;
	VEC3_SUB(capsule_diff, dst, src);
	VEC3_NORMALIZE(float, capsule_normal, capsule_diff);
	VEC3_MULV(line_end_offset, capsule_normal, SPHERE_RADIUS);
	VEC3_ADD(a, src, line_end_offset);
	VEC3_SUB(b, dst, line_end_offset);
	for (size_t i = 0; i < triangles->size; ++i)
	{
		struct collision_triangle *triangle = JKS_ARRAY_GET(triangles, i, struct collision_triangle);
		if (triangle->touched)
			continue;
#if 0
		struct vec3f norm;
		struct vec3f intersection;
		float dist;
		if (!get_closest_triangle_point(dst, triangle, &dist, &intersection, &norm))
			continue;
		if (dist >= best_dist)
			continue;
		best_dist = dist;
		best_triangle = triangle;
		best_norm = norm;
		best_point = intersection;
#else
		struct vec3f e1;
		struct vec3f e2;
		struct vec3f norm;
		VEC3_SUB(e1, triangle->points[1], triangle->points[0]);
		VEC3_SUB(e2, triangle->points[2], triangle->points[0]);
		VEC3_CROSS(norm, e1, e2);
		float tmp1 = fabsf(VEC3_DOT(norm, capsule_normal));
		struct vec3f diff;
		VEC3_SUB(diff, triangle->points[0], src);
		VEC3_DIVV(diff, diff, tmp1);
		float t = VEC3_DOT(norm, diff);
		struct vec3f line_plane_intersection;
		VEC3_MULV(line_plane_intersection, capsule_normal, t);
		VEC3_ADD(line_plane_intersection, line_plane_intersection, src);
		struct vec3f reference_point;
		if (!get_closest_triangle_point2(line_plane_intersection, triangle, norm, &reference_point))
			continue;
		struct vec3f center = edge_collision(a, b, reference_point);

		//struct vec3f norm;
		struct vec3f intersection;
		float dist;
		if (!get_closest_triangle_point(center, triangle, &dist, &intersection, &norm))
			continue;
		if (dist >= best_dist)
			continue;
		best_dist = dist;
		best_triangle = triangle;
		best_norm = norm;
		best_point = intersection;
#endif
	}
	if (!best_triangle)
		return dst;
	best_triangle->touched = true;
	struct vec3f penetration; /* the distance after collision */
	VEC3_SUB(penetration, dst, best_point);
	float len = VEC3_NORM(penetration);
	if (len > EPSILON)
	{
		LOG_INFO("correct %f", len);
		struct vec3f pene_normalized;
		VEC3_DIVV(pene_normalized, penetration, len);
		float d = VEC3_DOT(pene_normalized, best_norm);
		struct vec3f repulsion; 
		VEC3_MULV(repulsion, best_norm, d);
		struct vec3f ray_dir;
		VEC3_SUB(ray_dir, dst, src);
		float dir_len = VEC3_NORM(ray_dir);
		VEC3_MULV(repulsion, repulsion, dir_len + 0.00001f);
		VEC3_SUB(dst, dst, repulsion);
	}
	return update_position_collision(camera, triangles, src, dst, recursion + 1, normp, ground_touched);
}
#endif

static uint32_t
reduce_flags(uint32_t flags, uint32_t test)
{
	if ((flags & test) == test)
		flags &= ~test;
	return flags;
}

static uint32_t
get_move_flags(struct camera *camera)
{
	uint32_t flags = 0;

	if (gfx_is_key_down(g_wow->window, GFX_KEY_W))
		flags |= CAMERA_MOVE_FRONT;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_S))
		flags |= CAMERA_MOVE_BACK;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_Q))
		flags |= CAMERA_MOVE_LEFT;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_E))
		flags |= CAMERA_MOVE_RIGHT;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_A))
		flags |= CAMERA_TURN_LEFT;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_D))
		flags |= CAMERA_TURN_RIGHT;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_SPACE))
		flags |= CAMERA_MOVE_UP;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_LSHIFT))
		flags |= CAMERA_MOVE_DOWN;
	if (gfx_is_mouse_button_down(g_wow->window, GFX_MOUSE_BUTTON_LEFT)
	 && gfx_is_mouse_button_down(g_wow->window, GFX_MOUSE_BUTTON_RIGHT))
		flags |= CAMERA_MOVE_FRONT;
	if (camera->autorun)
	{
		if (flags & (CAMERA_MOVE_FRONT | CAMERA_MOVE_BACK))
			camera->autorun = false;
		else
			flags |= CAMERA_MOVE_FRONT;
	}
	flags = reduce_flags(flags, CAMERA_MOVE_FRONT | CAMERA_MOVE_BACK);
	flags = reduce_flags(flags, CAMERA_MOVE_LEFT | CAMERA_MOVE_RIGHT);
	flags = reduce_flags(flags, CAMERA_TURN_LEFT | CAMERA_TURN_RIGHT);
	flags = reduce_flags(flags, CAMERA_MOVE_UP | CAMERA_MOVE_DOWN);
	return flags;
}

static void
handle_keyboard_worldobj(struct camera *camera)
{
	uint32_t move_flags;
	uint32_t delta;

	move_flags = get_move_flags(camera);
	if (GROUNDED(camera))
	{
		if (gfx_is_key_down(g_wow->window, GFX_KEY_SPACE))
		{
			unit_jump((struct unit*)camera->worldobj);
			worldobj_send_move_packet(camera->worldobj, MSG_MOVE_JUMP);
		}
	}
	if (move_flags & CAMERA_MOVE_FRONT)
		camera->worldobj->movement_data.flags |= MOVEFLAG_FORWARD;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_FORWARD;
	if (move_flags & CAMERA_MOVE_BACK)
		camera->worldobj->movement_data.flags |= MOVEFLAG_BACKWARD;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_BACKWARD;
	if (move_flags & CAMERA_MOVE_LEFT)
		camera->worldobj->movement_data.flags |= MOVEFLAG_STRAFE_LEFT;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_STRAFE_LEFT;
	if (move_flags & CAMERA_MOVE_RIGHT)
		camera->worldobj->movement_data.flags |= MOVEFLAG_STRAFE_RIGHT;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_STRAFE_RIGHT;
	if (move_flags & CAMERA_MOVE_UP)
		camera->worldobj->movement_data.flags |= MOVEFLAG_ASCENDING;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_ASCENDING;
	if (move_flags & CAMERA_MOVE_DOWN)
		camera->worldobj->movement_data.flags |= MOVEFLAG_DESCENDING;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_DESCENDING;
	if (move_flags & CAMERA_TURN_LEFT)
		camera->worldobj->movement_data.flags |= MOVEFLAG_TURN_LEFT;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_TURN_LEFT;
	if (move_flags & CAMERA_TURN_RIGHT)
		camera->worldobj->movement_data.flags |= MOVEFLAG_TURN_RIGHT;
	else
		camera->worldobj->movement_data.flags &= ~MOVEFLAG_TURN_RIGHT;
	if (!(g_wow->wow_opt & WOW_OPT_FOCUS_3D))
		camera->rot.y = -camera->worldobj->orientation;
	unit_physics((struct unit*)camera->worldobj);
	delta = camera->last_move_flags ^ move_flags;
	if (delta)
	{
		if (delta & CAMERA_MOVE_FRONT)
		{
			if (move_flags & CAMERA_MOVE_FRONT)
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_START_FORWARD);
			else
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_STOP);
		}
		if (delta & CAMERA_MOVE_BACK)
		{
			if (move_flags & CAMERA_MOVE_BACK)
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_START_BACKWARD);
			else
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_STOP);
		}
		if (delta & CAMERA_MOVE_LEFT)
		{
			if (move_flags & CAMERA_MOVE_LEFT)
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_START_STRAFE_LEFT);
			else
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_STOP_STRAFE);
		}
		if (delta & CAMERA_MOVE_RIGHT)
		{
			if (move_flags & CAMERA_MOVE_RIGHT)
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_START_STRAFE_RIGHT);
			else
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_STOP_STRAFE);
		}
		if (delta & CAMERA_TURN_LEFT)
		{
			if (move_flags & CAMERA_TURN_LEFT)
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_START_TURN_LEFT);
			else
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_STOP_TURN);
		}
		if (delta & CAMERA_TURN_RIGHT)
		{
			if (move_flags & CAMERA_TURN_RIGHT)
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_START_TURN_RIGHT);
			else
				worldobj_send_move_packet(camera->worldobj, MSG_MOVE_STOP_TURN);
		}
		camera->last_move_update = g_wow->frametime;
	}
	else if (move_flags)
	{
		if (g_wow->frametime - camera->last_move_update >= 500000000)
		{
			worldobj_send_move_packet(camera->worldobj, MSG_MOVE_HEARTBEAT);
			camera->last_move_update = g_wow->frametime;
		}
	}
	camera->last_move_flags = move_flags;
}

void
camera_handle_keyboard(struct camera *camera)
{
	uint32_t move_flags = 0;
	struct vec3f src;
	struct vec3f dst;
	struct vec3f rot;
	float speed;
	float dt;

	if (camera->worldobj)
	{
		handle_keyboard_worldobj(camera);
		return;
	}
	move_flags = get_move_flags(camera);
	src = camera->pos;
	rot = camera->rot;
	speed = MOVE_SPEED;
	dst = src;
	dt = (g_wow->frametime - g_wow->lastframetime) / 1000000000.0;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_LCONTROL))
		speed *= 10;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_RCONTROL))
		speed *= 10;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_RSHIFT))
		speed /= 10;
	if (move_flags & (CAMERA_MOVE_FRONT | CAMERA_MOVE_BACK | CAMERA_MOVE_LEFT | CAMERA_MOVE_RIGHT | CAMERA_MOVE_UP | CAMERA_MOVE_DOWN))
	{
		struct vec3f nxt = {0};
		if (move_flags & CAMERA_MOVE_FRONT)
			nxt.z -= 1;
		if (move_flags & CAMERA_MOVE_BACK)
			nxt.z += 1;
		if (move_flags & CAMERA_MOVE_LEFT)
			nxt.x -= 1;
		if (move_flags & CAMERA_MOVE_RIGHT)
			nxt.x += 1;
		struct mat3f mat;
		MAT3_IDENTITY(mat);
		float norm = VEC3_NORM(nxt);
		if (norm != 0)
		{
			struct mat3f tmp;
			VEC3_DIVV(nxt, nxt, norm);
			MAT3_ROTATEZ(float, tmp, mat, -rot.z);
			MAT3_ROTATEY(float, mat, tmp, -rot.y);
			MAT3_ROTATEX(float, tmp, mat, -rot.x);
			mat = tmp;
		}
		if (move_flags & CAMERA_MOVE_UP)
			nxt.y += 1;
		if (move_flags & CAMERA_MOVE_DOWN)
			nxt.y -= 1;
		float nxt_norm = VEC3_NORM(nxt);
		if (nxt_norm >= EPSILON)
		{
			VEC3_DIVV(nxt, nxt, nxt_norm);
			struct vec3f tmp;
			MAT3_VEC3_MUL(tmp, mat, nxt);
			float step = speed * dt;
			VEC3_MULV(nxt, tmp, step);
			VEC3_ADD(dst, src, nxt);
		}
		else
		{
			dst = src;
		}
	}
	if (move_flags & CAMERA_TURN_LEFT)
		camera->rot.y -= M_PI * dt;
	else if (move_flags & CAMERA_TURN_RIGHT)
		camera->rot.y += M_PI * dt;
	camera->pos = dst;
}

void
camera_handle_scroll(struct camera *camera, float scroll)
{
	if (!camera->worldobj)
		return;
	scroll = -scroll;
	float delta = scroll * 2;
	if (scroll < 0)
	{
		if (camera->unit_max_distance + delta < 0)
			camera->unit_max_distance = 0;
		else
			camera->unit_max_distance += delta;
	}
	else if (scroll > 0)
	{
		if (camera->unit_max_distance + delta > 100)
			camera->unit_max_distance = 100;
		else
			camera->unit_max_distance += delta;
	}
}
