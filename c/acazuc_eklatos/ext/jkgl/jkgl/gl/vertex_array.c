#include "internal.h"
#include "fnv.h"

#include <stdlib.h>
#include <string.h>

struct vertex_array *
vertex_array_alloc(struct gl_ctx *ctx)
{
	struct vertex_array *vao;

	vao = calloc(1, sizeof(*vao));
	if (!vao)
		return NULL;
	vao->object.type = OBJECT_VERTEX_ARRAY;
	for (size_t i = 0; i < ctx->jkg_caps->max_vertex_attrib_bindings; ++i)
	{
		vao->vertex_buffers[i].buffer = NULL;
		vao->vertex_buffers[i].stride = 0;
		vao->vertex_buffers[i].offset = 0;
	}
	for (size_t i = 0; i < ctx->jkg_caps->max_vertex_attribs; ++i)
	{
		struct jkg_input_layout_attrib *attrib = &vao->input_layout_state.attribs[i];
		attrib->enable = false;
		attrib->offset = 0;
		attrib->binding = 0;
		attrib->divisor = 0;
		attrib->format = JKG_FORMAT_UNKNOWN;
	}
	vao->index_buffer = NULL;
	vao->input_layout = NULL;
	vao->dirty = 0;
	return vao;
}

void
glGenVertexArrays(GLsizei n, GLuint *arrays)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;
	GLsizei i;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (i = 0; i < n; ++i)
	{
		vao = vertex_array_alloc(ctx);
		if (!vao)
			goto err;
		if (!object_alloc(ctx, &vao->object))
		{
			vertex_array_free(ctx, vao);
			goto err;
		}
		arrays[i] = vao->object.id;
	}
	return;

err:
	for (GLsizei j = 0; j < i; ++j)
	{
		vao = object_get(ctx, arrays[j], OBJECT_VERTEX_ARRAY);
		if (!vao)
			continue;
		vertex_array_free(ctx, vao);
	}
}

void
glCreateVertexArrays(GLsizei n,
                     GLuint *arrays)
{
	glGenVertexArrays(n, arrays); /* XXX more? */
}

void
vertex_array_free(struct gl_ctx *ctx,
                  struct vertex_array *vao)
{
	if (ctx->vao == vao)
		vertex_array_bind(ctx, ctx->default_vao);
	object_free(ctx, &vao->object);
}

void
glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		GLuint id = arrays[i];
		if (!id)
			continue;
		struct vertex_array *vao = object_get(ctx, id, OBJECT_VERTEX_ARRAY);
		if (!vao)
			continue;
		vertex_array_free(ctx, vao);
	}
}

GLboolean
glIsVertexArray(GLuint id)
{
	return object_is(g_ctx, id, OBJECT_VERTEX_ARRAY);
}

void
vertex_array_bind(struct gl_ctx *ctx,
                  struct vertex_array *vao)
{
	if (ctx->vao != vao)
	{
		ctx->vao = vao;
		ctx->dirty |= GL_CTX_DIRTY_VERTEX_ARRAY;
	}
}

void
glBindVertexArray(GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	if (id)
	{
		vao = object_get(ctx, id, OBJECT_VERTEX_ARRAY);
		if (!vao)
			return;
	}
	else
	{
		vao = ctx->default_vao;
	}
	vertex_array_bind(ctx, vao);
}

void
vertex_array_index_buffer(struct gl_ctx *ctx,
                          struct vertex_array *vao,
                          struct buffer *buffer)
{
	(void)ctx;
	if (vao->index_buffer != buffer)
	{
		vao->index_buffer = buffer;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INDEX_BUFFER;
	}
}

