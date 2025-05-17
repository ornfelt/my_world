#include "internal.h"

#include <stdlib.h>
#include <assert.h>

struct framebuffer *
framebuffer_alloc(struct gl_ctx *ctx)
{
	struct framebuffer *fbo;

	fbo = calloc(1, sizeof(*fbo));
	if (!fbo)
		return NULL;
	fbo->object.type = OBJECT_FRAMEBUFFER;
	for (size_t i = 0; i < ctx->jkg_caps->max_color_attachments; ++i)
	{
		fbo->color_attachments[i].type = GL_NONE;
		fbo->color_attachments[i].object = NULL;
		fbo->color_attachments[i].surface = NULL;
		fbo->color_attachments[i].dirty = false;
	}
	for (size_t i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		fbo->draw_buffers[i] = GL_NONE;
	fbo->depth_attachment.type = GL_NONE;
	fbo->depth_attachment.object = NULL;
	fbo->depth_attachment.surface = NULL;
	fbo->depth_attachment.dirty = false;
	fbo->stencil_attachment.type = GL_NONE;
	fbo->stencil_attachment.object = NULL;
	fbo->stencil_attachment.surface = NULL;
	fbo->stencil_attachment.dirty = false;
	fbo->dirty = false;
	return fbo;
}

static bool
get_fbo(struct gl_ctx *ctx, GLenum target, struct framebuffer **fbop)
{
	switch (target)
	{
		case GL_READ_FRAMEBUFFER:
			*fbop = ctx->read_fbo;
			return true;
		case GL_FRAMEBUFFER:
		case GL_DRAW_FRAMEBUFFER:
			*fbop = ctx->draw_fbo;
			return true;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return false;
	}
}

int
update_default_fbo(struct gl_ctx *ctx)
{
	struct jkg_image *color_images[2];
	struct jkg_image *depth_image;
	struct jkg_image *stencil_image;
	enum jkg_format color_format;
	enum jkg_format depth_format;
	enum jkg_format stencil_format;
	uint32_t width;
	uint32_t height;
	int ret;

	ret = ctx->jkg_op->get_default_images(ctx->jkg_ctx,
	                                      color_images,
	                                      &color_format,
	                                      &depth_image,
	                                      &depth_format,
	                                      &stencil_image,
	                                      &stencil_format,
	                                      &width,
	                                      &height);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	if (color_images[!ctx->back_buffer] != ctx->default_fbo->color_attachments[0].image)
	{
		ctx->default_fbo->color_attachments[0].type = GL_FRAMEBUFFER_DEFAULT;
		ctx->default_fbo->color_attachments[0].image = color_images[ctx->back_buffer];
		ctx->default_fbo->color_attachments[0].format = color_format;
		ctx->default_fbo->color_attachments[0].dirty = true;
	}
	if (color_images[ctx->back_buffer] != ctx->default_fbo->color_attachments[1].image)
	{
		ctx->default_fbo->color_attachments[1].type = GL_FRAMEBUFFER_DEFAULT;
		ctx->default_fbo->color_attachments[1].image = color_images[ctx->back_buffer];
		ctx->default_fbo->color_attachments[1].format = color_format;
		ctx->default_fbo->color_attachments[1].dirty = true;
	}
	if (depth_image != ctx->default_fbo->depth_attachment.image)
	{
		ctx->default_fbo->depth_attachment.type = GL_FRAMEBUFFER_DEFAULT;
		ctx->default_fbo->depth_attachment.image = depth_image;
		ctx->default_fbo->depth_attachment.format = depth_format;
		ctx->default_fbo->depth_attachment.dirty = true;
	}
	if (stencil_image != ctx->default_fbo->stencil_attachment.image)
	{
		ctx->default_fbo->stencil_attachment.type = GL_FRAMEBUFFER_DEFAULT;
		ctx->default_fbo->stencil_attachment.image = stencil_image;
		ctx->default_fbo->stencil_attachment.format = stencil_format;
		ctx->default_fbo->stencil_attachment.dirty = true;
	}
	return 0;
}

static int
attachment_commit(struct gl_ctx *ctx,
                  struct framebuffer *fbo,
                  struct framebuffer_attachment *attachment)
{
	struct jkg_surface_create_info create_info;
	struct jkg_surface *surface;
	int ret;

	switch (attachment->type)
	{
		default:
			assert(!"unknown attachment type");
			/* FALLTHROUGH */
		case GL_NONE:
			if (!attachment->dirty)
				return 0;
			attachment->format = JKG_FORMAT_UNKNOWN;
			create_info.image = NULL;
			break;
		case GL_FRAMEBUFFER_DEFAULT:
			if (!attachment->image)
			{
				if (!attachment->dirty)
					return 0;
				create_info.image = NULL;
				break;
			}
			if (!attachment->dirty)
				return 0;
			/* format already set on update */
			create_info.image = attachment->image;
			create_info.level = 0; /* XXX */
			create_info.min_layer = 0; /* XXX */
			create_info.num_layers = 1; /* XXX */
			break;
		case GL_TEXTURE:
			if (!attachment->texture)
			{
				if (!attachment->dirty)
					return 0;
				create_info.image = NULL;
				break;
			}
			if (attachment->texture->dirty & GL_TEXTURE_DIRTY_SURFACE)
			{
				attachment->dirty = true;
				attachment->texture->dirty &= ~GL_TEXTURE_DIRTY_SURFACE;
			}
			if (!attachment->dirty)
				return 0;
			attachment->format = attachment->texture->jkg_format;
			create_info.image = attachment->texture->image;
			create_info.level = 0; /* XXX */
			create_info.min_layer = 0; /* XXX */
			create_info.num_layers = 1; /* XXX */
			break;
		case GL_RENDERBUFFER:
			if (!attachment->renderbuffer)
			{
				if (!attachment->dirty)
					return 0;
				create_info.image = NULL;
				break;
			}
			if (attachment->renderbuffer->dirty)
			{
				attachment->dirty = true;
				attachment->renderbuffer->dirty = false;
			}
			if (!attachment->dirty)
				return 0;
			attachment->format = attachment->renderbuffer->format;
			create_info.image = attachment->renderbuffer->image;
			create_info.level = 0; /* XXX */
			create_info.min_layer = 0; /* XXX */
			create_info.num_layers = 1; /* XXX */
			break;
	}
	if (attachment == &fbo->depth_attachment
	 && attachment->type == fbo->stencil_attachment.type
	 && attachment->image == fbo->stencil_attachment.image
	 && !fbo->stencil_attachment.dirty)
	{
		surface = fbo->stencil_attachment.surface;
	}
	else if (attachment == &fbo->stencil_attachment
	 && attachment->type == fbo->depth_attachment.type
	 && attachment->image == fbo->depth_attachment.image
	 && !fbo->depth_attachment.dirty)
	{
		surface = fbo->depth_attachment.surface;
	}
	else if (create_info.image)
	{
		ret = ctx->jkg_op->surface_alloc(ctx->jkg_ctx,
		                                 &create_info,
		                                 &surface);
		if (ret)
		{
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return 1;
		}
	}
	else
	{
		surface = NULL;
	}
	if (attachment->surface)
	{
		if (!(attachment == &fbo->depth_attachment
		   && fbo->stencil_attachment.surface == attachment->surface)
		 && !(attachment == &fbo->stencil_attachment
		   && fbo->depth_attachment.surface == attachment->surface))
			ctx->jkg_op->surface_free(ctx->jkg_ctx, attachment->surface);
	}
	attachment->surface = surface;
	attachment->dirty = false;
	fbo->dirty = true;
	return 0;
}

int
framebuffer_commit(struct gl_ctx *ctx, struct framebuffer *fbo)
{
	for (size_t i = 0; i < ctx->jkg_caps->max_color_attachments; ++i)
	{
		if (attachment_commit(ctx, fbo, &fbo->color_attachments[i]))
			return 1;
	}
	if (attachment_commit(ctx, fbo, &fbo->depth_attachment))
		return 1;
	if (attachment_commit(ctx, fbo, &fbo->stencil_attachment))
		return 1;
	return 0;
}

void
glGenFramebuffers(GLsizei n, GLuint *ids)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;
	GLsizei i;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (i = 0; i < n; ++i)
	{
		fbo = framebuffer_alloc(ctx);
		if (!fbo)
			goto err;
		if (!object_alloc(ctx, &fbo->object))
		{
			framebuffer_free(ctx, fbo);
			goto err;
		}
		ids[i] = fbo->object.id;
	}
	return;

err:
	for (GLsizei j = 0; j < i; ++j)
	{
		fbo = object_get(ctx, ids[j], OBJECT_FRAMEBUFFER);
		if (!fbo)
			continue;
		framebuffer_free(ctx, fbo);
	}
}

