#include "gjit.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
gjit_buffer_alloc(struct jkg_ctx *ctx,
                  const struct jkg_buffer_create_info *create_info,
                  struct jkg_buffer **bufferp)
{
	struct jkg_buffer *buffer;

	(void)ctx;
	buffer = malloc(sizeof(*buffer));
	if (!buffer)
		return -ENOMEM;
	buffer->data = malloc(create_info->size);
	if (!buffer->data)
	{
		free(buffer);
		return -ENOMEM;
	}
	buffer->size = create_info->size;
	*bufferp = buffer;
	return 0;
}

int
gjit_buffer_read(struct jkg_ctx *ctx,
                 struct jkg_buffer *buffer,
                 void *data,
                 uint32_t size,
                 uint32_t offset)
{
	(void)ctx;
	memcpy(data, &((uint8_t*)buffer->data)[offset], size);
	return 0;
}

int
gjit_buffer_write(struct jkg_ctx *ctx,
                  struct jkg_buffer *buffer,
                  const void *data,
                  uint32_t size,
                  uint32_t offset)
{
	(void)ctx;
	memcpy(&((uint8_t*)buffer->data)[offset], data, size);
	return 0;
}

int
gjit_buffer_copy(struct jkg_ctx *ctx,
                 struct jkg_buffer *dst,
                 struct jkg_buffer *src,
                 uint32_t dst_off,
                 uint32_t src_off,
                 uint32_t size)
{
	(void)ctx;
	memcpy(&((uint8_t*)dst->data)[dst_off],
	       &((uint8_t*)src->data)[src_off],
	       size);
	return 0;
}

void
gjit_buffer_free(struct jkg_ctx *ctx, struct jkg_buffer *buffer)
{
	(void)ctx;
	free(buffer->data);
	free(buffer);
}

int
gjit_set_index_buffer(struct jkg_ctx *ctx,
                      struct jkg_buffer *buffer,
                      enum jkg_index_type type,
                      uint32_t offset)
{
	ctx->index_buffer = buffer;
	ctx->index_type = type;
	ctx->index_offset = offset;
	return 0;
}

int
gjit_set_vertex_buffers(struct jkg_ctx *ctx,
                        const struct jkg_vertex_buffer *vertex_buffers,
                        uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		ctx->vertex_buffers[i] = vertex_buffers[i];
	return 0;
}

int
gjit_set_uniform_blocks(struct jkg_ctx *ctx,
                        const struct jkg_uniform_block *uniform_blocks,
                        uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		ctx->uniform_blocks[i] = uniform_blocks[i];
	return 0;
}
