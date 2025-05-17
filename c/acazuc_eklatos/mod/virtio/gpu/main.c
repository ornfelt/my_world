#define ENABLE_TRACE

#include "pci.h"
#include "gpu.h"

#include <errno.h>
#include <kmod.h>
#include <tty.h>
#include <fb.h>
#include <sg.h>

static int
synchronous_request(struct virtio_gpu *gpu,
                    struct virtq *queue,
                    struct sg_head *sg_read,
                    struct sg_head *sg_write)
{
	int ret;

	(void)gpu;
	ret = virtq_send(queue, sg_read, sg_write);
	if (ret < 0)
	{
		TRACE("virtio_gpu: failed to send request: %s",
		      strerror(ret));
		return ret;
	}
	virtq_notify(queue);
	while (1)
	{
		/* XXX timeout ? */
		uint16_t id;
		uint32_t len;
		ret = virtq_poll(queue, &id, &len);
		if (ret != -EAGAIN)
			break;
	}
	return ret;
}

int
cmd_get_display_info(struct virtio_gpu *gpu,
                     struct virtio_gpu_resp_display_info *display_info)
{
	struct virtio_gpu_resp_display_info *reply;
	struct virtio_gpu_ctrl_hdr *request;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_resp_display_info*)&request[1];
	memset(request, 0, sizeof(*request));
	request->type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->hdr.type != VIRTIO_GPU_RESP_OK_DISPLAY_INFO)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_GET_DISPLAY_INFO response: 0x%" PRIx32,
		       display_info->hdr.type);
		ret = -EXDEV;
		goto end;
	}
	memcpy(display_info, reply, sizeof(*display_info));
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_resource_create_2d(struct virtio_gpu *gpu,
                       uint32_t id,
                       uint32_t format,
                       uint32_t width,
                       uint32_t height)
{
	struct virtio_gpu_resource_create_2d *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	request->resource_id = id;
	request->format = format;
	request->width = width;
	request->height = height;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_RESOURCE_CREATE_2D response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_resource_create_3d(struct virtio_gpu *gpu,
                       uint32_t id,
                       uint32_t target,
                       uint32_t format,
                       uint32_t bind,
                       uint32_t width,
                       uint32_t height,
                       uint32_t depth,
                       uint32_t array_size,
                       uint32_t last_level,
                       uint32_t nr_samples,
                       uint32_t flags)
{
	struct virtio_gpu_resource_create_3d *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_3D;
	request->resource_id = id;
	request->target = target;
	request->format = format;
	request->bind = bind;
	request->width = width ? width : 1;
	request->height = height ? height : 1;
	request->depth = depth ? depth : 1;
	request->array_size = array_size ? array_size : 1;
	request->last_level = last_level;
	request->nr_samples = nr_samples;
	request->flags = flags;
	request->padding = 0;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_RESOURCE_CREATE_3D response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_resource_unref(struct virtio_gpu *gpu, uint32_t id)
{
	struct virtio_gpu_resource_unref *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_RESOURCE_UNREF;
	request->resource_id = id;
	request->padding = 0;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_RESOURCE_UNREF response: 0x%" PRIx32,
		       reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_resource_attach_backing(struct virtio_gpu *gpu,
                            uint32_t id,
                            struct page **pages,
                            size_t pages_count)
{
	struct virtio_gpu_resource_attach_backing *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	size_t reply_offset = sizeof(*request) + sizeof(struct virtio_gpu_mem_entry) * pages_count;
	int ret;

	if (reply_offset + sizeof(*reply) > VIRTIO_GPU_BUF_SIZE)
	{
		TRACE("virtio_gpu: request too big");
		return -ENOMEM;
	}
	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&((uint8_t*)gpu->buf->data)[reply_offset];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	request->resource_id = id;
	request->nr_entries = pages_count;
	for (size_t i = 0; i < pages_count; ++i)
	{
		struct virtio_gpu_mem_entry *mem_entry = &((struct virtio_gpu_mem_entry*)&request[1])[i];
		mem_entry->addr = pm_page_addr(pages[i]);
		mem_entry->length = PAGE_SIZE;
		mem_entry->padding = 0;
	}
	ret = sg_add_dma_buf(&sg_read, gpu->buf, reply_offset, 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), reply_offset);
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_set_scanout(struct virtio_gpu *gpu,
                uint32_t scanout,
                uint32_t id,
                uint32_t x,
                uint32_t y,
                uint32_t width,
                uint32_t height)
{
	struct virtio_gpu_set_scanout *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_SET_SCANOUT;
	request->r.x = x;
	request->r.y = y;
	request->r.width = width;
	request->r.height = height;
	request->scanout_id = scanout;
	request->resource_id = id;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_SET_SCANOUT response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_transfer_to_host_2d(struct virtio_gpu *gpu,
                        uint32_t id,
                        uint64_t offset,
                        uint32_t x,
                        uint32_t y,
                        uint32_t width,
                        uint32_t height)
{
	struct virtio_gpu_transfer_2d *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
	request->r.x = x;
	request->r.y = y;
	request->r.width = width;
	request->r.height = height;
	request->offset = offset;
	request->resource_id = id;
	request->padding = 0;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_transfer_to_host_3d(struct virtio_gpu *gpu,
                        uint32_t id,
                        uint32_t x,
                        uint32_t y,
                        uint32_t z,
                        uint32_t width,
                        uint32_t height,
                        uint32_t depth,
                        uint64_t offset,
                        uint32_t level,
                        uint32_t stride,
                        uint32_t layer_stride)
{
	struct virtio_gpu_transfer_3d *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D;
	request->b.x = x;
	request->b.y = y;
	request->b.z = z;
	request->b.width = width;
	request->b.height = height;
	request->b.depth = depth;
	request->offset = offset;
	request->resource_id = id;
	request->level = level;
	request->stride = stride;
	request->layer_stride = layer_stride;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_transfer_from_host_3d(struct virtio_gpu *gpu,
                          uint32_t id,
                          uint32_t x,
                          uint32_t y,
                          uint32_t z,
                          uint32_t width,
                          uint32_t height,
                          uint32_t depth,
                          uint64_t offset,
                          uint32_t level,
                          uint32_t stride,
                          uint32_t layer_stride)
{
	struct virtio_gpu_transfer_3d *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D;
	request->b.x = x;
	request->b.y = y;
	request->b.z = z;
	request->b.width = width;
	request->b.height = height;
	request->b.depth = depth;
	request->offset = offset;
	request->resource_id = id;
	request->level = level;
	request->stride = stride;
	request->layer_stride = layer_stride;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_resource_flush(struct virtio_gpu *gpu,
                   uint32_t id,
                   uint32_t x,
                   uint32_t y,
                   uint32_t width,
                   uint32_t height)
{
	struct virtio_gpu_resource_flush *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
	request->r.x = x;
	request->r.y = y;
	request->r.width = width;
	request->r.height = height;
	request->resource_id = id;
	request->padding = 0;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_RESOURCE_FLUSH response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_get_capset_info(struct virtio_gpu *gpu,
                    uint32_t id,
                    struct virtio_gpu_resp_capset_info *info)
{
	struct virtio_gpu_get_capset_info *request;
	struct virtio_gpu_resp_capset_info *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_resp_capset_info*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_GET_CAPSET_INFO;
	request->capset_index = id;
	request->padding = 0;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->hdr.type != VIRTIO_GPU_RESP_OK_CAPSET_INFO)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_GET_CAPSET_INFO response: 0x%" PRIx32,
		      reply->hdr.type);
		ret =-EXDEV;
		goto end;
	}
	memcpy(info, reply, sizeof(*info));
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_get_capset(struct virtio_gpu *gpu,
               uint32_t id,
               uint32_t version,
               void *data,
               size_t size)
{
	struct virtio_gpu_get_capset *request;
	struct virtio_gpu_resp_capset *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	if (sizeof(*request) + sizeof(*reply) + size > VIRTIO_GPU_BUF_SIZE)
	{
		TRACE("virtio_gpu: capset request too long");
		return -EOVERFLOW;
	}
	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_resp_capset*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_GET_CAPSET;
	request->capset_id = id;
	request->capset_version = version;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply) + size, sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->hdr.type != VIRTIO_GPU_RESP_OK_CAPSET)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_GET_CAPSET response: 0x%" PRIx32,
		      reply->hdr.type);
		ret = -EXDEV;
		goto end;
	}
	/* XXX test bufs[1].size ? */
	memcpy(data, reply->capset_data, size);
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_ctx_create(struct virtio_gpu *gpu,
               uint32_t id,
               uint8_t capset_id,
               const char *name)
{
	struct virtio_gpu_ctx_create *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	size_t name_len;
	int ret;

	name_len = strnlen(name, sizeof(request->debug_name) + 1);
	if (name_len > sizeof(request->debug_name))
	{
		TRACE("virtio_gpu: invaliod debug name length");
		return -EINVAL;
	}
	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_CTX_CREATE;
	request->hdr.ctx_id = id;
	request->nlen = strlen(name);
	memcpy(request->debug_name, name, name_len);
	request->context_init = capset_id;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_CTX_CREATE response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_ctx_destroy(struct virtio_gpu *gpu, uint32_t id)
{
	struct virtio_gpu_ctx_destroy *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_CTX_DESTROY;
	request->hdr.ctx_id = id;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_CTX_DESTROY response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_submit_3d(struct virtio_gpu *gpu,
              uint32_t context,
              const void *udata,
              size_t size)
{
	struct virtio_gpu_sumbit_3d *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	if (sizeof(*request) + sizeof(*reply) + size > VIRTIO_GPU_BUF_SIZE)
	{
		TRACE("virtio_gpu: submit 3d buffer overflow");
		return -EINVAL;
	}
	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(request, 0, sizeof(*request));
	request->hdr.type = VIRTIO_GPU_CMD_SUBMIT_3D;
	request->hdr.ctx_id = context;
	request->size = size;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_ubuf(&sg_read, udata, size);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_SUBMIT_3D response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_ctx_attach_resource(struct virtio_gpu *gpu,
                        uint32_t ctx,
                        uint32_t resource)
{
	struct virtio_gpu_cmd_resource *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE;
	request->hdr.ctx_id = ctx;
	request->resource_id = resource;
	request->padding = 0;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

int
cmd_ctx_detach_resource(struct virtio_gpu *gpu,
                        uint32_t ctx,
                        uint32_t resource)
{
	struct virtio_gpu_cmd_resource *request;
	struct virtio_gpu_ctrl_hdr *reply;
	struct sg_head sg_write;
	struct sg_head sg_read;
	int ret;

	sg_init(&sg_write);
	sg_init(&sg_read);
	mutex_lock(&gpu->mutex);
	request = gpu->buf->data;
	reply = (struct virtio_gpu_ctrl_hdr*)&request[1];
	memset(&request->hdr, 0, sizeof(request->hdr));
	request->hdr.type = VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE;
	request->hdr.ctx_id = ctx;
	request->resource_id = resource;
	request->padding = 0;
	ret = sg_add_dma_buf(&sg_read, gpu->buf, sizeof(*request), 0);
	if (ret)
		goto end;
	ret = sg_add_dma_buf(&sg_write, gpu->buf, sizeof(*reply), sizeof(*request));
	if (ret)
		goto end;
	ret = synchronous_request(gpu, &gpu->dev.queues[0], &sg_read, &sg_write);
	if (ret < 0)
		goto end;
	if (reply->type != VIRTIO_GPU_RESP_OK_NODATA)
	{
		TRACE("virtio_gpu: invalid VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE response: 0x%" PRIx32,
		      reply->type);
		ret = -EXDEV;
		goto end;
	}
	ret = 0;

end:
	mutex_unlock(&gpu->mutex);
	sg_free(&sg_write);
	sg_free(&sg_read);
	return ret;
}

static int
gpu_fb_flush(struct fb *fb,
             uint32_t x,
             uint32_t y,
             uint32_t width,
             uint32_t height)
{
	struct virtio_gpu *gpu = fb->userdata;
	uint32_t right;
	uint32_t bottom;
	int ret;

	if (x >= gpu->fb->width
	 || y >= gpu->fb->height
	 || __builtin_add_overflow(x, width, &right)
	 || right > gpu->fb->width
	 || __builtin_add_overflow(y, height, &bottom)
	 || bottom > gpu->fb->height)
		return -EINVAL;
	ret = cmd_transfer_to_host_2d(gpu, gpu->framebuffer.id,
	                              gpu->fb->pitch * y + gpu->fb->bpp / 8 * x,
	                              x, y, width, height);
	if (ret)
	{
		TRACE("virtio_gpu: failed to transfer framebuffer memory");
		return ret;
	}
	ret = cmd_resource_flush(gpu, gpu->framebuffer.id,
	                         x, y, width, height);
	if (ret)
	{
		TRACE("virtio_gpu: failed to flush framebuffer");
		return ret;
	}
	return 0;
}

const struct fb_op
fb_op =
{
	.flush = gpu_fb_flush,
};

int
get_resource_id(struct virtio_gpu *gpu, uint32_t *id)
{
	/* XXX bitmap? */
	*id = __atomic_add_fetch(&gpu->resource_id, 1, __ATOMIC_SEQ_CST);
	return 0;
}

void
framebuffer_free(struct virtio_gpu *gpu, struct virtio_gpu_fb *fb)
{
	int ret;

	if (fb->id)
	{
		ret = cmd_resource_unref(gpu, fb->id);
		if (ret)
			TRACE("virtio_gpu: failed to unref resource");
	}
	if (fb->pages)
	{
		for (size_t i = 0; i < fb->pages_count; ++i)
			pm_free_page(fb->pages[i]);
		free(fb->pages);
	}
}

int
framebuffer_alloc(struct virtio_gpu *gpu,
                  struct virtio_gpu_fb *fb,
                  uint32_t format,
                  uint32_t width,
                  uint32_t height)
{
	uint32_t id;
	int ret;

	ret = get_resource_id(gpu, &id);
	if (ret)
	{
		TRACE("virtio_gpu: resource id allocation failed");
		return ret;
	}
	ret = cmd_resource_create_2d(gpu, id, format, width, height);
	if (ret)
	{
		TRACE("virtio_gpu: resource creation failed");
		return ret;
	}
	fb->id = id;
	fb->format = format;
	fb->width = width;
	fb->height = height;
	fb->size = width * height * 4; /* XXX bpp */
	fb->size += PAGE_SIZE - 1;
	fb->size -= fb->size % PAGE_SIZE;
	fb->pages_count = fb->size / PAGE_SIZE;
	fb->pages = malloc(sizeof(*fb->pages) * fb->pages_count, M_ZERO);
	if (!fb->pages)
	{
		TRACE("virtio_gpu: framebuffer pages allocation failed");
		framebuffer_free(gpu, fb);
		return ret;
	}
	for (size_t i = 0; i < fb->pages_count; ++i)
	{
		ret = pm_alloc_page(&fb->pages[i]);
		if (ret)
		{
			TRACE("virtio_gpu: framebuffer page allocation failed");
			framebuffer_free(gpu, fb);
			return ret;
		}
	}
	ret = cmd_resource_attach_backing(gpu, fb->id, fb->pages, fb->pages_count);
	if (ret)
	{
		TRACE("virtio_gpu: failed to attach framebuffer resources");
		framebuffer_free(gpu, fb);
		return ret;
	}
	return 0;
}

void
print_gpu_cfg(struct uio *uio, struct pci_map *gpu_cfg)
{
	uprintf(uio, "num scanouts: %" PRIu32 "\n",
	        pci_r32(gpu_cfg, VIRTIO_GPU_C_NUM_SCANOUTS));
	uprintf(uio, "num capsets: %" PRIu32 "\n",
	        pci_r32(gpu_cfg, VIRTIO_GPU_C_NUM_CAPSETS));
}

void
print_display_info(struct uio *uio,
                   struct virtio_gpu_resp_display_info *display_info)
{
	uprintf(uio, "x: %" PRIu32 "\n", display_info->pmodes[0].r.x);
	uprintf(uio, "y: %" PRIu32 "\n", display_info->pmodes[0].r.y);
	uprintf(uio, "width: %" PRIu32 "\n", display_info->pmodes[0].r.width);
	uprintf(uio, "height: %" PRIu32 "\n", display_info->pmodes[0].r.height);
	uprintf(uio, "enabled: %" PRIu32 "\n", display_info->pmodes[0].enabled);
	uprintf(uio, "flags: %" PRIx32 "\n", display_info->pmodes[0].flags);
}

void
virtio_gpu_delete(struct virtio_gpu *gpu)
{
	if (!gpu)
		return;
	fb_free(gpu->fb);
	framebuffer_free(gpu, &gpu->framebuffer);
	free(gpu->capsets);
	pci_unmap(gpu->dev.device, gpu->gpu_cfg);
	virtio_dev_destroy(&gpu->dev);
	mutex_destroy(&gpu->mutex);
	free(gpu);
}

int
init_pci(struct pci_device *device, void *userdata)
{
	struct virtio_gpu *gpu = NULL;
	uint8_t features[(VIRTIO_F_RING_RESET + 7) / 8];
	int ret;

	(void)userdata;
	gpu = malloc(sizeof(*gpu), M_ZERO);
	if (!gpu)
	{
		TRACE("virtio_gpu: allocation failed\n");
		ret = -ENOMEM;
		goto err;
	}
	mutex_init(&gpu->mutex, 0);
	memset(features, 0, sizeof(features));
	features[VIRTIO_GPU_F_VIRGL / 8] |= 1 << (VIRTIO_GPU_F_VIRGL % 8);
	ret = virtio_dev_init(&gpu->dev, device, features, VIRTIO_F_RING_RESET);
	if (ret)
	{
		TRACE("virtio_gpu: failed to init device");
		goto err;
	}
	if (gpu->dev.queues_nb < 2)
	{
		TRACE("virtio_gpu: no queues");
		ret = -EINVAL;
		goto err;
	}
	ret = virtio_get_cfg(device, VIRTIO_PCI_CAP_DEVICE_CFG,
	                     &gpu->gpu_cfg, 0x40, NULL);
	if (ret)
	{
		TRACE("virtio_gpu: failed to get device cfg");
		goto err;
	}
	if (pci_r32(gpu->gpu_cfg, VIRTIO_GPU_C_NUM_SCANOUTS) < 1)
	{
		TRACE("virtio_gpu: no scanouts");
		ret = -EXDEV;
		goto err;
	}
#if 0
	print_gpu_cfg(NULL, &gpu->gpu_cfg);
#endif
	ret = dma_buf_alloc(VIRTIO_GPU_BUF_SIZE, 0, &gpu->buf);
	if (ret)
	{
		TRACE("virtio_gpu: buffer allocation failed");
		goto err;
	}
	virtio_dev_init_end(&gpu->dev);
	ret = cmd_get_display_info(gpu, &gpu->display_info);
	if (ret)
	{
		TRACE("virtio_gpu: failed to get display info");
		goto err;
	}
	ret = framebuffer_alloc(gpu, &gpu->framebuffer,
	                        VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM,
	                        gpu->display_info.pmodes[0].r.width,
	                        gpu->display_info.pmodes[0].r.height);
	if (ret)
	{
		TRACE("virtio_gpu: failed to create framebuffer");
		goto err;
	}
	ret = cmd_set_scanout(gpu, 0, gpu->framebuffer.id, 0, 0,
	                      gpu->framebuffer.width,
	                      gpu->framebuffer.height);
	if (ret)
	{
		TRACE("virtio_gpu: failed to set scanout");
		goto err;
	}
	ret = fb_alloc(&fb_op, &gpu->fb);
	if (ret)
	{
		TRACE("virtio_gpu: failed to create fb");
		goto err;
	}
	ret = fb_update(gpu->fb,
	                gpu->framebuffer.width,
	                gpu->framebuffer.height,
	                FB_FMT_B8G8R8A8,
	                gpu->framebuffer.width * 4,
	                32, gpu->framebuffer.pages,
	                gpu->framebuffer.pages_count,
	                0);
	if (ret)
	{
		TRACE("virtio_gpu: failed to update fb");
		goto err;
	}
	if (virtio_dev_has_feature(&gpu->dev, VIRTIO_GPU_F_VIRGL))
	{
		gpu->ncapset = pci_r32(gpu->gpu_cfg, VIRTIO_GPU_C_NUM_CAPSETS);
		gpu->capsets = malloc(sizeof(*gpu->capsets) * gpu->ncapset, 0);
		if (!gpu->capsets)
		{
			TRACE("virtio_gpu: capset allocation failed");
			goto err;
		}
		for (size_t i = 0; i < gpu->ncapset; ++i)
		{
			ret = cmd_get_capset_info(gpu, i, &gpu->capsets[i]);
			if (ret)
			{
				TRACE("virtio_gpu: failed to get capset info");
				goto err;
			}
		}
		uint32_t virgl_id = UINT32_MAX;
		uint32_t virgl2_id = UINT32_MAX;
		for (size_t i = 0; i < gpu->ncapset; ++i)
		{
			switch (gpu->capsets[i].capset_id)
			{
				case VIRTIO_GPU_CAPSET_VIRGL:
					virgl_id = i;
					break;
				case VIRTIO_GPU_CAPSET_VIRGL2:
					virgl2_id = i;
					break;
			}
		}
		if (virgl2_id != UINT32_MAX)
			virgl_init(gpu, &gpu->capsets[virgl2_id]);
		else if (virgl_id != UINT32_MAX)
			virgl_init(gpu, &gpu->capsets[virgl_id]);
	}
	gpu->fb->userdata = gpu;
	ret = vtty_alloc("tty0", 0, gpu->fb, &gpu->tty);
	if (!ret)
		curtty = gpu->tty;
	else
		TRACE("virtio_gpu: failed to create tty: %s", strerror(ret));
	return 0;

err:
	virtio_gpu_delete(gpu);
	return ret;
}

int
init(void)
{
	pci_probe(0x1AF4, 0x1050, init_pci, NULL);
	return 0;
}

void
fini(void)
{
}

struct kmod_info
kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "virtio/gpu",
	.init = init,
	.fini = fini,
};