void
glCreateFramebuffers(GLsizei n, GLuint *framebuffers)
{
	glGenFramebuffers(n, framebuffers); /* XXX more? */
}

void
framebuffer_free(struct gl_ctx *ctx,
                 struct framebuffer *fbo)
{
	if (fbo == ctx->draw_fbo)
		ctx->draw_fbo = ctx->default_fbo;
	if (fbo == ctx->read_fbo)
		ctx->read_fbo = ctx->default_fbo;
	for (size_t i = 0; i < ctx->jkg_caps->max_color_attachments; ++i)
	{
		struct framebuffer_attachment *attachment = &fbo->color_attachments[i];
		if (attachment->surface)
			ctx->jkg_op->surface_free(ctx->jkg_ctx, attachment->surface);
	}
	if (fbo->depth_attachment.surface)
		ctx->jkg_op->surface_free(ctx->jkg_ctx, fbo->depth_attachment.surface);
	if (fbo->stencil_attachment.surface
	 && fbo->stencil_attachment.surface != fbo->depth_attachment.surface)
		ctx->jkg_op->surface_free(ctx->jkg_ctx, fbo->stencil_attachment.surface);
	object_free(ctx, &fbo->object);
}

void
glDeleteFramebuffers(GLsizei n, GLuint *framebuffers)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		GLuint id = framebuffers[i];
		struct framebuffer *fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			continue;
		framebuffer_free(ctx, fbo);
	}
}

