#include "internal.h"

#include <limits.h>
#include <string.h>

static const GLuint null_id;

static GLboolean
getv(struct gl_ctx *ctx,
     GLenum pname,
     const void **data,
     GLenum *format,
     GLuint *length)
{
	switch (pname)
	{
		case GL_ACTIVE_TEXTURE:
			*data = &ctx->active_texture;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_ARRAY_BUFFER_BINDING:
			*data = ctx->array_buffer ? &ctx->array_buffer->object.id : &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_BLEND:
			*data = &ctx->blend_state.attachments[0].enable;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_BLEND_COLOR:
			*data = ctx->blend_color;
			*format = GL_FLOAT;
			*length = 4;
			break;
		case GL_BLEND_DST_ALPHA:
			*data = &ctx->blend_state.attachments[0].dst_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_DST_RGB:
			*data = &ctx->blend_state.attachments[0].dst_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_EQUATION_ALPHA:
			*data = &ctx->blend_state.attachments[0].equation_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_EQUATION_RGB:
			*data = &ctx->blend_state.attachments[0].equation_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_SRC_ALPHA:
			*data = &ctx->blend_state.attachments[0].src_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_SRC_RGB:
			*data = &ctx->blend_state.attachments[0].src_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_ARRAY_SIZE:
			*data = &ctx->fixed.arrays.color.size;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_ARRAY_STRIDE:
			*data = &ctx->fixed.arrays.color.stride;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_ARRAY_TYPE:
			*data = &ctx->fixed.arrays.color.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_CLEAR_VALUE:
			*data = &ctx->clear_color[0];
			*format = GL_FLOAT;
			*length = 4;
			break;
		case GL_COLOR_LOGIC_OP:
			*data = &ctx->blend_state.logic_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_WRITEMASK:
			*data = &ctx->blend_state.attachments[0].color_mask[0];
			*format = GL_UNSIGNED_BYTE;
			*length = 4;
			break;
		case GL_COPY_READ_BUFFER_BINDING:
			*data = ctx->copy_read_buffer ? &ctx->copy_read_buffer->object.id : &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_COPY_WRITE_BUFFER_BINDING:
			*data = ctx->copy_write_buffer ? &ctx->copy_write_buffer->object.id : &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_CULL_FACE:
			*data = &ctx->rasterizer_state.cull_enable;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_CULL_FACE_MODE:
			*data = &ctx->rasterizer_state.cull_face;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_CURRENT_PROGRAM:
			*data = ctx->program ? &ctx->program->object.id : &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_DEPTH_CLEAR_VALUE:
			*data = &ctx->clear_depth;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_DEPTH_FUNC:
			*data = &ctx->depth_stencil_state.depth_compare;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_DEPTH_RANGE:
			*data = ctx->depth_range;
			*format = GL_FLOAT;
			*length = 2;
			break;
		case GL_DEPTH_TEST:
			*data = &ctx->depth_stencil_state.depth_test_enable;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_DEPTH_WRITEMASK:
			*data = &ctx->depth_stencil_state.depth_write_enable;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_DRAW_BUFFER:
			*data = &ctx->draw_fbo->draw_buffers[0];
			*format = GL_INT;
			*length = 1;
			break;
		case GL_DRAW_BUFFER0:
		case GL_DRAW_BUFFER1:
		case GL_DRAW_BUFFER2:
		case GL_DRAW_BUFFER3:
		case GL_DRAW_BUFFER4:
		case GL_DRAW_BUFFER5:
		case GL_DRAW_BUFFER6:
		case GL_DRAW_BUFFER7:
		case GL_DRAW_BUFFER8:
		case GL_DRAW_BUFFER9:
		case GL_DRAW_BUFFER10:
		case GL_DRAW_BUFFER11:
		case GL_DRAW_BUFFER12:
		case GL_DRAW_BUFFER13:
		case GL_DRAW_BUFFER14:
		case GL_DRAW_BUFFER15:
			if (pname - GL_DRAW_BUFFER0 > ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return GL_FALSE;
			}
			*data = &ctx->draw_fbo->draw_buffers[pname - GL_DRAW_BUFFER0];
			*format = GL_INT;
			*length = 1;
			break;
		case GL_DRAW_FRAMEBUFFER_BINDING:
			*data = ctx->draw_fbo == ctx->default_fbo ? &null_id : &ctx->draw_fbo->object.id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
			*data = ctx->vao->index_buffer ? &ctx->vao->index_buffer->object.id : &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_FOG:
			*data = &ctx->fixed.block.fog_enable;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_FOG_COLOR:
			*data = ctx->fixed.block.fog_color;
			*format = GL_FLOAT;
			*length = 4;
			break;
		case GL_FOG_DENSITY:
			*data = &ctx->fixed.block.fog_density;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FOG_END:
			*data = &ctx->fixed.block.fog_end;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FOG_INDEX:
			*data = &ctx->fixed.block.fog_index;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FOG_MODE:
			*data = &ctx->fixed.block.fog_mode;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_FOG_START:
			*data = &ctx->fixed.block.fog_start;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_FRONT_FACE:
			*data = &ctx->rasterizer_state.front_face;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_LINE_WIDTH:
			*data = &ctx->rasterizer_state.line_width;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_LINE_SMOOTH:
			*data = &ctx->rasterizer_state.line_smooth_enable;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_LOGIC_OP_MODE:
			*data = &ctx->blend_state.logic_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MAJOR_VERSION:
			*data = &ctx->version_major;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MATRIX_MODE:
			*data = &ctx->fixed.matrix_mode;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MAX_3D_TEXTURE_SIZE:
			*data = &ctx->jkg_caps->max_texture_3d_size;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_COLOR_ATTACHMENTS:
			*data = &ctx->jkg_caps->max_color_attachments;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_DRAW_BUFFERS:
			*data = &ctx->jkg_caps->max_draw_buffers;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_FRAMEBUFFER_HEIGHT:
			*data = &ctx->jkg_caps->max_framebuffer_height;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_FRAMEBUFFER_WIDTH:
			*data = &ctx->jkg_caps->max_framebuffer_width;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_LIGHTS:
			*data = &ctx->fixed.max_lights;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_MODELVIEW_STACK_DEPTH:
			*data = &ctx->fixed.modelview_max_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MAX_PROJECTION_STACK_DEPTH:
			*data = &ctx->fixed.projection_max_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MAX_RENDERBUFFER_SIZE:
			*data = &ctx->jkg_caps->max_renderbuffer_size;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_TEXTURE_MAX_ANISOTROPY:
			*data = &ctx->jkg_caps->max_anisotropy;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_TEXTURE_SIZE:
			*data = &ctx->jkg_caps->max_texture_2d_size;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_VERTEX_ATTRIB_BINDINGS:
			*data = &ctx->jkg_caps->max_vertex_attrib_bindings;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
			*data = &ctx->jkg_caps->max_vertex_attrib_relative_offset;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_VERTEX_ATTRIB_STRIDE:
			*data = &ctx->jkg_caps->max_vertex_attrib_stride;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MAX_VERTEX_ATTRIBS:
			*data = &ctx->jkg_caps->max_vertex_attribs;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MINOR_VERSION:
			*data = &ctx->version_minor;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_MODELVIEW_STACK_DEPTH:
			*data = &ctx->fixed.modelview_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_MODELVIEW_MATRIX:
			*data = ctx->fixed.modelview_matrix[ctx->fixed.modelview_stack_depth];
			*format = GL_FLOAT;
			*length = 16;
			break;
		case GL_NORMAL_ARRAY_STRIDE:
			*data = &ctx->fixed.arrays.normal.stride;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_NORMAL_ARRAY_TYPE:
			*data = &ctx->fixed.arrays.normal.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_NUM_EXTENSIONS:
			*data = &ctx->nextensions;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_POINT_SIZE:
			*data = &ctx->rasterizer_state.point_size;
			*format = GL_FLOAT;
			*length = 1;
			break;
		case GL_PRIMITIVE_RESTART_INDEX:
			*data = &ctx->primitive_restart_index;
			*format = GL_UNSIGNED_INT;
			*length =  1;
			break;
		case GL_PROJECTION_MATRIX:
			*data = ctx->fixed.projection_matrix[ctx->fixed.projection_stack_depth];
			*format = GL_FLOAT;
			*length = 16;
			break;
		case GL_PROJECTION_STACK_DEPTH:
			*data = &ctx->fixed.projection_stack_depth;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_READ_FRAMEBUFFER_BINDING:
			*data = ctx->read_fbo == ctx->default_fbo ? &null_id : &ctx->read_fbo->object.id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_SCISSOR_BOX:
			*data = &ctx->scissor;
			*format = GL_FLOAT;
			*length = 4;
			break;
		case GL_STENCIL_BACK_FAIL:
			*data = &ctx->depth_stencil_state.back.fail_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_BACK_FUNC:
			*data = &ctx->depth_stencil_state.back.compare_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
			*data = &ctx->depth_stencil_state.back.zfail_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
			*data = &ctx->depth_stencil_state.back.pass_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_BACK_REF:
			*data = &ctx->stencil_ref[0];
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_BACK_VALUE_MASK:
			*data = &ctx->depth_stencil_state.back.compare_mask;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_STENCIL_BACK_WRITEMASK:
			*data = &ctx->depth_stencil_state.back.write_mask;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_STENCIL_CLEAR_VALUE:
			*data = &ctx->clear_stencil;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_STENCIL_FAIL:
			*data = &ctx->depth_stencil_state.front.fail_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_FUNC:
			*data = &ctx->depth_stencil_state.front.compare_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_PASS_DEPTH_FAIL:
			*data = &ctx->depth_stencil_state.front.zfail_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_PASS_DEPTH_PASS:
			*data = &ctx->depth_stencil_state.front.pass_op;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_REF:
			*data = &ctx->stencil_ref[1];
			*format = GL_INT;
			*length = 1;
			break;
		case GL_STENCIL_TEST:
			*data = &ctx->depth_stencil_state.stencil_enable;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_STENCIL_VALUE_MASK:
			*data = &ctx->depth_stencil_state.front.compare_mask;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_STENCIL_WRITEMASK:
			*data = &ctx->depth_stencil_state.front.write_mask;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_TEXTURE_BINDING_1D:
			if (ctx->textures[ctx->active_texture - GL_TEXTURE0])
				*data = &ctx->textures[ctx->active_texture - GL_TEXTURE0]->object.id;
			else
				*data = &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_TEXTURE_BINDING_2D:
			if (ctx->textures[ctx->active_texture - GL_TEXTURE0])
				*data = &ctx->textures[ctx->active_texture - GL_TEXTURE0]->object.id;
			else
				*data = &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_TEXTURE_BINDING_3D:
			if (ctx->textures[ctx->active_texture - GL_TEXTURE0])
				*data = &ctx->textures[ctx->active_texture - GL_TEXTURE0]->object.id;
			else
				*data = &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_TEXTURE_COORD_ARRAY_SIZE:
			*data = &ctx->fixed.arrays.texcoord.size;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_TEXTURE_COORD_ARRAY_STRIDE:
			*data = &ctx->fixed.arrays.texcoord.stride;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_TEXTURE_COORD_ARRAY_TYPE:
			*data = &ctx->fixed.arrays.texcoord.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_UNIFORM_BUFFER_BINDING:
			*data = ctx->uniform_buffer ? &ctx->uniform_buffer->object.id : &null_id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_VERTEX_ARRAY_BINDING:
			if (ctx->vao == ctx->default_vao)
				*data = &null_id;
			else
				*data = &ctx->vao->object.id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_VERTEX_ARRAY_SIZE:
			*data = &ctx->fixed.arrays.vertex.size;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_VERTEX_ARRAY_STRIDE:
			*data = &ctx->fixed.arrays.vertex.stride;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_VERTEX_ARRAY_TYPE:
			*data = &ctx->fixed.arrays.vertex.type;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_VIEWPORT:
			*data = &ctx->viewport;
			*format = GL_FLOAT;
			*length = 4;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return GL_FALSE;
	}
	return GL_TRUE;
}

