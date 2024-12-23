#ifndef PLAYER_INVENTORY_H
#define PLAYER_INVENTORY_H

#include <stdint.h>

struct player_inventory_slot
{
	uint16_t id;
	uint8_t amount;
};

struct player_inventory
{
	struct player_inventory_slot slots[36];
	int8_t bar_pos;
};

void player_inventory_init(struct player_inventory *inventory);
void player_inventory_destroy(struct player_inventory *inventory);

#endif