void
vertex_array_vertex_buffer(struct gl_ctx *ctx,
                           struct vertex_array *vao,
                           uint32_t index,
                           struct buffer *buffer,
                           uint32_t offset,
                           uint32_t stride)
{
	(void)ctx;
	if (vao->vertex_buffers[index].buffer != buffer)
	{
		vao->vertex_buffers[index].buffer = buffer;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_VERTEX_BUFFERS;
	}
	if (vao->vertex_buffers[index].stride != stride)
	{
		vao->vertex_buffers[index].stride = stride;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_VERTEX_BUFFERS;
	}
	if (vao->vertex_buffers[index].offset != offset)
	{
		vao->vertex_buffers[index].offset = offset;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_VERTEX_BUFFERS;
	}
}

static int
vertex_array_vertex_buffers(struct gl_ctx *ctx,
                            struct vertex_array *vao,
                            uint32_t first,
                            uint32_t count,
                            const GLuint *buffers,
                            const GLintptr *offsets,
                            const GLsizei *strides)
{
	struct buffer *buffer;
	GLuint last;

	if (first >= ctx->jkg_caps->max_vertex_attrib_bindings
	 || __builtin_add_overflow(first, count, &last)
	 || last > ctx->jkg_caps->max_vertex_attrib_bindings)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return 1;
	}
	for (uint32_t i = 0; i < count; ++i)
	{
		if (strides[i] < 0
		 || (GLuint)strides[i] > ctx->jkg_caps->max_vertex_attrib_stride)
		{
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return 1;
		}
		if (offsets[i] < 0)
		{
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return 1;
		}
		if (!strides[i]) /* XXX consider as thightly packed ? */
		{
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return 1;
		}
		if (buffers[i])
		{
			buffer = object_get(ctx, buffers[i], OBJECT_BUFFER);
			if (!buffer)
				return 1;
		}
		else
		{
			buffer = NULL;
		}
		vertex_array_vertex_buffer(ctx,
		                           vao,
		                           first + i,
		                           buffer,
		                           strides[i],
		                           offsets[i]);
	}
	return 0;
}

void
glBindVertexBuffer(GLuint binding,
                   GLuint id,
                   GLintptr offset,
                   GLsizei stride)
{
	struct gl_ctx *ctx = g_ctx;

	vertex_array_vertex_buffers(ctx,
	                            ctx->vao,
	                            binding,
	                            1,
	                            &id,
	                            &offset,
	                            &stride);
}

void
glVertexArrayVertexBuffer(GLuint vaobj,
                          GLuint bindingindex,
                          GLuint id,
                          GLintptr offset,
                          GLsizei stride)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_vertex_buffers(ctx,
	                            vao,
	                            bindingindex,
	                            1,
	                            &id,
	                            &offset,
	                            &stride);
}

void
glBindVertexBuffers(GLuint first,
                    GLsizei count,
                    const GLuint *buffers,
                    const GLintptr *offsets,
                    const GLsizei *strides)
{
	struct gl_ctx *ctx = g_ctx;

	vertex_array_vertex_buffers(ctx,
	                            ctx->vao,
	                            first,
	                            count,
	                            buffers,
	                            offsets,
	                            strides);
}

void
glVertexArrayVertexBuffers(GLuint vaobj,
                           GLuint first,
                           GLsizei count,
                           const GLuint *buffers,
                           const GLintptr *offsets,
                           const GLsizei *strides)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_vertex_buffers(ctx,
	                            vao,
	                            first,
	                            count,
	                            buffers,
	                            offsets,
	                            strides);
}

void
vertex_array_attrib_binding(struct gl_ctx *ctx,
                            struct vertex_array *vao,
                            uint32_t attribindex,
                            uint32_t bindingindex)
{
	if (attribindex >= ctx->jkg_caps->max_vertex_attribs)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (bindingindex >= ctx->jkg_caps->max_vertex_attrib_bindings)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (vao->input_layout_state.attribs[attribindex].binding != bindingindex)
	{
		vao->input_layout_state.attribs[attribindex].binding = bindingindex;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
}

void
glVertexAttribBinding(GLuint attribindex,
                      GLuint bindingindex)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->vao == ctx->default_vao)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	vertex_array_attrib_binding(ctx, ctx->vao, attribindex, bindingindex);
}

