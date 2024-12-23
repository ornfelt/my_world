#include "player/player.h"

#include "world/block.h"
#include "world/world.h"

#include "voxel.h"

#include <jks/vec3.h>

#include <gfx/events.h>
#include <gfx/window.h>

#include <stdlib.h>
#include <math.h>

#define WALK_SPEED 1
#define RUN_SPEED  1.3
#define FLY_SPEED  5
#define SWIM_SPEED 0.02

static void player_destroy(struct entity *entity);
static void player_tick(struct entity *entity);
static void player_draw(struct entity *entity);

static const struct entity_vtable player_vtable =
{
	.destroy = player_destroy,
	.tick = player_tick,
	.draw = player_draw,
	.update_parent_chunk = entity_priv_update_parent_chunk,
};

struct player *player_new(struct world *world)
{
	struct player *player = malloc(sizeof(*player));
	if (!player)
		return NULL;
	player->entity.vtable = &player_vtable;
	entity_init(&player->entity, world, NULL);
	player_inventory_init(&player->inventory);
	player_raycast_init(&player->raycast, player);
	player->old_mouse_x = 0;
	player->old_mouse_y = 0;
	player->eye_in_water = false;
	player->entity.flying = true;
	entity_set_size(&player->entity, (struct vec3f){.6, 1.8, .6});
	entity_set_pos(&player->entity, (struct vec3f){8, 150, 8});
	return player;
}

static void player_destroy(struct entity *entity)
{
	struct player *player = (struct player*)entity;
	player_raycast_destroy(&player->raycast);
	player_inventory_destroy(&player->inventory);
}

static void handle_movement(struct player *player)
{
	struct vec3f add;
	VEC3_SETV(add, 0);
	bool key_w = gfx_is_key_down(g_voxel->window, GFX_KEY_W);
	bool key_a = gfx_is_key_down(g_voxel->window, GFX_KEY_A);
	bool key_s = gfx_is_key_down(g_voxel->window, GFX_KEY_S);
	bool key_d = gfx_is_key_down(g_voxel->window, GFX_KEY_D);
	bool key_space = gfx_is_key_down(g_voxel->window, GFX_KEY_SPACE);
	bool key_lshift = gfx_is_key_down(g_voxel->window, GFX_KEY_LSHIFT);
	if (!key_w && !key_a && !key_s && !key_d && !key_space && !key_lshift)
		return;
	if (key_w && key_s)
	{
		key_w = false;
		key_s = false;
	}
	if (key_a && key_d)
	{
		key_a = false;
		key_d = false;
	}
	if (key_space && key_lshift)
	{
		key_space = false;
		key_lshift = false;
	}
	if (key_w || key_a || key_s || key_d)
	{
		float angle = player->entity.rot.y;
		if (key_w && key_d)
			angle -= 45;
		else if (key_w && key_a)
			angle -= 135;
		else if (key_w)
			angle -= 90;
		else if (key_s && key_a)
			angle += 135;
		else if (key_s && key_d)
			angle += 45;
		else if (key_s)
			angle += 90;
		else if (key_a)
			angle += 180;
		else if (key_d)
			angle += 0;
		add.x = cosf(angle / 180. * M_PI);
		add.z = sinf(angle / 180. * M_PI);
		if (player->entity.flying)
		{
			add.x *= FLY_SPEED;
			add.z *= FLY_SPEED;
		}
		else if (player->entity.in_water)
		{
			add.x *= SWIM_SPEED;
			add.z *= SWIM_SPEED;
		}
		else if (gfx_is_key_down(g_voxel->window, GFX_KEY_LCONTROL))
		{
			add.x *= RUN_SPEED;
			add.z *= RUN_SPEED;
		}
		else
		{
			add.x *= WALK_SPEED;
			add.z *= WALK_SPEED;
		}
	}
	if (player->entity.flying)
	{
		if (key_space)
			add.y += FLY_SPEED;
		else if (key_lshift)
			add.y -= FLY_SPEED;
	}
	else
	{
		if (key_space)
		{
			if (player->entity.in_water)
				add.y += .04;
			else if (player->entity.is_on_floor)
				entity_jump(&player->entity);
		}
	}
	if (player->entity.flying)
		VEC3_MULV(add, add, .1);
	else if (!player->entity.in_water)
	{
		if (player->entity.is_on_floor)
			VEC3_MULV(add, add, .1);
		else
			VEC3_MULV(add, add, .05);
	}
	VEC3_ADD(player->entity.pos_dst, player->entity.pos_dst, add);
}

