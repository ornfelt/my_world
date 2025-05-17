#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

static int
get_bind(enum jkg_buffer_type type)
{
	switch (type)
	{
		default:
			assert(!"unknown buffer type");
			/* FALLTHROUGH */
		case JKG_BUFFER_VERTEX:
			return VIRGL_BIND_VERTEX_BUFFER;
		case JKG_BUFFER_INDEX:
			return VIRGL_BIND_INDEX_BUFFER;
		case JKG_BUFFER_UNIFORM:
			return VIRGL_BIND_CONSTANT_BUFFER;
	}
}

int
virgl_buffer_alloc(struct jkg_ctx *ctx,
                   const struct jkg_buffer_create_info *create_info,
                   struct jkg_buffer **bufferp)
{
	struct virgl_res_info res_info;
	struct jkg_buffer *buffer;
	int ret;

	buffer = calloc(1, sizeof(*buffer));
	if (!buffer)
		return -ENOMEM;
	res_info.target = PIPE_BUFFER;
	res_info.format = VIRGL_FORMAT_NONE;
	res_info.bind = get_bind(create_info->type);
	res_info.size.x = create_info->size;
	res_info.size.y = 0;
	res_info.size.z = 0;
	res_info.array_size = 1;
	res_info.last_level = 0;
	res_info.nr_samples = 0;
	/* XXX VIRGL_RESOURCE_FLAG_MAP_PERSISTENT | VIRGL_RESOURCE_FLAG_MAP_COHERENT */
	res_info.flags = 0;
	res_info.bytes = create_info->size;
	ret = virgl_res_alloc(ctx, &res_info, &buffer->res);
	if (ret)
		goto err;
	*bufferp = buffer;
	return 0;

err:
	free(buffer);
	return ret;
}

int
virgl_buffer_read(struct jkg_ctx *ctx,
                  struct jkg_buffer *buffer,
                  void *data,
                  uint32_t size,
                  uint32_t offset)
{
	struct jkg_extent offset_extent = {offset, 0, 0};
	struct jkg_extent size_extent = {size, 0, 0};
	int ret;

	ret = virgl_flush(ctx);
	if (ret)
		return ret;
	ret = vgl_transfer_in(ctx,
	                      buffer->res,
	                      &offset_extent,
	                      &size_extent,
	                      offset,
	                      0,
	                      0,
	                      0);
	if (ret)
		return ret;
	memcpy(data, &((uint8_t*)buffer->res->data)[offset], size);
	return 0;
}

int
virgl_buffer_write(struct jkg_ctx *ctx,
                   struct jkg_buffer *buffer,
                   const void *data,
                   uint32_t size,
                   uint32_t offset)
{
	struct jkg_extent offset_extent = {offset, 0, 0};
	struct jkg_extent size_extent = {size, 0, 0};
	int ret;

	ret = virgl_flush(ctx);
	if (ret)
		return ret;
	memcpy(&((uint8_t*)buffer->res->data)[offset], data, size);
	ret = vgl_transfer_out(ctx,
	                       buffer->res,
	                       &offset_extent,
	                       &size_extent,
	                       offset,
	                       0,
	                       0,
	                       0);
	if (ret)
		return ret;
	return 0;
}

int
virgl_buffer_copy(struct jkg_ctx *ctx,
                  struct jkg_buffer *dst,
                  struct jkg_buffer *src,
                  uint32_t dst_off,
                  uint32_t src_off,
                  uint32_t size)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_RESOURCE_COPY_REGION,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_CMD_RESOURCE_COPY_REGION_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_CMD_RCR_DST_RES_HANDLE] = src->res->id;
	request[VIRGL_CMD_RCR_DST_LEVEL] = 0;
	request[VIRGL_CMD_RCR_DST_X] = dst_off;
	request[VIRGL_CMD_RCR_DST_Y] = 0;
	request[VIRGL_CMD_RCR_DST_Z] = 0;
	request[VIRGL_CMD_RCR_SRC_RES_HANDLE] = dst->res->id;
	request[VIRGL_CMD_RCR_SRC_LEVEL] = 0;
	request[VIRGL_CMD_RCR_SRC_X] = src_off;
	request[VIRGL_CMD_RCR_SRC_Y] = 0;
	request[VIRGL_CMD_RCR_SRC_Z] = 0;
	request[VIRGL_CMD_RCR_SRC_W] = size;
	request[VIRGL_CMD_RCR_SRC_H] = 0;
	request[VIRGL_CMD_RCR_SRC_D] = 0;
	return 0;
}