void
glVertexArrayAttribBinding(GLuint vaobj,
                           GLuint attribindex,
                           GLuint bindingindex)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_attrib_binding(ctx, vao, attribindex, bindingindex);
}

void
vertex_array_binding_divisor(struct gl_ctx *ctx,
                             struct vertex_array *vao,
                             uint32_t index,
                             uint32_t divisor)
{
	if (index >= ctx->jkg_caps->max_vertex_attrib_bindings)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (vao->input_layout_state.attribs[index].divisor != divisor)
	{
		vao->input_layout_state.attribs[index].divisor = divisor;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
}

void
glVertexAttribDivisor(GLuint index, GLuint divisor)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->vao == ctx->default_vao)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	vertex_array_binding_divisor(ctx, ctx->vao, index, divisor);
}

void
glVertexBindingDivisor(GLuint bindingindex,
                       GLuint divisor)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->vao == ctx->default_vao)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	vertex_array_binding_divisor(ctx, ctx->vao, bindingindex, divisor);
}

void
glVertexArrayBindingDivisor(GLuint vaobj,
                            GLuint bindingindex,
                            GLuint divisor)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_binding_divisor(ctx, vao, bindingindex, divisor);
}

int
vertex_array_attrib_format(struct gl_ctx *ctx,
                           struct vertex_array *vao,
                           uint32_t index,
                           uint32_t size,
                           GLenum type,
                           enum conv_type conv,
                           uint32_t offset)
{
	enum jkg_format format;

	if (index >= ctx->jkg_caps->max_vertex_attribs)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return 1;
	}
	if (offset >= ctx->jkg_caps->max_vertex_attrib_relative_offset)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return 1;
	}
	if (get_attrib_format(ctx, size, type, conv, &format))
		return 1;
	if (vao->input_layout_state.attribs[index].format != format)
	{
		vao->input_layout_state.attribs[index].format = format;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
	if (vao->input_layout_state.attribs[index].offset != offset)
	{
		vao->input_layout_state.attribs[index].offset = offset;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
	return 0;
}

void
glVertexAttribFormat(GLuint attribindex,
                     GLint size,
                     GLenum type,
                     GLboolean normalized,
                     GLuint relativeoffset)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->vao == ctx->default_vao)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	vertex_array_attrib_format(ctx,
	                           ctx->vao,
	                           attribindex,
	                           size,
	                           type,
	                           normalized ? CONV_NORM : CONV_SCALED,
	                           relativeoffset);
}

void
glVertexArrayAttribFormat(GLuint vaobj,
                          GLuint attribindex,
                          GLint size,
                          GLenum type,
                          GLboolean normalized,
                          GLuint relativeoffset)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_attrib_format(ctx,
	                           vao,
	                           attribindex,
	                           size,
	                           type,
	                           normalized ? CONV_NORM : CONV_SCALED,
	                           relativeoffset);
}

void
glVertexAttribIFormat(GLuint attribindex,
                      GLint size,
                      GLenum type,
                      GLuint relativeoffset)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->vao == ctx->default_vao)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	vertex_array_attrib_format(ctx,
	                           ctx->vao,
	                           attribindex,
	                           size,
	                           type,
	                           CONV_INT,
	                           relativeoffset);
}

void
glVertexArrayAttribIFormat(GLuint vaobj,
                           GLuint attribindex,
                           GLint size,
                           GLenum type,
                           GLuint relativeoffset)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_attrib_format(ctx,
	                           vao,
	                           attribindex,
	                           size,
	                           type,
	                           CONV_INT,
	                           relativeoffset);
}

void
glVertexAttribLFormat(GLuint attribindex,
                      GLint size,
                      GLenum type,
                      GLuint relativeoffset)
{
	struct gl_ctx *ctx = g_ctx;

	/* XXX */
	GL_SET_ERR(ctx, GL_INVALID_OPERATION);
	(void)attribindex;
	(void)size;
	(void)type;
	(void)relativeoffset;
}

