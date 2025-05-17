#include "internal.h"

#include <stdlib.h>

struct renderbuffer *
renderbuffer_alloc(struct gl_ctx *ctx)
{
	struct renderbuffer *renderbuffer;

	(void)ctx;
	renderbuffer = calloc(1, sizeof(*renderbuffer));
	if (!renderbuffer)
		return NULL;
	renderbuffer->object.type = OBJECT_RENDERBUFFER;
	renderbuffer->image = NULL;
	renderbuffer->dirty = false;
	return renderbuffer;
}

void
glGenRenderbuffers(GLsizei n,
                   GLuint *renderbuffers)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		struct renderbuffer *renderbuffer = renderbuffer_alloc(ctx);
		if (!renderbuffer)
		{
			/* XXX */
			continue;
		}
		if (!object_alloc(ctx, &renderbuffer->object))
		{
			renderbuffer_free(ctx, renderbuffer);
			/* XXX */
			continue;
		}
		renderbuffers[i] = renderbuffer->object.id;
	}
}

void
renderbuffer_free(struct gl_ctx *ctx,
                  struct renderbuffer *renderbuffer)
{
	if (renderbuffer == ctx->renderbuffer)
		renderbuffer_bind(ctx, NULL);
	if (renderbuffer->image)
		ctx->jkg_op->image_free(ctx->jkg_ctx, renderbuffer->image);
	object_free(ctx, &renderbuffer->object);
}

void
glDeleteRenderbuffers(GLsizei n,
                      GLuint *renderbuffers)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		GLuint id = renderbuffers[i];
		struct renderbuffer *renderbuffer = object_get(ctx, id, OBJECT_RENDERBUFFER);
		if (!renderbuffer)
			continue;
		renderbuffer_free(ctx, renderbuffer);
	}
}

void
renderbuffer_bind(struct gl_ctx *ctx,
                  struct renderbuffer *renderbuffer)
{
	ctx->renderbuffer = renderbuffer;
}

void
glBindRenderbuffer(GLenum target,
                   GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct renderbuffer *renderbuffer;

	if (target != GL_RENDERBUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (id)
	{
		renderbuffer = object_get(ctx, id, OBJECT_RENDERBUFFER);
		if (!renderbuffer)
			return;
	}
	else
	{
		renderbuffer = NULL;
	}
	renderbuffer_bind(ctx, renderbuffer);
}

static void
renderbuffer_storage(struct gl_ctx *ctx,
                     struct renderbuffer *renderbuffer,
                     GLenum internalformat,
                     GLsizei width,
                     GLsizei height)
{
	struct jkg_image_create_info create_info;
	struct jkg_image *image;
	int ret;

	if (width < 0
	 || height < 0
	 || (GLuint)width > ctx->jkg_caps->max_renderbuffer_size
	 || (GLuint)height > ctx->jkg_caps->max_renderbuffer_size)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_image_internal_format(internalformat, &create_info.format))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	create_info.type = JKG_IMAGE_2D;
	create_info.size.x = width;
	create_info.size.y = height;
	create_info.size.z = 0;
	create_info.levels = 1;
	create_info.layers = 0;
	ret = ctx->jkg_op->image_alloc(ctx->jkg_ctx,
	                               &create_info,
	                               &image);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
	if (renderbuffer->image)
		ctx->jkg_op->image_free(ctx->jkg_ctx, renderbuffer->image);
	renderbuffer->image = image;
	renderbuffer->format = create_info.format;
	renderbuffer->width = width;
	renderbuffer->height = height;
	renderbuffer->dirty = true;
}

void
glRenderbufferStorage(GLenum target,
                      GLenum internalformat,
                      GLsizei width,
                      GLsizei height)
{
	struct gl_ctx *ctx = g_ctx;
	struct renderbuffer *renderbuffer;

	if (target != GL_RENDERBUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	renderbuffer = ctx->renderbuffer;
	if (!renderbuffer)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	renderbuffer_storage(ctx, renderbuffer, internalformat, width, height);
}

void
glNamedRenderbufferStorage(GLuint id,
                           GLenum internalformat,
                           GLsizei width,
                           GLsizei height)
{
	struct gl_ctx *ctx = g_ctx;
	struct renderbuffer *renderbuffer;

	renderbuffer = object_get(ctx, id, OBJECT_RENDERBUFFER);
	if (!renderbuffer)
		return;
	renderbuffer_storage(ctx, renderbuffer, internalformat, width, height);
}