void
virgl_buffer_free(struct jkg_ctx *ctx,
                  struct jkg_buffer *buffer)
{
	virgl_res_free(ctx, buffer->res);
	free(buffer);
}

static uint32_t
get_index_size(enum jkg_index_type type)
{
	switch (type)
	{
		case JKG_INDEX_UINT8:
			return 1;
		default:
			assert(!"unknown index type");
			/* FALLTHROUGH */
		case JKG_INDEX_UINT16:
			return 2;
		case JKG_INDEX_UINT32:
			return 4;
	}
}

int
virgl_set_index_buffer(struct jkg_ctx *ctx,
                       struct jkg_buffer *buffer,
                       enum jkg_index_type type,
                       uint32_t offset)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_INDEX_BUFFER,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_INDEX_BUFFER_SIZE(1),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_INDEX_BUFFER_HANDLE] = buffer->res->id;
	request[VIRGL_SET_INDEX_BUFFER_INDEX_SIZE] = get_index_size(type);
	request[VIRGL_SET_INDEX_BUFFER_OFFSET] = offset;
	return 0;
}

int
virgl_set_vertex_buffers(struct jkg_ctx *ctx,
                         const struct jkg_vertex_buffer *vertex_buffers,
                         uint32_t count)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_VERTEX_BUFFERS,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_VERTEX_BUFFERS_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	for (size_t i = 0; i < count; ++i)
	{
		request[VIRGL_SET_VERTEX_BUFFER_STRIDE(i)] = vertex_buffers[i].stride;
		request[VIRGL_SET_VERTEX_BUFFER_OFFSET(i)] = vertex_buffers[i].offset;
		request[VIRGL_SET_VERTEX_BUFFER_HANDLE(i)] = vertex_buffers[i].buffer ? vertex_buffers[i].buffer->res->id : 0;
	}
	return 0;
}

int
virgl_set_uniform_blocks(struct jkg_ctx *ctx,
                         const struct jkg_uniform_block *uniform_blocks,
                         uint32_t count)
{
	uint32_t *request;
	int ret;

	for (uint32_t i = 0; i < count; ++i)
	{
		ret = virgl_req_alloc(ctx,
		                      VIRGL_CCMD_SET_UNIFORM_BUFFER,
		                      VIRGL_OBJECT_NULL,
		                      VIRGL_SET_UNIFORM_BUFFER_SIZE,
		                      &request);
		if (ret)
			return ret;
		request[VIRGL_SET_UNIFORM_BUFFER_SHADER_TYPE] = PIPE_SHADER_VERTEX;
		request[VIRGL_SET_UNIFORM_BUFFER_INDEX] = i + 1;
		request[VIRGL_SET_UNIFORM_BUFFER_OFFSET] = uniform_blocks[i].offset;
		request[VIRGL_SET_UNIFORM_BUFFER_LENGTH] = uniform_blocks[i].length;
		request[VIRGL_SET_UNIFORM_BUFFER_RES_HANDLE] = uniform_blocks[i].buffer ? uniform_blocks[i].buffer->res->id : 0;
		ret = virgl_req_alloc(ctx,
		                      VIRGL_CCMD_SET_UNIFORM_BUFFER,
		                      VIRGL_OBJECT_NULL,
		                      VIRGL_SET_UNIFORM_BUFFER_SIZE,
		                      &request);
		if (ret)
			return ret;
		request[VIRGL_SET_UNIFORM_BUFFER_SHADER_TYPE] = PIPE_SHADER_FRAGMENT;
		request[VIRGL_SET_UNIFORM_BUFFER_INDEX] = i + 1;
		request[VIRGL_SET_UNIFORM_BUFFER_OFFSET] = uniform_blocks[i].offset;
		request[VIRGL_SET_UNIFORM_BUFFER_LENGTH] = uniform_blocks[i].length;
		request[VIRGL_SET_UNIFORM_BUFFER_RES_HANDLE] = uniform_blocks[i].buffer ? uniform_blocks[i].buffer->res->id : 0;
	}
	return 0;
}
