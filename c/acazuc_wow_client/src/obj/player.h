#ifndef PLAYER_H
#define PLAYER_H

#include "obj/unit.h"

struct player
{
	struct unit unit;
	struct unit_item left_hand_item;
	struct unit_item right_hand_item;
	bool loading_skin_texture;
	bool dirty_skin_texture;
	bool clean_batches;
	bool db_get;
};

struct player *player_new(uint64_t guid);

extern const struct object_vtable player_object_vtable;
extern const struct unit_vtable player_unit_vtable;
extern const struct worldobj_vtable player_worldobj_vtable;

#endif