void
glVertexArrayAttribLFormat(GLuint vaobj,
                           GLuint attribindex,
                           GLint size,
                           GLenum type,
                           GLuint relativeoffset)
{
	struct gl_ctx *ctx = g_ctx;

	/* XXX */
	GL_SET_ERR(ctx, GL_INVALID_OPERATION);
	(void)vaobj;
	(void)attribindex;
	(void)size;
	(void)type;
	(void)relativeoffset;
}

static void
vertex_attrib_pointer(struct gl_ctx *ctx,
                      struct vertex_array *vao,
                      GLuint index,
                      GLint size,
                      GLenum type,
                      enum conv_type conv,
                      GLsizei stride,
                      GLintptr relativeoffset)
{
	enum jkg_format format;

	if (!ctx->array_buffer)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (index >= ctx->jkg_caps->max_vertex_attribs
	 || index >= ctx->jkg_caps->max_vertex_attrib_bindings)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (relativeoffset >= ctx->jkg_caps->max_vertex_attrib_relative_offset)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (get_attrib_format(ctx, size, type, conv, &format))
		return;
	if (vertex_array_vertex_buffers(ctx,
	                                vao,
	                                index,
	                                1,
	                                &ctx->array_buffer->object.id,
	                                &relativeoffset,
	                                &stride))
		return;
	if (vao->input_layout_state.attribs[index].binding != index)
	{
		vao->input_layout_state.attribs[index].binding = index;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
	if (vao->input_layout_state.attribs[index].format != format)
	{
		vao->input_layout_state.attribs[index].format = format;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
	if (vao->input_layout_state.attribs[index].offset != relativeoffset)
	{
		vao->input_layout_state.attribs[index].offset = relativeoffset;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
}

void
glVertexAttribPointer(GLuint index,
                      GLint size,
                      GLenum type,
                      GLboolean normalized,
                      GLsizei stride,
                      const GLvoid *pointer)
{
	struct gl_ctx *ctx = g_ctx;

	vertex_attrib_pointer(ctx,
	                      ctx->vao,
	                      index,
	                      size,
	                      type,
	                      normalized ? CONV_NORM : CONV_SCALED,
	                      stride,
	                      (GLuint)(GLintptr)pointer);
}

void
glVertexAttribIPointer(GLuint index,
                       GLint size,
                       GLenum type,
                       GLsizei stride,
                       const GLvoid *pointer)
{
	struct gl_ctx *ctx = g_ctx;

	vertex_attrib_pointer(ctx,
	                      ctx->vao,
	                      index,
	                      size,
	                      type,
	                      CONV_INT,
	                      stride,
	                      (GLuint)(GLintptr)pointer);
}

void
glVertexAttribLPointer(GLuint index,
                       GLint size,
                       GLenum type,
                       GLsizei stride,
                       const GLvoid *pointer)
{
	struct gl_ctx *ctx = g_ctx;

	/* XXX */
	GL_SET_ERR(ctx, GL_INVALID_OPERATION);
	(void)index;
	(void)size;
	(void)type;
	(void)stride;
	(void)pointer;
}

void
vertex_array_attrib_enable(struct gl_ctx *ctx,
                           struct vertex_array *vao,
                           uint32_t index,
                           bool enable)
{
	if (index >= ctx->jkg_caps->max_vertex_attribs)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (vao->input_layout_state.attribs[index].enable != enable)
	{
		vao->input_layout_state.attribs[index].enable = enable;
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	}
}

void
glEnableVertexAttribArray(GLuint index)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->vao == ctx->default_vao)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	vertex_array_attrib_enable(ctx, ctx->vao, index, GL_TRUE);
}

void
glDisableVertexAttribArray(GLuint index)
{
	struct gl_ctx *ctx = g_ctx;

	if (ctx->vao == ctx->default_vao)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	vertex_array_attrib_enable(ctx, ctx->vao, index, GL_FALSE);
}

void
glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_attrib_enable(ctx, vao, index, GL_TRUE);
}