static GLboolean
getiv(struct gl_ctx *ctx,
      GLenum target,
      GLuint index,
      const void **data,
      GLenum *format,
      GLuint *length)
{
	switch (target)
	{
		case GL_VERTEX_BINDING_BUFFER:
			if (index >= ctx->jkg_caps->max_vertex_attrib_bindings)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->vao->vertex_buffers[index].buffer->object.id;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_VERTEX_BINDING_DIVISOR:
			if (index >= ctx->jkg_caps->max_vertex_attribs)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->vao->input_layout_state.attribs[index].divisor;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_VERTEX_BINDING_OFFSET:
			if (index >= ctx->jkg_caps->max_vertex_attrib_bindings)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->vao->vertex_buffers[index].offset;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_VERTEX_BINDING_STRIDE:
			if (index >= ctx->jkg_caps->max_vertex_attrib_bindings)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->vao->vertex_buffers[index].stride;
			*format = GL_UNSIGNED_INT;
			*length = 1;
			break;
		case GL_BLEND:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].enable;
			*format = GL_UNSIGNED_BYTE;
			*length = 1;
			break;
		case GL_BLEND_DST_ALPHA:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].dst_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_DST_RGB:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].dst_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_EQUATION_ALPHA:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].equation_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_EQUATION_RGB:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].equation_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_SRC_ALPHA:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].src_alpha;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_BLEND_SRC_RGB:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].src_rgb;
			*format = GL_INT;
			*length = 1;
			break;
		case GL_COLOR_WRITEMASK:
			if (index >= ctx->jkg_caps->max_draw_buffers)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return GL_FALSE;
			}
			*data = &ctx->blend_state.attachments[index].color_mask[0];
			*format = GL_UNSIGNED_BYTE;
			*length = 4;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return GL_FALSE;
	}
	return GL_TRUE;
}

