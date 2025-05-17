#include "internal.h"

#include <assert.h>
#include <stdlib.h>

static struct buffer *
get_buffer(struct gl_ctx *ctx, GLenum target)
{
	struct buffer *buffer;

	switch (target)
	{
		case GL_ARRAY_BUFFER:
			buffer = ctx->array_buffer;
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			buffer = ctx->vao->index_buffer;
			break;
		case GL_COPY_READ_BUFFER:
			buffer = ctx->copy_read_buffer;
			break;
		case GL_COPY_WRITE_BUFFER:
			buffer = ctx->copy_write_buffer;
			break;
		case GL_UNIFORM_BUFFER:
			buffer = ctx->uniform_buffer;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return NULL;
	}
	if (!buffer)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return NULL;
	}
	return buffer;
}

struct buffer *
buffer_alloc(struct gl_ctx *ctx)
{
	struct buffer *buffer;

	(void)ctx;
	buffer = calloc(1, sizeof(*buffer));
	if (!buffer)
		return NULL;
	buffer->object.type = OBJECT_BUFFER;
	buffer->buffer = NULL;
	buffer->dirty = 0;
	return buffer;
}

void
glGenBuffers(GLsizei n, GLuint *buffers)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;
	GLsizei i;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (i = 0; i < n; ++i)
	{
		struct buffer *buffer = buffer_alloc(ctx);
		if (!buffer)
			goto err;
		if (!object_alloc(ctx, &buffer->object))
		{
			buffer_free(ctx, buffer);
			goto err;
		}
		buffers[i] = buffer->object.id;
	}
	return;

err:
	for (GLsizei j = 0; j < i; ++j)
	{
		buffer = object_get(ctx, buffers[j], OBJECT_BUFFER);
		if (!buffer)
			continue;
		buffer_free(ctx, buffer);
	}
}

void
glCreateBuffers(GLsizei n, GLuint *buffers)
{
	glGenBuffers(n, buffers); /* XXX really ? */
}

void
buffer_free(struct gl_ctx *ctx, struct buffer *buffer)
{
	if (buffer->buffer)
		ctx->jkg_op->buffer_free(ctx->jkg_ctx, buffer->buffer);
	object_free(ctx, &buffer->object);
}

void
glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
	struct gl_ctx *ctx = g_ctx;

	if (n < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	for (GLsizei i = 0; i < n; ++i)
	{
		GLuint id = buffers[i];
		struct buffer *buffer = object_get(ctx, id, OBJECT_BUFFER);
		if (!buffer)
			continue;
		buffer_free(ctx, buffer);
	}
}

GLboolean
glIsBuffer(GLuint id)
{
	return object_is(g_ctx, id, OBJECT_BUFFER);
}

