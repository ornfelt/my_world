#include "internal.h"

#include <assert.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>

#define DRAW_DIRTY_MASK (GL_CTX_DIRTY_BLEND_COLOR \
                       | GL_CTX_DIRTY_VIEWPORT \
                       | GL_CTX_DIRTY_SCISSOR \
                       | GL_CTX_DIRTY_DEPTH_STENCIL_STATE \
                       | GL_CTX_DIRTY_RASTERIZER_STATE \
                       | GL_CTX_DIRTY_STENCIL_REF \
                       | GL_CTX_DIRTY_BLEND_STATE \
                       | GL_CTX_DIRTY_VERTEX_ARRAY \
                       | GL_CTX_DIRTY_IMAGE_VIEWS \
                       | GL_CTX_DIRTY_SAMPLERS \
                       | GL_CTX_DIRTY_PROGRAM \
                       | GL_CTX_DIRTY_DRAW_FBO \
                       | GL_CTX_DIRTY_UNIFORM_BLOCKS) \

static bool
get_primitive(struct gl_ctx *ctx,
              GLenum mode,
              enum jkg_primitive *primitive)
{
	switch (mode)
	{
		case GL_POINTS:
			*primitive = JKG_PRIMITIVE_POINTS;
			return true;
		case GL_LINES:
			*primitive = JKG_PRIMITIVE_LINES;
			return true;
		case GL_LINE_LOOP:
			*primitive = JKG_PRIMITIVE_LINE_LOOP;
			return true;
		case GL_LINE_STRIP:
			*primitive = JKG_PRIMITIVE_LINE_STRIP;
			return true;
		case GL_TRIANGLES:
			*primitive = JKG_PRIMITIVE_TRIANGLES;
			return true;
		case GL_TRIANGLE_FAN:
			*primitive = JKG_PRIMITIVE_TRIANGLE_FAN;
			return true;
		case GL_TRIANGLE_STRIP:
			*primitive = JKG_PRIMITIVE_TRIANGLE_STRIP;
			return true;
		case GL_QUADS:
			if (!(ctx->jkg_caps->caps1 & JKG_CAPS1_DRAW_QUADS))
				return false;
			*primitive = JKG_PRIMITIVE_QUADS;
			return true;
		case GL_QUAD_STRIP:
			if (!(ctx->jkg_caps->caps1 & JKG_CAPS1_DRAW_QUADS))
				return false;
			*primitive = JKG_PRIMITIVE_QUAD_STRIP;
			return true;
		default:
			return false;
	}
}

static bool
get_index_type(GLenum type, enum jkg_index_type *typep)
{
	switch (type)
	{
		case GL_UNSIGNED_BYTE:
			*typep = JKG_INDEX_UINT8;
			return true;
		case GL_UNSIGNED_SHORT:
			*typep = JKG_INDEX_UINT16;
			return true;
		case GL_UNSIGNED_INT:
			*typep = JKG_INDEX_UINT32;
			return true;
		default:
			return false;
	}
}

static void
get_restart_index(struct gl_ctx *ctx,
                  struct jkg_draw_cmd *cmd,
                  enum jkg_index_type index_type)
{
	cmd->primitive_restart_enable = ctx->primitive_restart_enable;
	if (ctx->primitive_restart_fixed_index_enable)
	{
		switch (index_type)
		{
			case JKG_INDEX_UINT8:
				cmd->primitive_restart_index = UINT8_MAX;
				break;
			case JKG_INDEX_UINT16:
				cmd->primitive_restart_index = UINT16_MAX;
				break;
			case JKG_INDEX_UINT32:
				cmd->primitive_restart_index = UINT32_MAX;
				break;
		}
		return;
	}
	cmd->primitive_restart_index = ctx->primitive_restart_index;
}