void
glGetBooleanv(GLenum pname, GLboolean *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getv(ctx, pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i] != 0;
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i] != 0;
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i] != 0;
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i] != 0;
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i] != 0;
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i] != 0;
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i] != 0;
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i] != 0;
				break;
		}
	}
}

void
glGetDoublev(GLenum pname, GLdouble *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getv(ctx, pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void
glGetFloatv(GLenum pname, GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getv(ctx, pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void
glGetIntegerv(GLenum pname, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getv(ctx, pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void
glGetInteger64v(GLenum pname, GLint64 *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getv(ctx, pname, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void
glGetPointerv(GLenum pname, GLvoid **params)
{
	struct gl_ctx *ctx = g_ctx;

	switch (pname)
	{
		case GL_COLOR_ARRAY_POINTER:
			*params = (GLvoid*)ctx->fixed.arrays.color.pointer;
			break;
		case GL_TEXTURE_COORD_ARRAY_POINTER:
			*params = (GLvoid*)ctx->fixed.arrays.texcoord.pointer;
			break;
		case GL_VERTEX_ARRAY_POINTER:
			*params = (GLvoid*)ctx->fixed.arrays.vertex.pointer;
			break;
		case GL_NORMAL_ARRAY_POINTER:
			*params = (GLvoid*)ctx->fixed.arrays.normal.pointer;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			break;
	}
}

void
glGetBooleani_v(GLenum target, GLuint index, GLboolean *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getiv(ctx, target, index, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i] != 0;
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i] != 0;
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i] != 0;
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i] != 0;
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i] != 0;
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i] != 0;
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i] != 0;
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i] != 0;
				break;
		}
	}
}

