#ifndef ENTITY_ENTITIES_H
#define ENTITY_ENTITIES_H

#include <gfx/objects.h>

#include <sys/queue.h>

struct entity;
struct chunk;

struct entities
{
	TAILQ_HEAD(, entity) entities;
	gfx_buffer_t uniform_buffer;
	struct chunk *chunk;
};

bool entities_init(struct entities *entities, struct chunk *chunk);
void entities_destroy(struct entities *entities);
void entities_tick(struct entities *entities);
void entities_draw(struct entities *entities);
void entities_add(struct entities *entitites, struct entity *entity);
void entities_remove(struct entities *entities, struct entity *entity);

#endif
