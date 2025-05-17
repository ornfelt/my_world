#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool
object_alloc(struct gl_ctx *ctx, struct object *object)
{
	GLuint id;

	if (!jkg_id_alloc(&ctx->id_list, &id))
	{
		GL_SET_ERR(ctx, GL_OUT_OF_MEMORY);
		return false;
	}
	if (id >= ctx->objects_size)
	{
		struct object **objects;
		size_t size;

		size = ctx->objects_size * 2;
		if (size < 1024)
			size = 1024;
		while (size <= id)
			size *= 2;
		objects = realloc(ctx->objects, size * sizeof(*objects));
		if (!objects)
		{
			GL_SET_ERR(ctx, GL_OUT_OF_MEMORY);
			return false;
		}
		memset(&objects[ctx->objects_size],
		       0,
		       sizeof(*objects) * (size - ctx->objects_size));
		ctx->objects = objects;
		ctx->objects_size = size;
	}
	object->id = id;
	ctx->objects[id] = object;
	return true;
}

void
object_free(struct gl_ctx *ctx, struct object *object)
{
	if (object->id)
	{
		ctx->objects[object->id] = NULL;
		if (!jkg_id_free(&ctx->id_list, object->id))
			assert(!"failed to release id");
	}
	free(object);
}

void *
object_get(struct gl_ctx *ctx, GLuint id, enum object_type type)
{
	if (!id || id >= ctx->objects_size)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return NULL;
	}
	if (!ctx->objects[id])
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return NULL;
	}
	if (ctx->objects[id]->type != type)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return NULL;
	}
	return ctx->objects[id];
}

bool
object_is(struct gl_ctx *ctx, GLuint id, enum object_type type)
{
	if (!id || id >= ctx->objects_size)
		return false;
	if (!ctx->objects[id])
		return false;
	if (ctx->objects[id]->type != type)
		return false;
	return true;
}
