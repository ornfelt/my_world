#include "virgl.h"

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define VGLGCAPSETINFO 0x301
#define VGLGCAPSET     0x302
#define VGLSUBMIT      0x303
#define VGLCREATECTX   0x304
#define VGLFLUSH       0x305
#define VGLCREATERES   0x306
#define VGLTRANSFERIN  0x307
#define VGLTRANSFEROUT 0x308
#define VGLGID         0x309

struct vgl_capset_info
{
	uint32_t id;
	uint32_t max_version;
	uint32_t max_size;
};

struct vgl_submit
{
	void *data;
	uint32_t size;
};

struct vgl_create_res
{
	uint32_t target;
	uint32_t format;
	uint32_t bind;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t array_size;
	uint32_t last_level;
	uint32_t nr_samples;
	uint32_t flags;
	uint32_t bytes;
};

struct vgl_transfer
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;
	uint32_t h;
	uint32_t d;
	uint64_t offset;
	uint32_t resource;
	uint32_t level;
	uint32_t stride;
	uint32_t layer_stride;
};

struct vgl_create_ctx
{
	uint32_t fb;
};

int
vgl_flush(struct jkg_ctx *ctx)
{
	if (ioctl(ctx->fd, VGLFLUSH, 0))
		return -errno;
	return 0;
}

int
vgl_submit(struct jkg_ctx *ctx)
{
	struct vgl_submit req;
	int ret;

	req.data = ctx->cmd_buf.data;
	req.size = ctx->cmd_buf.pos;
	ret = ioctl(ctx->ctx_fd, VGLSUBMIT, &req);
	if (ret)
		return -errno;
	ctx->cmd_buf.pos = 0;
	return 0;
}

int
vgl_create_resource(struct jkg_ctx *ctx, struct virgl_res *res)
{
	struct vgl_create_res req;
	int ret;

	req.target = res->info.target;
	req.format = res->info.format;
	req.bind = res->info.bind;
	req.width = res->info.size.x;
	req.height = res->info.size.y;
	req.depth = res->info.size.z;
	req.array_size = res->info.array_size;
	req.last_level = res->info.last_level;
	req.nr_samples = res->info.nr_samples;
	req.flags = res->info.flags;
	req.bytes = res->info.bytes;
	ret = ioctl(ctx->ctx_fd, VGLCREATERES, &req);
	if (ret < 0)
		return -errno;
	res->fd = ret;
	if (res->info.bytes)
	{
		res->data = mmap(NULL,
		                 res->info.bytes,
		                 PROT_READ | PROT_WRITE,
		                 MAP_PRIVATE,
		                 ret,
		                 0);
		if (res->data == MAP_FAILED)
			return -errno;
	}
	else
	{
		res->data = NULL;
	}
	if (ioctl(res->fd, VGLGID, &res->id) < 0)
		return -errno;
	return 0;
}

int
vgl_transfer_in(struct jkg_ctx *ctx,
                struct virgl_res *res,
                const struct jkg_extent *offset,
                const struct jkg_extent *size,
                uint64_t buffer_offset,
                uint32_t level,
                uint32_t stride,
                uint32_t layer_stride)
{
	struct vgl_transfer req;

	(void)ctx;
	req.x = offset->x;
	req.y = offset->y;
	req.z = offset->z;
	req.w = size->x ? size->x : 1;
	req.h = size->y ? size->y : 1;
	req.d = size->z ? size->z : 1;
	req.offset = buffer_offset;
	req.resource = res->id;
	req.level = level;
	req.stride = stride;
	req.layer_stride = layer_stride;
	if (ioctl(res->fd, VGLTRANSFERIN, &req) < 0)
		return -errno;
	return 0;
}

int
vgl_transfer_out(struct jkg_ctx *ctx,
                 struct virgl_res *res,
                 const struct jkg_extent *offset,
                 const struct jkg_extent *size,
                 uint64_t buffer_offset,
                 uint32_t level,
                 uint32_t stride,
                 uint32_t layer_stride)
{
	struct vgl_transfer req;

	(void)ctx;
	req.x = offset->x;
	req.y = offset->y;
	req.z = offset->z;
	req.w = size->x ? size->x : 1;
	req.h = size->y ? size->y : 1;
	req.d = size->z ? size->z : 1;
	req.offset = buffer_offset;
	req.resource = res->id;
	req.level = level;
	req.stride = stride;
	req.layer_stride = layer_stride;
	if (ioctl(res->fd, VGLTRANSFEROUT, &req) < 0)
		return -errno;
	return 0;
}

int
vgl_create_context(struct jkg_ctx *ctx)
{
	struct vgl_create_ctx req;

	req.fb = ctx->fb;
	ctx->ctx_fd = ioctl(ctx->fd, VGLCREATECTX, &req);
	if (ctx->ctx_fd == -1)
		return -errno;
	ctx->fb = req.fb;
	return 0;
}

int
vgl_get_capset_info(struct jkg_ctx *ctx,
                    uint32_t *id,
                    uint32_t *version,
                    uint32_t *size)
{
	struct vgl_capset_info info;

	if (ioctl(ctx->fd, VGLGCAPSETINFO, &info))
		return -errno;
	*id = info.id;
	*version = info.max_version;
	*size = info.max_size;
	return 0;
}

int
vgl_get_capset(struct jkg_ctx *ctx,
               void *data,
               size_t size)
{
	(void)size; /* XXX use it */
	if (ioctl(ctx->fd, VGLGCAPSET, data) < 0)
		return -errno;
	return 0;
}