static void
draw_elements(struct gl_ctx *ctx,
              GLenum mode,
              GLsizei count,
              GLenum type,
              const GLvoid *indices,
              GLsizei instancecount,
              GLint basevertex,
              GLuint baseinstance)
{
	struct jkg_draw_cmd cmd;
	enum jkg_index_type index_type;
	int ret;

	if (count < 0
	 || instancecount < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_primitive(ctx, mode, &cmd.primitive))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!get_index_type(type, &index_type))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!count)
		return;
	if (index_type != ctx->index_type)
	{
		ctx->index_type = index_type;
		ctx->dirty |= GL_CTX_DIRTY_INDEX_TYPE;
	}
	if (!ctx->program)
	{
		if (!fixed_update(ctx, indices, type, count))
			return;
	}
	if (gl_ctx_commit(ctx, DRAW_DIRTY_MASK | GL_CTX_DIRTY_INDEX_TYPE))
		return;
	cmd.start = ctx->program ? (uintptr_t)indices : 0;
	cmd.count = count;
	cmd.instance_start = baseinstance;
	cmd.instance_count = instancecount;
	cmd.base_vertex = basevertex;
	cmd.indexed = true;
	get_restart_index(ctx, &cmd, index_type);
	ret = ctx->jkg_op->draw(ctx->jkg_ctx, &cmd);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}
void
glDrawElements(GLenum mode,
               GLsizei count,
               GLenum type,
               const GLvoid *indices)
{
	draw_elements(g_ctx,
	              mode,
	              count,
	              type,
	              indices,
	              0,
	              0,
	              0);
}

void
glDrawElementsBaseVertex(GLenum mode,
                         GLsizei count,
                         GLenum type,
                         const GLvoid *indices,
                         GLint basevertex)
{
	draw_elements(g_ctx,
	              mode,
	              count,
	              type,
	              indices,
	              0,
	              basevertex,
	              0);
}

void
glDrawElementsInstanced(GLenum mode,
                        GLsizei count,
                        GLenum type,
                        const GLvoid *indices,
                        GLsizei instancecount)
{
	draw_elements(g_ctx,
	              mode,
	              count,
	              type,
	              indices,
	              instancecount,
	              0,
	              0);
}

void
glDrawElementsInstancedBaseVertex(GLenum mode,
                                  GLsizei count,
                                  GLenum type,
                                  const GLvoid *indices,
                                  GLsizei instancecount,
                                  GLint basevertex)
{
	draw_elements(g_ctx,
	              mode,
	              count,
	              type,
	              indices,
	              instancecount,
	              basevertex,
	              0);
}

void
glDrawElementsInstancedBaseInstance(GLenum mode,
                                    GLsizei count,
                                    GLenum type,
                                    const GLvoid *indices,
                                    GLsizei instancecount,
                                    GLuint baseinstance)
{
	draw_elements(g_ctx,
	              mode,
	              count,
	              type,
	              indices,
	              instancecount,
	              0,
	              baseinstance);
}

void
glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode,
                                              GLsizei count,
                                              GLenum type,
                                              const GLvoid *indices,
                                              GLsizei instancecount,
                                              GLint basevertex,
                                              GLuint baseinstance)
{
	draw_elements(g_ctx,
	              mode,
	              count,
	              type,
	              indices,
	              instancecount,
	              basevertex,
	              baseinstance);
}

static void
multi_draw_elements(struct gl_ctx *ctx,
                    GLenum mode,
                    const GLsizei *count,
                    GLenum type,
                    const GLvoid * const *indices,
                    GLsizei drawcount,
                    const GLint *basevertex)
{
	struct jkg_draw_cmd cmd;
	enum jkg_index_type index_type;
	int ret;

	if (drawcount < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_primitive(ctx, mode, &cmd.primitive))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!get_index_type(type, &index_type))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!drawcount)
		return;
	if (index_type != ctx->index_type)
	{
		ctx->index_type = index_type;
		ctx->dirty |= GL_CTX_DIRTY_INDEX_TYPE;
	}
	if (gl_ctx_commit(ctx, DRAW_DIRTY_MASK | GL_CTX_DIRTY_INDEX_TYPE))
		return;
	cmd.instance_start = 0;
	cmd.instance_count = 0;
	cmd.indexed = true;
	get_restart_index(ctx, &cmd, index_type);
	for (GLsizei i = 0; i < drawcount; ++i)
	{
		if (count[i] < 0)
		{
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			continue;
		}
		if (!count[i])
			continue;
		if (!ctx->program)
		{
			if (!fixed_update(ctx, indices[i], type, count[i]))
				continue;
			if (gl_ctx_commit(ctx, GL_CTX_DIRTY_VERTEX_ARRAY))
				return;
		}
		cmd.start = ctx->program ? (uintptr_t)indices[i] : 0;
		cmd.count = count[i];
		cmd.base_vertex = basevertex ? basevertex[i] : 0;
		ret = ctx->jkg_op->draw(ctx->jkg_ctx, &cmd);
		if (ret)
		{
			GL_SET_ERR(ctx, get_jkg_error(ret));
			continue;
		}
	}
}