GLboolean
glIsFramebuffer(GLuint id)
{
	return object_is(g_ctx, id, OBJECT_FRAMEBUFFER);
}

void
glBindFramebuffer(GLenum target, GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	switch (target)
	{
		case GL_READ_FRAMEBUFFER:
			ctx->read_fbo = fbo;
			break;
		case GL_FRAMEBUFFER:
		case GL_DRAW_FRAMEBUFFER:
			ctx->draw_fbo = fbo;
			ctx->dirty |= GL_CTX_DIRTY_DRAW_FBO;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

static void
framebuffer_attachment(struct gl_ctx *ctx,
                       struct framebuffer *fbo,
                       GLenum attachment,
                       GLenum type,
                       struct object *object)
{
	if (fbo == ctx->default_fbo)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	/* XXX test that image is in a valid format */
	switch (attachment)
	{
		case GL_DEPTH_STENCIL_ATTACHMENT:
			if (fbo->depth_attachment.object != object)
			{
				fbo->depth_attachment.type = type;
				fbo->depth_attachment.object = object;
				fbo->depth_attachment.dirty = true;
			}
			if (fbo->stencil_attachment.object != object)
			{
				fbo->stencil_attachment.type = type;
				fbo->stencil_attachment.object = object;
				fbo->stencil_attachment.dirty = true;
			}
			break;
		case GL_DEPTH_ATTACHMENT:
			if (fbo->depth_attachment.object != object)
			{
				fbo->depth_attachment.type = type;
				fbo->depth_attachment.object = object;
				fbo->depth_attachment.dirty = true;
			}
			break;
		case GL_STENCIL_ATTACHMENT:
			if (fbo->stencil_attachment.object != object)
			{
				fbo->stencil_attachment.type = type;
				fbo->stencil_attachment.object = object;
				fbo->stencil_attachment.dirty = true;
			}
			break;
		case GL_COLOR_ATTACHMENT0:
		case GL_COLOR_ATTACHMENT1:
		case GL_COLOR_ATTACHMENT2:
		case GL_COLOR_ATTACHMENT3:
		case GL_COLOR_ATTACHMENT4:
		case GL_COLOR_ATTACHMENT5:
		case GL_COLOR_ATTACHMENT6:
		case GL_COLOR_ATTACHMENT7:
		case GL_COLOR_ATTACHMENT8:
		case GL_COLOR_ATTACHMENT9:
		case GL_COLOR_ATTACHMENT10:
		case GL_COLOR_ATTACHMENT11:
		case GL_COLOR_ATTACHMENT12:
		case GL_COLOR_ATTACHMENT13:
		case GL_COLOR_ATTACHMENT14:
		case GL_COLOR_ATTACHMENT15:
		case GL_COLOR_ATTACHMENT16:
		case GL_COLOR_ATTACHMENT17:
		case GL_COLOR_ATTACHMENT18:
		case GL_COLOR_ATTACHMENT19:
		case GL_COLOR_ATTACHMENT20:
		case GL_COLOR_ATTACHMENT21:
		case GL_COLOR_ATTACHMENT22:
		case GL_COLOR_ATTACHMENT23:
		case GL_COLOR_ATTACHMENT24:
		case GL_COLOR_ATTACHMENT25:
		case GL_COLOR_ATTACHMENT26:
		case GL_COLOR_ATTACHMENT27:
		case GL_COLOR_ATTACHMENT28:
		case GL_COLOR_ATTACHMENT29:
		case GL_COLOR_ATTACHMENT30:
		case GL_COLOR_ATTACHMENT31:
		{
			if (attachment - GL_COLOR_ATTACHMENT0 >= ctx->jkg_caps->max_color_attachments)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			if (fbo->color_attachments[attachment - GL_COLOR_ATTACHMENT0].object != object)
			{
				fbo->color_attachments[attachment - GL_COLOR_ATTACHMENT0].type = type;
				fbo->color_attachments[attachment - GL_COLOR_ATTACHMENT0].object = object;
				fbo->color_attachments[attachment - GL_COLOR_ATTACHMENT0].dirty = true;
			}
			break;
		}
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

static void
framebuffer_texture(struct gl_ctx *ctx,
                    struct framebuffer *fbo,
                    GLenum attachment,
                    GLuint texture_id,
                    GLint level,
                    GLint layer)
{
	struct texture *texture;

	if (texture_id)
	{
		texture = object_get(ctx, texture_id, OBJECT_TEXTURE);
		if (!texture)
			return;
	}
	else
	{
		texture = NULL;
	}
	/* XXX */
	(void)layer;
	(void)level;
	framebuffer_attachment(ctx,
	                       fbo,
	                       attachment,
	                       GL_TEXTURE,
	                       texture ? &texture->object : NULL);
}

void
glFramebufferTexture(GLenum target,
                     GLenum attachment,
                     GLuint texture,
                     GLint level)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (!get_fbo(ctx, target, &fbo))
		return;
	if (fbo == ctx->default_fbo)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	framebuffer_texture(ctx, fbo, attachment, texture, level, 0);
}

void
glFramebufferTexture1D(GLenum target,
                       GLenum attachment,
                       GLenum textarget,
                       GLuint texture,
                       GLint level)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (!get_fbo(ctx, target, &fbo))
		return;
	if (fbo == ctx->default_fbo)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	framebuffer_texture(ctx, fbo, attachment, texture, level, 0);
}

void
glFramebufferTexture2D(GLenum target,
                       GLenum attachment,
                       GLenum textarget,
                       GLuint texture,
                       GLint level)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (!get_fbo(ctx, target, &fbo))
		return;
	if (fbo == ctx->default_fbo)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	framebuffer_texture(ctx, fbo, attachment, texture, level, 0);
}

void
glFramebufferTexture3D(GLenum target,
                       GLenum attachment,
                       GLenum textarget,
                       GLuint texture,
                       GLint level,
                       GLint layer)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (!get_fbo(ctx, target, &fbo))
		return;
	if (fbo == ctx->default_fbo)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	framebuffer_texture(ctx, fbo, attachment, texture, level, layer);
}

