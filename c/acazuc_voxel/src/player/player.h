#ifndef PLAYER_H
#define PLAYER_H

#include "entity/entity.h"
#include "inventory.h"
#include "raycast.h"

#include <jks/mat4.h>

struct world;

struct player
{
	struct entity entity;
	struct player_inventory inventory;
	struct player_raycast raycast;
	struct mat4f mat_vp;
	struct mat4f mat_p;
	struct mat4f mat_v;
	int32_t old_mouse_x;
	int32_t old_mouse_y;
	uint8_t eye_in_water;
	uint8_t eye_light;
};

struct player *player_new(struct world *world);
void player_update(struct player *player);

#endif