void
glGetIntegeri_v(GLenum target, GLuint index, GLint *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getiv(ctx, target, index, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void
glGetFloati_v(GLenum target, GLuint index, GLfloat *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getiv(ctx, target, index, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void
glGetDoublei_v(GLenum target, GLuint index, GLdouble *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getiv(ctx, target, index, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

void
glGetInteger64i_v(GLenum target, GLuint index, GLint64 *params)
{
	struct gl_ctx *ctx = g_ctx;
	const void *data;
	GLenum format;
	GLuint length;

	if (!getiv(ctx, target, index, &data, &format, &length))
		return;
	for (GLuint i = 0; i < length; ++i)
	{
		switch (format)
		{
			case GL_BYTE:
				params[i] = ((GLbyte*)data)[i];
				break;
			case GL_UNSIGNED_BYTE:
				params[i] = ((GLubyte*)data)[i];
				break;
			case GL_SHORT:
				params[i] = ((GLshort*)data)[i];
				break;
			case GL_UNSIGNED_SHORT:
				params[i] = ((GLushort*)data)[i];
				break;
			case GL_INT:
				params[i] = ((GLint*)data)[i];
				break;
			case GL_UNSIGNED_INT:
				params[i] = ((GLuint*)data)[i];
				break;
			case GL_FLOAT:
				params[i] = ((GLfloat*)data)[i];
				break;
			case GL_DOUBLE:
				params[i] = ((GLdouble*)data)[i];
				break;
		}
	}
}

const GLubyte *
glGetString(GLenum name)
{
	struct gl_ctx *ctx = g_ctx;

	switch (name)
	{
		case GL_VENDOR:
			return (const GLubyte*)ctx->jkg_caps->vendor;
		case GL_RENDERER:
			return (const GLubyte*)ctx->jkg_caps->renderer;
		case GL_VERSION:
			return (const GLubyte*)ctx->jkg_caps->version;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return NULL;
	}
}

const GLubyte *
glGetStringi(GLenum name, GLuint index)
{
	struct gl_ctx *ctx = g_ctx;

	switch (name)
	{
		case GL_EXTENSIONS:
			if (index >= ctx->nextensions)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return NULL;
			}
			return (const GLubyte*)ctx->extensions[index];
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return NULL;
	}
}