void
glNamedFramebufferTexture(GLuint id,
                          GLenum attachment,
                          GLuint texture,
                          GLint level)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (!id)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
	if (!fbo)
		return;
	framebuffer_texture(ctx, fbo, attachment, texture, level, 0);
}

static void
framebuffer_renderbuffer(struct gl_ctx *ctx,
                         struct framebuffer *fbo,
                         GLenum attachment,
                         GLuint renderbuffer_id)
{
	struct renderbuffer *renderbuffer;

	if (!renderbuffer_id)
	{
		renderbuffer = object_get(ctx, renderbuffer_id, OBJECT_RENDERBUFFER);
		if (!renderbuffer)
			return;
	}
	else
	{
		renderbuffer = NULL;
	}
	framebuffer_attachment(ctx,
	                       fbo,
	                       attachment,
	                       GL_RENDERBUFFER,
	                       renderbuffer ? &renderbuffer->object : NULL);
}

void
glFramebufferRenderbuffer(GLenum target,
                          GLenum attachment,
                          GLenum renderbuffertarget,
                          GLuint renderbuffer)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (renderbuffertarget != GL_RENDERBUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!get_fbo(ctx, target, &fbo))
		return;
	if (fbo == ctx->default_fbo)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	framebuffer_renderbuffer(ctx, fbo, attachment, renderbuffer);
}

void
glNamedFramebufferRenderbuffer(GLuint id,
                               GLenum attachment,
                               GLenum renderbuffertarget,
                               GLuint renderbuffer)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (renderbuffertarget != GL_RENDERBUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!id)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
	if (!fbo)
		return;
	framebuffer_renderbuffer(ctx, fbo, attachment, renderbuffer);
}

static void
get_color_attachment(struct gl_ctx *ctx,
                     struct framebuffer *fbo,
                     GLint drawbuffer,
                     struct framebuffer_attachment **attachmentp)
{
	if (fbo == ctx->default_fbo)
	{
		switch (fbo->draw_buffers[drawbuffer])
		{
			case GL_FRONT:
			case GL_FRONT_LEFT:
			case GL_FRONT_RIGHT:
			case GL_LEFT:
			case GL_RIGHT:
				*attachmentp = &fbo->color_attachments[0];
				return;
			case GL_BACK:
			case GL_BACK_LEFT:
			case GL_BACK_RIGHT:
				*attachmentp = &fbo->color_attachments[1];
				return;
			default:
				assert(!"unknown draw buffer");
				/* FALLTHROUGH */
			case GL_NONE:
				*attachmentp = NULL;
				return;
		}
	}
	if (fbo->draw_buffers[drawbuffer] == GL_NONE)
	{
		*attachmentp = NULL;
		return;
	}
	*attachmentp = &fbo->color_attachments[fbo->draw_buffers[drawbuffer] - GL_COLOR_ATTACHMENT0];
}

