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

int vgl_flush(struct env *env)
{
	if (ioctl(env->fd, VGLFLUSH, 0))
	{
		fprintf(stderr, "%s: ioctl(VGLFLUSH): %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

int vgl_submit(struct ctx *ctx)
{
	struct vgl_submit req;
	int ret;

	req.data = ctx->cmd_buf.data;
	req.size = ctx->cmd_buf.pos;
	ret = ioctl(ctx->fd, VGLSUBMIT, &req);
	if (ret)
	{
		fprintf(stderr, "%s: ioctl(VGLSUBMIT): %s\n",
		        ctx->env->progname, strerror(errno));
		return 1;
	}
	ctx->cmd_buf.pos = 0;
	return 0;
}

int vgl_create_resource(struct res *res)
{
	struct vgl_create_res req;
	int ret;

	req.target = res->target;
	req.format = res->format;
	req.bind = res->bind;
	req.width = res->width;
	req.height = res->height;
	req.depth = res->depth;
	req.array_size = res->array_size;
	req.last_level = res->last_level;
	req.nr_samples = res->nr_samples;
	req.flags = res->flags;
	req.bytes = res->bytes;
	ret = ioctl(res->ctx->fd, VGLCREATERES, &req);
	if (ret < 0)
	{
		fprintf(stderr, "%s: ioctl(VGLCREATERES): %s\n",
		        res->ctx->env->progname, strerror(errno));
		return 1;
	}
	res->fd = ret;
	res->data = mmap(NULL, res->bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE, ret, 0);
	if (res->data == MAP_FAILED)
	{
		fprintf(stderr, "%s: mmap failed\n", res->ctx->env->progname);
		return 1;
	}
	if (ioctl(res->fd, VGLGID, &res->id) < 0)
	{
		fprintf(stderr, "%s: ioctl(VGLVID): %s\n",
		        res->ctx->env->progname, strerror(errno));
		return 1;
	}
	return 0;
}

int vgl_transfer_out(struct res *res,
                     uint32_t x, uint32_t y, uint32_t z,
                     uint32_t w, uint32_t h, uint32_t d,
                     uint64_t offset, uint32_t level, uint32_t stride,
                     uint32_t layer_stride)
{
	struct vgl_transfer req;

	req.x = x;
	req.y = y;
	req.z = z;
	req.w = w;
	req.h = h;
	req.d = d;
	req.offset = offset;
	req.resource = res->id;
	req.level = level;
	req.stride = stride;
	req.layer_stride = layer_stride;
	if (ioctl(res->fd, VGLTRANSFEROUT, &req) < 0)
	{
		fprintf(stderr, "%s: ioctl(VGLTRANSFEROUT): %s\n",
		        res->ctx->env->progname, strerror(errno));
		return 1;
	}
	return 0;
}

int vgl_create_context(struct ctx *ctx)
{
	struct vgl_create_ctx req;

	req.fb = ctx->fb;
	ctx->fd = ioctl(ctx->env->fd, VGLCREATECTX, &req);
	if (ctx->fd == -1)
	{
		fprintf(stderr, "%s: ioctl(VGLCREATECTX): %s\n",
		        ctx->env->progname, strerror(errno));
		return 1;
	}
	ctx->fb = req.fb;
	return 0;
}

int vgl_get_capset_info(struct env *env, uint32_t *id, uint32_t *version,
                        uint32_t *size)
{
	struct vgl_capset_info info;

	if (ioctl(env->fd, VGLGCAPSETINFO, &info))
	{
		fprintf(stderr, "%s: ioctl(VGLGCAPSETINFO): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	*id = info.id;
	*version = info.max_version;
	*size = info.max_size;
	return 0;
}

int vgl_get_capset(struct env *env, void *data, size_t size)
{
	(void)size; /* XXX use it */
	if (ioctl(env->fd, VGLGCAPSET, data) < 0)
	{
		fprintf(stderr, "%s: ioctl(VGLGCAPSET): %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	return 0;
}
