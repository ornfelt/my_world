#include "internal.h"

#include <string.h>
#include <assert.h>

static void
read_attachment(struct gl_ctx *ctx,
                struct framebuffer_attachment *attachment,
                void *data,
                enum jkg_format format,
                const struct jkg_extent *size,
                const struct jkg_extent *offset)
{
	struct jkg_image *image;
	int ret;

	switch (attachment->type)
	{
		default:
			assert(!"invalid attachment type");
			/* FALLTHROUGH */
		case GL_NONE:
			image = NULL;
			break;
		case GL_FRAMEBUFFER_DEFAULT:
			image = attachment->image;
			break;
		case GL_TEXTURE:
			image = attachment->texture->image;
			break;
		case GL_RENDERBUFFER:
			image = attachment->renderbuffer->image;
			break;
	}
	if (!image)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	ret = ctx->jkg_op->image_read(ctx->jkg_ctx,
	                              image,
	                              0, /* XXX image level */
	                              data,
	                              format,
	                              size,
	                              offset);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glReadPixels(GLint x,
             GLint y,
             GLsizei width,
             GLsizei height,
             GLenum format,
             GLenum type,
             GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct jkg_extent offset;
	struct jkg_extent size;
	enum jkg_format jkg_format;

	if (width < 0
	 || height < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	offset.x = x;
	offset.y = y;
	offset.z = 0;
	size.x = width;
	size.y = height;
	size.z = 0;
	switch (format)
	{
		case GL_STENCIL_INDEX:
			if (type != GL_UNSIGNED_BYTE)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			jkg_format = JKG_FORMAT_S8_UINT;
			break;
		case GL_DEPTH_COMPONENT:
			if (type != GL_FLOAT)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			jkg_format = JKG_FORMAT_D32_SFLOAT;
			break;
		case GL_DEPTH_STENCIL:
			if (type != GL_UNSIGNED_INT_24_8)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
			}
			jkg_format = JKG_FORMAT_D24_UNORM_S8_UINT;
			break;
		case GL_RED:
			switch (type)
			{
				case GL_BYTE:
					jkg_format = JKG_FORMAT_R8_SNORM;
					break;
				case GL_UNSIGNED_BYTE:
					jkg_format = JKG_FORMAT_R8_UNORM;
					break;
				case GL_SHORT:
					jkg_format = JKG_FORMAT_R16_SNORM;
					break;
				case GL_UNSIGNED_SHORT:
					jkg_format = JKG_FORMAT_R16_UNORM;
					break;
				case GL_FLOAT:
					jkg_format = JKG_FORMAT_R32_SFLOAT;
					break;
				default:
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
			}
			break;
		case GL_GREEN:
			/* XXX */
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
		case GL_BLUE:
			/* XXX */
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
		case GL_RGB:
			switch (type)
			{
				case GL_BYTE:
					jkg_format = JKG_FORMAT_R8G8B8_SNORM;
					break;
				case GL_UNSIGNED_BYTE:
					jkg_format = JKG_FORMAT_R8G8B8_UNORM;
					break;
				case GL_SHORT:
					jkg_format = JKG_FORMAT_R16G16B16_SNORM;
					break;
				case GL_UNSIGNED_SHORT:
					jkg_format = JKG_FORMAT_R16G16B16_UNORM;
					break;
				case GL_FLOAT:
					jkg_format = JKG_FORMAT_R32G32B32_SFLOAT;
					break;
				case GL_UNSIGNED_SHORT_5_6_5:
					jkg_format = JKG_FORMAT_R5G6B5_UNORM_PACK16;
					break;
				case GL_UNSIGNED_SHORT_5_6_5_REV:
					jkg_format = JKG_FORMAT_B5G6R5_UNORM_PACK16;
					break;
				default:
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
			}
			break;
		case GL_BGR:
			/* XXX */
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
		case GL_RGBA:
			switch (type)
			{
				case GL_BYTE:
					jkg_format = JKG_FORMAT_R8G8B8A8_SNORM;
					break;
				case GL_UNSIGNED_BYTE:
					jkg_format = JKG_FORMAT_R8G8B8A8_UNORM;
					break;
				case GL_SHORT:
					jkg_format = JKG_FORMAT_R16G16B16A16_SNORM;
					break;
				case GL_UNSIGNED_SHORT:
					jkg_format = JKG_FORMAT_R16G16B16A16_UNORM;
					break;
				case GL_FLOAT:
					jkg_format = JKG_FORMAT_R32G32B32A32_SFLOAT;
					break;
				case GL_UNSIGNED_SHORT_4_4_4_4:
					jkg_format = JKG_FORMAT_R4G4B4A4_UNORM_PACK16;
					break;
				case GL_UNSIGNED_SHORT_4_4_4_4_REV:
					jkg_format = JKG_FORMAT_A4B4G4R4_UNORM_PACK16;
					break;
				case GL_UNSIGNED_SHORT_5_5_5_1:
					jkg_format = JKG_FORMAT_R5G5B5A1_UNORM_PACK16;
					break;
				case GL_UNSIGNED_SHORT_1_5_5_5_REV:
					jkg_format = JKG_FORMAT_A1B5G5R5_UNORM_PACK16;
					break;
				default:
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
			}
			break;
		case GL_BGRA:
			/* XXX */
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	switch (format)
	{
		case GL_STENCIL_INDEX:
			read_attachment(ctx,
			                &ctx->read_fbo->stencil_attachment,
			                data,
			                jkg_format,
			                &size,
			                &offset);
			break;
		case GL_DEPTH_COMPONENT:
			read_attachment(ctx,
			                &ctx->read_fbo->depth_attachment,
			                data,
			                jkg_format,
			                &size,
			                &offset);
			break;
		case GL_DEPTH_STENCIL:
			read_attachment(ctx,
			                &ctx->read_fbo->stencil_attachment,
			                data,
			                jkg_format,
			                &size,
			                &offset);
			read_attachment(ctx,
			                &ctx->read_fbo->depth_attachment,
			                data,
			                jkg_format,
			                &size,
			                &offset);
			break;
		default:
		{
			size_t id;
			if (ctx->read_fbo == ctx->default_fbo)
			{
				switch (ctx->read_fbo->read_buffer)
				{
					default:
						assert(!"unknown read buffer");
						/* FALLTHROUGH */
					case GL_FRONT:
					case GL_FRONT_LEFT:
					case GL_FRONT_RIGHT:
					case GL_LEFT:
					case GL_RIGHT:
						id = 0;
						break;
					case GL_BACK:
					case GL_BACK_LEFT:
					case GL_BACK_RIGHT:
						id = 1;
						break;
				}
			}
			else
			{
				id = ctx->read_fbo->read_buffer - GL_COLOR_ATTACHMENT0;
			}
			read_attachment(ctx,
			                &ctx->read_fbo->color_attachments[id],
			                data,
			                jkg_format,
			                &size,
			                &offset);
			break;
		}
	}
}