static void
clear_buffer_colorf(struct gl_ctx *ctx,
                    struct framebuffer *fbo,
                    GLint drawbuffer,
                    const GLfloat *color)
{
	struct framebuffer_attachment *attachment;
	enum jkg_format_type format_type;
	enum jkg_format format;

	if (fbo->draw_buffers[drawbuffer] == GL_NONE)
		return;
	if (framebuffer_commit(ctx, fbo))
		return;
	get_color_attachment(ctx, fbo, drawbuffer, &attachment);
	if (!attachment->surface)
		return;
	format_type = jkg_get_format_type(attachment->format);
	if (format_type != JKG_FORMAT_TYPE_COLOR_UNORM
	 && format_type != JKG_FORMAT_TYPE_COLOR_SNORM
	 && format_type != JKG_FORMAT_TYPE_COLOR_USCALED
	 && format_type != JKG_FORMAT_TYPE_COLOR_SSCALED
	 && format_type != JKG_FORMAT_TYPE_COLOR_SFLOAT)
		return;
	if (ctx->jkg_caps->caps1 & JKG_CAPS1_SURFACE_CLEAR)
	{
		struct jkg_clear_cmd cmd;
		int ret;

		cmd.mask = JKG_CLEAR_COLOR;
		for (size_t i = 0; i < 4; ++i)
			cmd.color[i] = color[i];
		ret  = ctx->jkg_op->surface_clear(ctx->jkg_ctx,
		                                  attachment->surface,
		                                  &cmd);
		if (ret)
		{
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return;
		}
		return;
	}
	/* XXX
	 * bind "clear fbo"
	 * set fbo surface
	 * set draw buffer
	 * glClear()
	 * restore framebuffer
	 */
}

static void
clear_buffer_coloru(struct gl_ctx *ctx,
                    struct framebuffer *fbo,
                    GLuint drawbuffer,
                    const GLuint *color)
{
	if (fbo->draw_buffers[drawbuffer] == GL_NONE)
		return;
	/* XXX */
}

static void
clear_buffer_colori(struct gl_ctx *ctx,
                    struct framebuffer *fbo,
                    GLint drawbuffer,
                    const GLint *color)
{
	if (fbo->draw_buffers[drawbuffer] == GL_NONE)
		return;
	/* XXX */
}

static void
clear_buffer_depth(struct gl_ctx *ctx,
                   struct framebuffer *fbo,
                   GLfloat depth)
{
	struct framebuffer_attachment *attachment;
	enum jkg_format_type format_type;
	enum jkg_format format;

	attachment = &fbo->depth_attachment;
	if (attachment_commit(ctx, fbo, attachment))
		return;
	if (!attachment->surface)
		return;
	format_type = jkg_get_format_type(attachment->format);
	if (format_type != JKG_FORMAT_TYPE_DEPTH_SFLOAT
	 && format_type != JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT)
		return;
	if (ctx->jkg_caps->caps1 & JKG_CAPS1_SURFACE_CLEAR)
	{
		struct jkg_clear_cmd cmd;
		int ret;

		cmd.mask = JKG_CLEAR_DEPTH;
		cmd.depth = depth;
		ret  = ctx->jkg_op->surface_clear(ctx->jkg_ctx,
		                                  attachment->surface,
		                                  &cmd);
		if (ret)
		{
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return;
		}
		return;
	}
	/* XXX
	 * bind "clear fbo"
	 * set fbo surface
	 * set draw buffer
	 * glClear()
	 * restore framebuffer
	 */
}

static void
clear_buffer_stencil(struct gl_ctx *ctx,
                     struct framebuffer *fbo,
                     GLint stencil)
{
	struct framebuffer_attachment *attachment;
	enum jkg_format_type format_type;
	enum jkg_format format;

	attachment = &fbo->depth_attachment;
	if (attachment_commit(ctx, fbo, attachment))
		return;
	if (!attachment->surface)
		return;
	format_type = jkg_get_format_type(attachment->format);
	if (format_type != JKG_FORMAT_TYPE_STENCIL_UINT
	 && format_type != JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT)
		return;
	if (ctx->jkg_caps->caps1 & JKG_CAPS1_SURFACE_CLEAR)
	{
		struct jkg_clear_cmd cmd;
		int ret;

		cmd.mask = JKG_CLEAR_STENCIL;
		cmd.stencil = stencil;
		ret  = ctx->jkg_op->surface_clear(ctx->jkg_ctx,
		                                  attachment->surface,
		                                  &cmd);
		if (ret)
		{
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return;
		}
		return;
	}
	/* XXX
	 * bind "clear fbo"
	 * set fbo surface
	 * set draw buffer
	 * glClear()
	 * restore framebuffer
	 */
}

static void
clear_buffer_depth_stencil(struct gl_ctx *ctx,
                           struct framebuffer *fbo,
                           GLfloat depth,
                           GLint stencil)
{
	struct framebuffer_attachment *attachment;
	enum jkg_format_type format_type;
	enum jkg_format format;

	attachment = &fbo->depth_attachment;
	if (attachment_commit(ctx, fbo, attachment))
		return;
	if (!attachment->surface)
		return;
	format_type = jkg_get_format_type(attachment->format);
	if (format_type != JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT)
		return;
	if (ctx->jkg_caps->caps1 & JKG_CAPS1_SURFACE_CLEAR)
	{
		struct jkg_clear_cmd cmd;
		int ret;

		cmd.mask = JKG_CLEAR_DEPTH | JKG_CLEAR_STENCIL;
		cmd.depth = depth;
		cmd.stencil = stencil;
		ret  = ctx->jkg_op->surface_clear(ctx->jkg_ctx,
		                                  attachment->surface,
		                                  &cmd);
		if (ret)
		{
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return;
		}
		return;
	}
	/* XXX
	 * bind "clear fbo"
	 * set fbo surface
	 * set draw buffer
	 * glClear()
	 * restore framebuffer
	 */
}

