#include "xsrv.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static const char *
object_name(struct xsrv *xsrv, struct object *object)
{
	return xsrv->objects_defs[object->type]->name;
}

void
object_init(struct xsrv *xsrv,
            struct client *client,
            struct object *object,
            uint8_t type,
            uint32_t id)
{
	(void)xsrv;
	object->client = client;
	object->type = type;
	object->id = id;
	object->refs = 1;
	xsrv->objects_stats[type].count++;
}

static uint32_t
object_hash(uint32_t id)
{
	id = ((id >> 16) ^ id) * 0x45D9F3B;
	id = ((id >> 16) ^ id) * 0x45D9F3B;
	return id;
}

static struct object_head *
object_head(struct xsrv *xsrv, uint32_t id)
{
	return &xsrv->objects_map[object_hash(id) % xsrv->objects_map_size];
}

static void
grow_objects_map(struct xsrv *xsrv)
{
	size_t new_size;
	struct object_head *objects;

	new_size = xsrv->objects_map_size * 2;
	objects = realloc(xsrv->objects_map, sizeof(*objects) * new_size);
	if (!objects)
	{
		fprintf(stderr, "%s: realloc: %s\n",
		        xsrv->progname,
		        strerror(errno));
		abort(); /* XXX */
	}
	for (size_t i = 0; i < new_size; ++i)
		TAILQ_INIT(&objects[i]);
	for (size_t i = 0; i < xsrv->objects_map_size; ++i)
	{
		struct object_head *head = &xsrv->objects_map[i];
		struct object *object;
		while ((object = TAILQ_FIRST(head)))
		{
			TAILQ_REMOVE(head, object, map_chain);
			TAILQ_INSERT_TAIL(&objects[object_hash(object->id) % new_size], object, map_chain);
		}
	}
	xsrv->objects_map = objects;
	xsrv->objects_map_size = new_size;
}

void
object_add(struct xsrv *xsrv, struct object *object)
{
	if (xsrv->objects_map_count + 1 >= xsrv->objects_map_size / 2)
		grow_objects_map(xsrv);
	xsrv->objects_map_count++;
	object->refs++;
	TAILQ_INSERT_TAIL(object_head(xsrv, object->id), object, map_chain);
}

void
object_remove(struct xsrv *xsrv, struct object *object)
{
	xsrv->objects_map_count--;
	TAILQ_REMOVE(object_head(xsrv, object->id), object, map_chain);
	object_free(xsrv, object);
}

struct object *
object_get(struct xsrv *xsrv, uint32_t id)
{
	struct object *object;

	TAILQ_FOREACH(object, object_head(xsrv, id), map_chain)
	{
		if (object->id == id)
		{
			object->refs++;
			return object;
		}
	}
	return NULL;
}

void
object_free(struct xsrv *xsrv, struct object *object)
{
	if (!object)
		return;
	if (!object->refs)
	{
		fprintf(stderr, "%s: %s double free: %" PRIx32 "\n",
		        xsrv->progname,
		        object_name(xsrv, object),
		        object->id);
		abort();
	}
	if (--object->refs)
		return;
	if (object->id)
	{
		fprintf(stderr, "%s: freeing non-destroyed %s: %" PRIx32 "\n",
		        xsrv->progname,
		        object_name(xsrv, object),
		        object->id);
		abort();
	}
	xsrv->objects_stats[object->type].destroyed_count--;
	xsrv->objects_stats[object->type].count--;
	if (xsrv->objects_defs[object->type]->free)
		xsrv->objects_defs[object->type]->free(xsrv, object);
	free(object);
}

void
object_destroy(struct xsrv *xsrv, struct object *object)
{
	if (!object->id)
	{
		fprintf(stderr, "%s: %s double destroy\n",
		        xsrv->progname,
		        object_name(xsrv, object));
		abort();
	}
	if (object->client)
	{
		TAILQ_REMOVE(&object->client->objects,
		             object,
		             client_chain);
	}
	object_remove(xsrv, object);
	xsrv->objects_stats[object->type].destroyed_count++;
	if (xsrv->objects_defs[object->type]->destroy)
		xsrv->objects_defs[object->type]->destroy(xsrv, object);
	object->id = 0;
}
