#ifndef PLAYER_RAYCAST_H
#define PLAYER_RAYCAST_H

#include <jks/aabb.h>
#include <jks/vec3.h>

#include <gfx/objects.h>

struct player;
struct chunk;

struct player_raycast
{
	struct player *player;
	gfx_attributes_state_t hover_attributes_state;
	gfx_attributes_state_t break_attributes_state;
	gfx_buffer_t hover_vertexes_buffer;
	gfx_buffer_t break_vertexes_buffer;
	gfx_buffer_t hover_indices_buffer;
	gfx_buffer_t break_indices_buffer;
	gfx_buffer_t hover_uniform_buffer;
	gfx_buffer_t break_uniform_buffer;
	struct vec3f pos;
	int32_t todo_ticks;
	int32_t done_ticks;
	uint8_t face;
	bool found;
};

void player_raycast_init(struct player_raycast *raycast, struct player *player);
void player_raycast_destroy(struct player_raycast *raycast);
void player_raycast_tick(struct player_raycast *raycast);
void player_raycast_update(struct player_raycast *raycast);
void player_raycast_draw(struct player_raycast *raycast);

#endif