static bool
is_valid_drawbuffer(struct gl_ctx *ctx,
                    GLenum buffer,
                    GLint drawbuffer)
{
	if (buffer != GL_COLOR)
		return drawbuffer == 0;
	if (drawbuffer < 0)
		return false;
	if ((GLuint)drawbuffer >= ctx->jkg_caps->max_draw_buffers)
		return false;
	return true;
}

static void
clear_bufferiv(struct gl_ctx *ctx,
               struct framebuffer *fbo,
               GLenum buffer,
               GLint drawbuffer,
               const GLint *value)
{
	if (!is_valid_drawbuffer(ctx, buffer, drawbuffer))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	switch (buffer)
	{
		case GL_COLOR:
			clear_buffer_colori(ctx, fbo, drawbuffer, value);
			break;
		case GL_STENCIL:
			clear_buffer_stencil(ctx, fbo, value[0]);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glClearBufferiv(GLenum buffer,
                GLint drawbuffer,
                const GLint *value)
{
	struct gl_ctx *ctx = g_ctx;

	clear_bufferiv(ctx, ctx->draw_fbo, buffer, drawbuffer, value);
}

void
glClearNamedFramebufferiv(GLuint id,
                          GLenum buffer,
                          GLint drawbuffer,
                          const GLint *value)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	clear_bufferiv(ctx, fbo, buffer, drawbuffer, value);
}

static void
clear_bufferuiv(struct gl_ctx *ctx,
                struct framebuffer *fbo,
                GLenum buffer,
                GLint drawbuffer,
                const GLuint *value)
{
	if (!is_valid_drawbuffer(ctx, buffer, drawbuffer))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	switch (buffer)
	{
		case GL_COLOR:
			clear_buffer_coloru(ctx, fbo, drawbuffer, value);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glClearBufferuiv(GLenum buffer,
                 GLint drawbuffer,
                 const GLuint *value)
{
	struct gl_ctx *ctx = g_ctx;

	clear_bufferuiv(ctx, ctx->draw_fbo, buffer, drawbuffer, value);
}

void
glClearNamedFramebufferuiv(GLuint id,
                           GLenum buffer,
                           GLint drawbuffer,
                           const GLuint *value)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	clear_bufferuiv(ctx, fbo, buffer, drawbuffer, value);
}

static void
clear_bufferfv(struct gl_ctx *ctx,
               struct framebuffer *fbo,
               GLenum buffer,
               GLint drawbuffer,
               const GLfloat *value)
{
	if (!is_valid_drawbuffer(ctx, buffer, drawbuffer))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	switch (buffer)
	{
		case GL_COLOR:
			clear_buffer_colorf(ctx, fbo, drawbuffer, value);
			break;
		case GL_DEPTH:
			clear_buffer_depth(ctx, fbo, value[0]);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glClearBufferfv(GLenum buffer,
                GLint drawbuffer,
                const GLfloat *value)
{
	struct gl_ctx *ctx = g_ctx;

	clear_bufferfv(ctx, ctx->draw_fbo, buffer, drawbuffer, value);
}

void
glClearNamedFramebufferfv(GLuint id,
                          GLenum buffer,
                          GLint drawbuffer,
                          const GLfloat *value)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	clear_bufferfv(ctx, fbo, buffer, drawbuffer, value);
}

static void
clear_bufferfi(struct gl_ctx *ctx,
               struct framebuffer *fbo,
               GLenum buffer,
               GLint drawbuffer,
               GLfloat depth,
               GLint stencil)
{
	if (!is_valid_drawbuffer(ctx, buffer, drawbuffer))
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	switch (buffer)
	{
		case GL_DEPTH_STENCIL:
			clear_buffer_depth_stencil(ctx, fbo, depth, stencil);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glClearBufferfi(GLenum buffer,
                GLint drawbuffer,
                GLfloat depth,
                GLint stencil)
{
	struct gl_ctx *ctx = g_ctx;

	clear_bufferfi(ctx, ctx->draw_fbo, buffer, drawbuffer, depth, stencil);
}

void
glClearNamedFramebufferfi(GLuint id,
                          GLenum buffer,
                          GLint drawbuffer,
                          GLfloat depth,
                          GLint stencil)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	clear_bufferfi(ctx, fbo, buffer, drawbuffer, depth, stencil);
}

static bool
is_valid_draw_buffer(struct gl_ctx *ctx,
                     struct framebuffer *fbo,
                     GLenum buf)
{
	if (buf == GL_NONE)
		return true;
	if (fbo == ctx->default_fbo)
	{
		switch (buf)
		{
			case GL_FRONT_LEFT:
			case GL_FRONT_RIGHT:
			case GL_BACK_LEFT:
			case GL_BACK_RIGHT:
			case GL_FRONT:
			case GL_BACK:
			case GL_LEFT:
			case GL_RIGHT:
			case GL_FRONT_AND_BACK:
				return true;
			default:
				return false;
		}
	}
	if (buf < GL_COLOR_ATTACHMENT0
	 || buf >= GL_COLOR_ATTACHMENT0 + ctx->jkg_caps->max_color_attachments)
		return false;
	return true;
}

static void
draw_buffers(struct gl_ctx *ctx,
             struct framebuffer *fbo,
             GLsizei n,
             const GLenum *bufs)
{
	if (n < 0
	 || (size_t)n > ctx->jkg_caps->max_draw_buffers)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		if (!is_valid_draw_buffer(ctx, fbo, bufs[i]))
		{
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
		}
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		if (fbo->draw_buffers[i] == bufs[i])
			continue;
		fbo->draw_buffers[i] = bufs[i];
		fbo->dirty = true;
	}
}

void
glDrawBuffers(GLsizei n, const GLenum *bufs)
{
	struct gl_ctx *ctx = g_ctx;

	draw_buffers(ctx, ctx->draw_fbo, n, bufs);
}

void
glNamedFramebufferDrawBuffers(GLuint id,
                              GLsizei n,
                              const GLenum *bufs)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	draw_buffers(ctx, fbo, n, bufs);
}

static void
draw_buffer(struct gl_ctx *ctx,
            struct framebuffer *fbo,
            GLenum buf)
{
	if (!is_valid_draw_buffer(ctx, fbo, buf))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (fbo->draw_buffers[0] != buf)
	{
		fbo->draw_buffers[0] = buf;
		fbo->dirty = true;
	}
	for (size_t i = 1; i < ctx->jkg_caps->max_draw_buffers; ++i)
	{
		if (fbo->draw_buffers[i] == GL_NONE)
			continue;
		fbo->draw_buffers[i] = GL_NONE;
		fbo->dirty = true;
	}
}

void
glDrawBuffer(GLenum buf)
{
	struct gl_ctx *ctx = g_ctx;

	draw_buffer(ctx, ctx->draw_fbo, buf);
}

void
glNamedFramebufferDrawBuffer(GLuint id, GLenum buf)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	draw_buffer(ctx, fbo, buf);
}

static void
read_buffer(struct gl_ctx *ctx,
            struct framebuffer *fbo,
            GLenum buf)
{
	if (fbo == ctx->default_fbo)
	{
		switch (buf)
		{
			case GL_FRONT_LEFT:
			case GL_FRONT_RIGHT:
			case GL_BACK_LEFT:
			case GL_BACK_RIGHT:
			case GL_FRONT:
			case GL_BACK:
			case GL_LEFT:
			case GL_RIGHT:
				break;
			default:
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
		}
	}
	else
	{
		if (buf < GL_COLOR_ATTACHMENT0
		 || buf >= GL_COLOR_ATTACHMENT0 + ctx->jkg_caps->max_color_attachments)
		{
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
		}
	}
	fbo->read_buffer = buf;
}

void
glReadBuffer(GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	read_buffer(ctx, ctx->read_fbo, mode);
}

void
glNamedFramebufferReadBuffer(GLuint id, GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	read_buffer(ctx, fbo, mode);
}

static GLenum
check_framebuffer_status(struct gl_ctx *ctx,
                         struct framebuffer *fbo,
                         GLenum target)
{
	/* XXX */
	return 0;
}

GLenum
glCheckFramebufferStatus(GLenum target)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (!get_fbo(ctx, target, &fbo))
		return 0;
	return check_framebuffer_status(ctx, fbo, target);
}

