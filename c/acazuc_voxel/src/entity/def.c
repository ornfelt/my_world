#include "entity/entity.h"
#include "entity/def.h"

#include "log.h"

#include <string.h>
#include <stdlib.h>

struct entities_def *entities_def_new(void)
{
	struct entities_def *entities = calloc(sizeof(*entities), 1);
	if (!entities)
	{
		LOG_ERROR("entities allocation failed");
		return NULL;
	}

#define INIT_ENTITY(name) \
	do \
	{ \
		LOG_INFO("initializing entity " #name); \
		if (!name##_def_init(&entities->name)) \
			goto err; \
	} while (0)

	INIT_ENTITY(pigzombie);
	INIT_ENTITY(skeleton);
	INIT_ENTITY(creeper);
	INIT_ENTITY(zombie);
	INIT_ENTITY(pigman);
	INIT_ENTITY(human);
	INIT_ENTITY(sheep);
	INIT_ENTITY(slime);
	INIT_ENTITY(pig);
	INIT_ENTITY(cow);
	return entities;

#undef INIT_ENTITY

err:
	entities_def_delete(entities);
	return NULL;
}

void entities_def_delete(struct entities_def *entities)
{
	if (!entities)
		return;

#define DESTROY_ENTITY(name) \
	do \
	{ \
		LOG_INFO("destroying entity " #name); \
		name##_def_destroy(&entities->name); \
	} while (0)

	DESTROY_ENTITY(pigzombie);
	DESTROY_ENTITY(skeleton);
	DESTROY_ENTITY(creeper);
	DESTROY_ENTITY(zombie);
	DESTROY_ENTITY(pigman);
	DESTROY_ENTITY(human);
	DESTROY_ENTITY(sheep);
	DESTROY_ENTITY(slime);
	DESTROY_ENTITY(pig);
	DESTROY_ENTITY(cow);

#undef DESTROY_ENTITY

	free(entities);
}