void
glDisableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
	struct gl_ctx *ctx = g_ctx;
	struct vertex_array *vao;

	vao = object_get(ctx, vaobj, OBJECT_VERTEX_ARRAY);
	if (!vao)
		return;
	vertex_array_attrib_enable(ctx, vao, index, GL_FALSE);
}

static uint32_t
input_layout_attrib_hash(uint32_t hash,
                         const struct jkg_input_layout_attrib *attrib)
{
	hash = fnv32v(hash, &attrib->enable, sizeof(attrib->enable));
	if (attrib->enable)
	{
		hash = fnv32v(hash, &attrib->offset, sizeof(attrib->offset));
		hash = fnv32v(hash, &attrib->binding, sizeof(attrib->binding));
		hash = fnv32v(hash, &attrib->divisor, sizeof(attrib->divisor));
		hash = fnv32v(hash, &attrib->format, sizeof(attrib->format));
	}
	return hash;
}

static uint32_t
input_layout_state_hash(struct gl_ctx *ctx,
                        const void *st)
{
	const struct input_layout_state *state = st;
	uint32_t hash;

	hash = FNV_BASIS32;
	for (uint32_t i = 0; i < ctx->jkg_caps->max_vertex_attribs; ++i)
		hash = input_layout_attrib_hash(hash, &state->attribs[i]);
	return hash;
}

static bool
input_layout_attrib_eq(const struct jkg_input_layout_attrib *a,
                       const struct jkg_input_layout_attrib *b)
{
	if (a->enable != b->enable)
		return false;
	if (a->enable)
	{
		if (a->offset != b->offset
		 || a->binding != b->binding
		 || a->divisor != b->divisor
		 || a->format != b->format)
			return false;
	}
	return true;
}

static bool
input_layout_state_eq(struct gl_ctx *ctx,
                      const struct cache_entry *entry,
                      const void *state)
{
	const struct input_layout_state *a = &((struct input_layout_entry*)entry)->state;
	const struct input_layout_state *b = state;

	for (GLuint i = 0; i < ctx->jkg_caps->max_vertex_attribs; ++i)
	{
		if (!input_layout_attrib_eq(&a->attribs[i], &b->attribs[i]))
			return false;
	}
	return true;
}

static struct cache_entry *
input_layout_entry_alloc(struct gl_ctx *ctx,
                         GLuint hash,
                         const void *st)
{
	struct jkg_input_layout_create_info create_info;
	const struct input_layout_state *state = st;
	struct input_layout_entry *entry;
	int ret;

	entry = malloc(sizeof(*entry));
	if (!entry)
	{
		GL_SET_ERR(ctx, GL_OUT_OF_MEMORY);
		return NULL;
	}
	memcpy(&entry->state, state, sizeof(*state));
	entry->cache.hash = hash;
	entry->cache.ref = 0;
	create_info.nattribs = ctx->jkg_caps->max_vertex_attribs;
	create_info.attribs = state->attribs;
	ret = ctx->jkg_op->input_layout_alloc(ctx->jkg_ctx,
	                                      &create_info,
	                                      &entry->input_layout);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		free(entry);
		return NULL;
	}
	return &entry->cache;
}

static void
input_layout_entry_free(struct gl_ctx *ctx,
                        struct cache_entry *entry)
{
	ctx->jkg_op->input_layout_free(ctx->jkg_ctx,
	                               ((struct input_layout_entry*)entry)->input_layout);
}

const struct cache_op
input_layout_cache_op =
{
	.alloc = input_layout_entry_alloc,
	.free = input_layout_entry_free,
	.hash = input_layout_state_hash,
	.eq = input_layout_state_eq,
};