GLenum
glCheckNamedFramebufferStatus(GLuint id, GLenum target)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return 0;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	return check_framebuffer_status(ctx, fbo, target);
}

static void
get_framebuffer_attachment_parameteriv(struct gl_ctx *ctx,
                                       struct framebuffer *fbo,
                                       GLenum attachment_id,
                                       GLenum pname,
                                       GLint *params)
{
	struct framebuffer_attachment *attachment;

	if (fbo == ctx->default_fbo)
	{
		switch (attachment_id)
		{
			case GL_FRONT_LEFT:
			case GL_FRONT_RIGHT:
				attachment = &fbo->color_attachments[0];
				break;
			case GL_BACK_LEFT:
			case GL_BACK_RIGHT:
				attachment = &fbo->color_attachments[1];
				break;
			case GL_DEPTH:
				attachment = &fbo->depth_attachment;
				break;
			case GL_STENCIL:
				attachment = &fbo->stencil_attachment;
				break;
			default:
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
		}
	}
	else
	{
		switch (attachment_id)
		{
			case GL_DEPTH:
				attachment = &fbo->depth_attachment;
				break;
			case GL_STENCIL:
				attachment = &fbo->stencil_attachment;
				break;
			default:
				if (attachment_id < GL_COLOR_ATTACHMENT0
				 || attachment_id >= GL_COLOR_ATTACHMENT0 + ctx->jkg_caps->max_color_attachments)
				{
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
				}
				attachment = &fbo->color_attachments[attachment_id - GL_COLOR_ATTACHMENT0];
				break;
		}
	}
	switch (pname)
	{
		case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
			*params = attachment->type;
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
			switch (attachment->type)
			{
				default:
					assert(!"unknown attachment type");
					/* FALLTHROUGH */
				case GL_NONE:
					*params = 0;
					break;
				case GL_TEXTURE:
					*params = attachment->texture ? attachment->texture->object.id : 0;
					break;
				case GL_RENDERBUFFER:
					*params = attachment->renderbuffer ? attachment->renderbuffer->object.id : 0;
					break;
				case GL_FRAMEBUFFER_DEFAULT:
					*params = 0;
					break;
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
			if (attachment_commit(ctx, fbo, attachment))
				return;
			*params = jkg_get_red_size(attachment->format);
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
			if (attachment_commit(ctx, fbo, attachment))
				return;
			*params = jkg_get_green_size(attachment->format);
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
			if (attachment_commit(ctx, fbo, attachment))
				return;
			*params = jkg_get_blue_size(attachment->format);
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
			if (attachment_commit(ctx, fbo, attachment))
				return;
			*params = jkg_get_alpha_size(attachment->format);
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
			if (attachment_commit(ctx, fbo, attachment))
				return;
			*params = jkg_get_depth_size(attachment->format);
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
			if (attachment_commit(ctx, fbo, attachment))
				return;
			*params = jkg_get_stencil_size(attachment->format);
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
			if (attachment_commit(ctx, fbo, attachment))
				return;
			switch (jkg_get_format_type(attachment->format))
			{
				default:
					assert(!"unknown format type");
					/* FALLTHROUGH */
				case JKG_FORMAT_TYPE_UNKNOWN:
					*params = GL_NONE;
					break;
				case JKG_FORMAT_TYPE_COLOR_UINT:
					*params = GL_UNSIGNED_INT;
					break;
				case JKG_FORMAT_TYPE_COLOR_SINT:
					*params = GL_INT;
					break;
				case JKG_FORMAT_TYPE_COLOR_UNORM:
					*params = GL_UNSIGNED_NORMALIZED;
					break;
				case JKG_FORMAT_TYPE_COLOR_SNORM:
					*params = GL_SIGNED_NORMALIZED;
					break;
				case JKG_FORMAT_TYPE_COLOR_USCALED:
					*params = GL_UNSIGNED_INT;
					break;
				case JKG_FORMAT_TYPE_COLOR_SSCALED:
					*params = GL_INT;
					break;
				case JKG_FORMAT_TYPE_COLOR_SFLOAT:
					*params = GL_FLOAT;
					break;
				case JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK:
				case JKG_FORMAT_TYPE_COLOR_SNORM_BLOCK:
					*params = GL_NONE;
					break;
				case JKG_FORMAT_TYPE_DEPTH_SFLOAT:
					*params = GL_FLOAT;
					break;
				case JKG_FORMAT_TYPE_STENCIL_UINT:
					*params = GL_UNSIGNED_INT;
					break;
				case JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT:
					*params = GL_NONE;
					break;
			}
			break;
		case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
			*params = GL_LINEAR; /* XXX sRGB */
			break;
		/* XXX
		 * GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL
		 * GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE
		 * GL_FRAMEBUFFER_ATTACHMENT_LAYERED
		 * GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER
		 */
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glGetFramebufferAttachmentParameteriv(GLenum target,
                                      GLenum attachment,
                                      GLenum pname,
                                      GLint *params)
{
	struct framebuffer *fbo;
	struct gl_ctx *ctx = g_ctx;

	if (!get_fbo(ctx, target, &fbo))
		return;
	get_framebuffer_attachment_parameteriv(ctx,
	                                       fbo,
	                                       attachment,
	                                       pname,
	                                       params);
}

void
glGetNamedFramebufferAttachmentParameteriv(GLuint id,
                                           GLenum attachment,
                                           GLenum pname,
                                           GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	struct framebuffer *fbo;

	if (id)
	{
		fbo = object_get(ctx, id, OBJECT_FRAMEBUFFER);
		if (!fbo)
			return;
	}
	else
	{
		fbo = ctx->default_fbo;
	}
	get_framebuffer_attachment_parameteriv(ctx,
	                                       fbo,
	                                       attachment,
	                                       pname,
	                                       params);
}