void
glMultiDrawElements(GLenum mode,
                    const GLsizei *count,
                    GLenum type,
                    const GLvoid * const *indices,
                    GLsizei drawcount)
{
	multi_draw_elements(g_ctx,
	                    mode,
	                    count,
	                    type,
	                    indices,
	                    drawcount,
	                    NULL);
}

void
glMultiDrawElementsBaseVertex(GLenum mode,
                              const GLsizei *count,
                              GLenum type,
                              const GLvoid * const *indices,
                              GLsizei drawcount,
                              const GLint *basevertex)
{
	multi_draw_elements(g_ctx,
	                    mode,
	                    count,
	                    type,
	                    indices,
	                    drawcount,
	                    basevertex);
}

void
draw_arrays(struct gl_ctx *ctx,
            GLenum mode,
            GLint first,
            GLsizei count,
            GLsizei instancecount,
            GLuint baseinstance)
{
	struct jkg_draw_cmd cmd;
	int ret;

	if (count < 0
	 || instancecount < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_primitive(ctx, mode, &cmd.primitive))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!count)
		return;
	if (!ctx->program)
	{
		if (!fixed_update(ctx, NULL, GL_UNSIGNED_INT, count))
			return;
	}
	if (gl_ctx_commit(ctx, DRAW_DIRTY_MASK))
		return;
	cmd.start = first;
	cmd.count = count;
	cmd.instance_start = baseinstance;
	cmd.instance_count = instancecount;
	cmd.base_vertex = 0;
	cmd.indexed = false;
	cmd.primitive_restart_enable = false;
	cmd.primitive_restart_index = 0;
	ret = ctx->jkg_op->draw(ctx->jkg_ctx, &cmd);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	draw_arrays(g_ctx, mode, first, count, 0, 0);
}

void
glDrawArraysInstanced(GLenum mode,
                      GLint first,
                      GLsizei count,
                      GLsizei instancecount)
{
	draw_arrays(g_ctx, mode, first, count, instancecount, 0);
}

void
glDrawArraysInstancedBaseInstance(GLenum mode,
                                  GLint first,
                                  GLsizei count,
                                  GLsizei instancecount,
                                  GLuint baseinstance)
{
	draw_arrays(g_ctx, mode, first, count, instancecount, baseinstance);
}

void
glMultiDrawArrays(GLenum mode,
                  const GLint *first,
                  const GLsizei *count,
                  GLsizei drawcount)
{
	struct gl_ctx *ctx = g_ctx;
	struct jkg_draw_cmd cmd;
	int ret;

	if (drawcount < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (!get_primitive(ctx, mode, &cmd.primitive))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (!drawcount)
		return;
	if (gl_ctx_commit(ctx, DRAW_DIRTY_MASK))
		return;
	cmd.instance_start = 0;
	cmd.instance_count = 0;
	cmd.base_vertex = 0;
	cmd.indexed = false;
	cmd.primitive_restart_enable = false;
	cmd.primitive_restart_index = 0;
	for (GLsizei i = 0; i < drawcount; ++i)
	{
		if (count[i] < 0)
		{
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			continue;
		}
		if (!count[i])
			continue;
		if (!ctx->program)
		{
			if (!fixed_update(ctx, NULL, GL_UNSIGNED_INT, count[i]))
				continue;
		}
		cmd.start = first[i];
		cmd.count = count[i];
		ret = ctx->jkg_op->draw(ctx->jkg_ctx, &cmd);
		if (ret)
		{
			GL_SET_ERR(ctx, get_jkg_error(ret));
			continue;
		}
	}
}

void
glPrimitiveRestartIndex(GLuint index)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->primitive_restart_index = index;
}