static void handle_rotation(struct player *player)
{
	int32_t mouse_x = gfx_get_mouse_x(g_voxel->window);
	int32_t mouse_y = gfx_get_mouse_y(g_voxel->window);
	int32_t delta_x = mouse_x - player->old_mouse_x;
	int32_t delta_y = mouse_y - player->old_mouse_y;
	if (!delta_x && !delta_y)
		return;
	player->old_mouse_x = mouse_x;
	player->old_mouse_y = mouse_y;
	if (!g_voxel->grabbed)
		return;
	player->entity.rot.y += delta_x / 20.0f;
	player->entity.rot.x += delta_y / 20.0f;
	player->entity.rot.y = fmodf(player->entity.rot.y, 3600.0f);
	if (player->entity.rot.y < 0)
		player->entity.rot.y += 360;
	if (player->entity.rot.x > 90)
		player->entity.rot.x = 90;
	else if (player->entity.rot.x < -90)
		player->entity.rot.x = -90;
}

static void player_tick(struct entity *entity)
{
	struct player *player = (struct player*)entity;
	handle_movement(player);
	entity_priv_tick(entity);
	player_raycast_tick(&player->raycast);
}

static void player_draw(struct entity *entity)
{
	struct player *player = (struct player*)entity;
	player_raycast_draw(&player->raycast);
}

void player_update(struct player *player)
{
	struct vec3f pos;
	VEC3_CPY(pos, player->entity.pos);
	pos.y += 0.72 + 2.0 / 16;
	VEC3_FN1(pos, pos, ceil);
	struct block block;
	if (world_get_block(player->entity.world, pos.x, pos.y, pos.z, &block))
		player->eye_in_water = (block.type == 8 || block.type == 9);
	else
		player->eye_in_water = false;
	player->eye_light = world_get_light(player->entity.world, pos.x, pos.y, pos.z);
	handle_rotation(player);
	float fov = 90;
	if (player->eye_in_water)
		fov -= 10;
	if (gfx_is_key_down(g_voxel->window, GFX_KEY_LCONTROL))
		fov += 10;
	MAT4_PERSPECTIVE(player->mat_p, fov / 180. * M_PI, g_voxel->window->width / (float)g_voxel->window->height, .05, 1500);
	//player->mat_v = Mat4::translate(player->mat_v, Vec3(std::cos(nanotime / 800000000. * M_PI * 2) * 0.02, 0, 0));
	//player->mat_v = Mat4::rotateZ(player->mat_v, std::pow(std::cos(nanotime / 1600000000. * M_PI * 2) * 2, 2) / 4 * 0.010);
	//player->mat_v = Mat4::rotateX(player->mat_v, std::pow(std::cos(nanotime /  800000000. * M_PI * 2) * 2, 2) / 4 * 0.005 + player->rot.x / 180. * M_PI);
	struct mat4f tmp1;
	struct vec3f camera_pos = entity_get_real_pos(&player->entity);
	VEC3_NEGATE(camera_pos, camera_pos);
	camera_pos.y -= 0.72;
	MAT4_IDENTITY(player->mat_v);
	MAT4_ROTATEX(float, tmp1, player->mat_v, player->entity.rot.x / 180.0 * M_PI);
	MAT4_ROTATEY(float, player->mat_v, tmp1, player->entity.rot.y / 180.0 * M_PI);
	MAT4_ROTATEZ(float, tmp1, player->mat_v, player->entity.rot.z / 180.0 * M_PI);
	MAT4_TRANSLATE(player->mat_v, tmp1, camera_pos);
	MAT4_MUL(player->mat_vp, player->mat_p, player->mat_v);
	frustum_update(&player->entity.world->frustum, &player->mat_vp);
	player_raycast_update(&player->raycast);
}