void
glBindBuffer(GLenum target, GLuint id)
{
	struct buffer *buffer;
	struct gl_ctx *ctx = g_ctx;

	if (!id)
	{
		switch (target)
		{
			case GL_ARRAY_BUFFER:
				ctx->array_buffer = NULL;
				break;
			case GL_ELEMENT_ARRAY_BUFFER:
				vertex_array_index_buffer(ctx, ctx->vao, NULL);
				break;
			case GL_COPY_READ_BUFFER:
				ctx->copy_read_buffer = NULL;
				break;
			case GL_COPY_WRITE_BUFFER:
				ctx->copy_write_buffer = NULL;
				break;
			case GL_UNIFORM_BUFFER:
				ctx->uniform_buffer = NULL;
				break;
			default:
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return;
		}
		return;
	}
	buffer = object_get(ctx, id, OBJECT_BUFFER);
	if (!buffer)
		return;
	switch (target)
	{
		case GL_ARRAY_BUFFER:
			if (ctx->array_buffer == buffer)
				break;
			ctx->array_buffer = buffer;
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			vertex_array_index_buffer(ctx, ctx->vao, buffer);
			break;
		case GL_COPY_READ_BUFFER:
			if (ctx->copy_read_buffer == buffer)
				break;
			ctx->copy_read_buffer = buffer;
			break;
		case GL_COPY_WRITE_BUFFER:
			if (ctx->copy_write_buffer == buffer)
				break;
			ctx->copy_write_buffer = buffer;
			break;
		case GL_UNIFORM_BUFFER:
			if (ctx->uniform_buffer == buffer)
				break;
			ctx->uniform_buffer = buffer;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	buffer->target = target;
}

void
buffer_storage(struct gl_ctx *ctx,
               struct buffer *buffer,
               GLsizeiptr size,
               const GLvoid *data,
               GLbitfield flags)
{
	struct jkg_buffer_create_info create_info;
	struct jkg_buffer *jkg_buffer;
	int ret;

	(void)flags; /* XXX */
	if (size <= 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	create_info.size = size;
	switch (buffer->target)
	{
		case GL_ARRAY_BUFFER:
		case GL_COPY_READ_BUFFER:
		case GL_COPY_WRITE_BUFFER:
			create_info.type = JKG_BUFFER_VERTEX;
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			create_info.type = JKG_BUFFER_INDEX;
			break;
		case GL_UNIFORM_BUFFER:
			create_info.type = JKG_BUFFER_UNIFORM;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	ret = ctx->jkg_op->buffer_alloc(ctx->jkg_ctx,
	                                &create_info,
	                                &jkg_buffer);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
	if (data)
	{
		ret = ctx->jkg_op->buffer_write(ctx->jkg_ctx,
		                                jkg_buffer,
		                                data,
		                                size,
		                                0);
		if (ret)
		{
			ctx->jkg_op->buffer_free(ctx->jkg_ctx, jkg_buffer);
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return;
		}
	}
	if (buffer->buffer)
		ctx->jkg_op->buffer_free(ctx->jkg_ctx, buffer->buffer);
	buffer->buffer = jkg_buffer;
	buffer->size = size;
	buffer->dirty = GL_BUFFER_DIRTY_VERTEX
	              | GL_BUFFER_DIRTY_INDEX
	              | GL_BUFFER_DIRTY_UNIFORM;
}

void
glBufferStorage(GLenum target,
                GLsizeiptr size,
                const GLvoid *data,
                GLbitfield flags)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = get_buffer(ctx, target);
	if (!buffer)
		return;
	buffer_storage(ctx, buffer, size, data, flags);
}

void
glNamedBufferStorage(GLuint id,
                     GLsizeiptr size,
                     const GLvoid *data,
                     GLbitfield flags)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = object_get(ctx, id, OBJECT_BUFFER);
	if (!buffer)
		return;
	buffer_storage(ctx, buffer, size, data, flags);
}

void
buffer_data(struct gl_ctx *ctx,
            struct buffer *buffer,
            GLsizeiptr size,
            const GLvoid *data,
            GLenum usage)
{
	struct jkg_buffer_create_info create_info;
	struct jkg_buffer *jkg_buffer;
	int ret;

	switch (usage)
	{
		case GL_STATIC_DRAW:
		case GL_STATIC_READ:
		case GL_STATIC_COPY:
		case GL_DYNAMIC_DRAW:
		case GL_DYNAMIC_READ:
		case GL_DYNAMIC_COPY:
		case GL_STREAM_DRAW:
		case GL_STREAM_READ:
		case GL_STREAM_COPY:
			/* XXX */
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	if (size <= 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	create_info.size = size;
	switch (buffer->target)
	{
		case GL_ARRAY_BUFFER:
		case GL_COPY_READ_BUFFER:
		case GL_COPY_WRITE_BUFFER:
			create_info.type = JKG_BUFFER_VERTEX;
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			create_info.type = JKG_BUFFER_INDEX;
			break;
		case GL_UNIFORM_BUFFER:
			create_info.type = JKG_BUFFER_UNIFORM;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	ret = ctx->jkg_op->buffer_alloc(ctx->jkg_ctx,
	                                &create_info,
	                                &jkg_buffer);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
	if (data)
	{
		ret = ctx->jkg_op->buffer_write(ctx->jkg_ctx,
		                                jkg_buffer,
		                                data,
		                                size,
		                                0);
		if (ret)
		{
			ctx->jkg_op->buffer_free(ctx->jkg_ctx, jkg_buffer);
			GL_SET_ERR(ctx, get_jkg_error(ret));
			return;
		}
	}
	if (buffer->buffer)
		ctx->jkg_op->buffer_free(ctx->jkg_ctx, buffer->buffer);
	buffer->dirty = GL_BUFFER_DIRTY_VERTEX
	              | GL_BUFFER_DIRTY_INDEX
	              | GL_BUFFER_DIRTY_UNIFORM;
	buffer->buffer = jkg_buffer;
	buffer->size = size;
}

void
glBufferData(GLenum target,
             GLsizeiptr size,
             const GLvoid *data,
             GLenum usage)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = get_buffer(ctx, target);
	if (!buffer)
		return;
	buffer_data(ctx, buffer, size, data, usage);
}

void
glNamedBufferData(GLuint id,
                  GLsizeiptr size,
                  const GLvoid *data,
                  GLenum usage)
{
	struct buffer *buffer;
	struct gl_ctx *ctx = g_ctx;

	buffer = object_get(ctx, id, OBJECT_BUFFER);
	if (!buffer)
		return;
	buffer_data(ctx, buffer, size, data, usage);
}

void
buffer_subdata(struct gl_ctx *ctx,
               struct buffer *buffer,
               GLintptr offset,
               GLsizeiptr size,
               const GLvoid *data)
{
	GLintptr end;
	int ret;

	if (!buffer->buffer)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (offset < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (size < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (__builtin_add_overflow(offset, size, &end)
	 || end > buffer->size)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	ret = ctx->jkg_op->buffer_write(ctx->jkg_ctx,
	                                buffer->buffer,
	                                data,
	                                size,
	                                offset);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glBufferSubData(GLenum target,
                GLintptr offset,
                GLsizeiptr size,
                const GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = get_buffer(ctx, target);
	if (!buffer)
		return;
	buffer_subdata(ctx, buffer, offset, size, data);
}

void
glNamedBufferSubData(GLuint id,
                     GLintptr offset,
                     GLsizeiptr size,
                     const GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = object_get(ctx, id, OBJECT_BUFFER);
	if (!buffer)
		return;
	buffer_subdata(ctx, buffer, offset, size, data);
}

static void
copy_buffer_data(struct gl_ctx *ctx,
                 struct buffer *dst,
                 struct buffer *src,
                 GLintptr dst_off,
                 GLintptr src_off,
                 GLsizeiptr size)
{
	GLintptr src_end;
	GLintptr dst_end;
	int ret;

	if (!src->buffer
	 || !dst->buffer)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (src_off < 0
	 || dst_off < 0
	 || size < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (__builtin_add_overflow(src_off, size, &src_end)
	 || __builtin_add_overflow(dst_off, size, &dst_end)
	 || src_end > src->size
	 || dst_end > dst->size)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (src == dst)
	{
		if ((src_off < dst_off && src_end > dst_off)
		 || (dst_off < src_off && dst_end > src_off))
		{
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
		}
	}
	ret = ctx->jkg_op->buffer_copy(ctx->jkg_ctx,
	                               dst->buffer,
	                               src->buffer,
	                               dst_off,
	                               src_off,
	                               size);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glCopyBufferSubData(GLenum readTarget,
                    GLenum writeTarget,
                    GLintptr readOffset,
                    GLintptr writeOffset,
                    GLsizeiptr size)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *src;
	struct buffer *dst;

	src = get_buffer(ctx, readTarget);
	if (!src)
		return;
	dst = get_buffer(ctx, writeTarget);
	if (!dst)
		return;
	copy_buffer_data(ctx, dst, src, writeOffset, readOffset, size);
}

void
glCopyNamedBufferSubData(GLuint readBuffer,
                         GLuint writeBuffer,
                         GLintptr readOffset,
                         GLintptr writeOffset,
                         GLsizeiptr size)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *src;
	struct buffer *dst;

	src = object_get(ctx, readBuffer, OBJECT_BUFFER);
	if (!src)
		return;
	dst = object_get(ctx, writeBuffer, OBJECT_BUFFER);
	if (!dst)
		return;
	copy_buffer_data(ctx, dst, src, writeOffset, readOffset, size);
}

static void
clear_buffer_subdata(struct gl_ctx *ctx,
                     struct buffer *buffer,
                     GLenum internalformat,
                     GLintptr offset,
                     GLsizeiptr size,
                     GLenum format,
                     GLenum type,
                     const GLvoid *data)
{
	/* XXX
	 * convert data to float[4]
	 * create jkg_op->buffer_clear()
	 */
}

void
glClearBufferSubData(GLenum target,
                     GLenum internalformat,
                     GLintptr offset,
                     GLsizeiptr size,
                     GLenum format,
                     GLenum type,
                     const GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = get_buffer(ctx, target);
	if (!buffer)
		return;
	clear_buffer_subdata(ctx, buffer, internalformat, offset, size, format, type, data);
}

void
glClearNamedBufferSubData(GLuint id,
                          GLenum internalformat,
                          GLintptr offset,
                          GLsizeiptr size,
                          GLenum format,
                          GLenum type,
                          const GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = object_get(ctx, id, OBJECT_BUFFER);
	if (!buffer)
		return;
	clear_buffer_subdata(ctx, buffer, internalformat, offset, size, format, type, data);
}

static void
get_buffer_subdata(struct gl_ctx *ctx,
                   struct buffer *buffer,
                   GLintptr offset,
                   GLsizeiptr size,
                   GLvoid *data)
{
	int ret;

	ret = ctx->jkg_op->buffer_read(ctx->jkg_ctx, buffer->buffer, data, size, offset);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
}

void
glGetBufferSubData(GLenum target,
                   GLintptr offset,
                   GLsizeiptr size,
                   GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = get_buffer(ctx, target);
	if (!buffer)
		return;
	get_buffer_subdata(ctx, buffer, offset, size, data);
}

void
glGetNamedBufferSubData(GLuint id,
                        GLintptr offset,
                        GLsizeiptr size,
                        GLvoid *data)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	buffer = object_get(ctx, id, OBJECT_BUFFER);
	if (!buffer)
		return;
	get_buffer_subdata(ctx, buffer, offset, size, data);
}

void
buffer_bind_range(struct gl_ctx *ctx,
                  struct buffer *buffer,
                  GLuint index,
                  GLintptr offset,
                  GLsizeiptr length)
{
	if (ctx->uniform_blocks[index].buffer != buffer)
	{
		ctx->uniform_blocks[index].buffer = buffer;
		ctx->dirty |= GL_CTX_DIRTY_UNIFORM_BLOCKS;
	}
	if (!buffer)
		return;
	if (ctx->uniform_blocks[index].offset != offset)
	{
		ctx->uniform_blocks[index].offset = offset;
		ctx->dirty |= GL_CTX_DIRTY_UNIFORM_BLOCKS;
	}
	if (ctx->uniform_blocks[index].length != length)
	{
		ctx->uniform_blocks[index].length = length;
		ctx->dirty |= GL_CTX_DIRTY_UNIFORM_BLOCKS;
	}
}

void
glBindBufferBase(GLenum target,
                 GLuint index,
                 GLuint id)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	if (target != GL_UNIFORM_BUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (index >= ctx->jkg_caps->max_uniform_blocks)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (id)
	{
		buffer = object_get(ctx, id, OBJECT_BUFFER);
		if (!buffer)
			return;
	}
	else
	{
		buffer = NULL;
	}
	buffer_bind_range(ctx, buffer, index, 0, -1);
}

void
glBindBuffersBase(GLenum target,
                  GLuint first,
                  GLsizei count,
                  const GLuint *buffers)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;
	GLuint last;

	if (target != GL_UNIFORM_BUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (count < 0
	 || __builtin_add_overflow(first, count, &last)
	 || last >= ctx->jkg_caps->max_uniform_blocks)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	for (GLsizei i = 0; i < count; ++i)
	{
		if (buffers[i])
		{
			buffer = object_get(ctx, buffers[i], OBJECT_BUFFER);
			if (!buffer)
				continue; /* XXX */
		}
		else
		{
			buffer = NULL;
		}
		buffer_bind_range(ctx, buffer, first + i, 0, -1);
	}
}

void
glBindBufferRange(GLenum target,
                  GLuint index,
                  GLuint id,
                  GLintptr offset,
                  GLsizeiptr size)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;

	if (target != GL_UNIFORM_BUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (index >= ctx->jkg_caps->max_uniform_blocks)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	if (offset < 0
	 || size < 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (id)
	{
		buffer = object_get(ctx, id, OBJECT_BUFFER);
		if (!buffer)
			return;
	}
	else
	{
		buffer = NULL;
	}
	buffer_bind_range(ctx, buffer, index, offset, size);
}

void
glBindBuffersRange(GLenum target,
                   GLuint first,
                   GLsizei count,
                   const GLuint *buffers,
                   const GLintptr *offsets,
                   const GLintptr *sizes)
{
	struct gl_ctx *ctx = g_ctx;
	struct buffer *buffer;
	GLuint last;

	if (target != GL_UNIFORM_BUFFER)
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (count < 0
	 || __builtin_add_overflow(first, count, &last)
	 || last >= ctx->jkg_caps->max_uniform_blocks)
	{
		GL_SET_ERR(ctx, GL_INVALID_OPERATION);
		return;
	}
	for (GLsizei i = 0; i < count; ++i)
	{
		if (offsets[i] < 0
		 || sizes[i] < 0)
		{
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
		}
		if (buffers[i])
		{
			buffer = object_get(ctx, buffers[i], OBJECT_BUFFER);
			if (!buffer)
				continue; /* XXX */
		}
		else
		{
			buffer = NULL;
		}
		buffer_bind_range(ctx, buffer, first + i, offsets[i], sizes[i]);
	}
}
