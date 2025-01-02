#define ENABLE_TRACE

#include "gpu.h"

#include <file.h>
#include <proc.h>
#include <std.h>
#include <vfs.h>
#include <uio.h>
#include <cpu.h>
#include <mem.h>
#include <fb.h>

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

struct virgl_res;
struct virgl_ctx;
struct virgl;

struct virgl_res
{
	struct virgl_ctx *ctx;
	refcount_t refcount;
	struct page **pages;
	size_t pages_count;
	size_t bytes;
	uint32_t id;
	uint32_t target;
	uint32_t format;
	uint32_t bind;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	struct cdev *cdev;
	TAILQ_ENTRY(virgl_res) chain;
};

struct virgl_ctx
{
	struct virgl *virgl;
	refcount_t refcount;
	uint32_t id;
	struct cdev *cdev;
	TAILQ_HEAD(, virgl_res) res;
	TAILQ_ENTRY(virgl_ctx) chain;
};

struct virgl
{
	struct virtio_gpu *gpu;
	uint8_t *capset;
	struct virtio_gpu_resp_capset_info capset_info;
	struct cdev *cdev;
	uint32_t ctx_id;
	TAILQ_HEAD(, virgl_ctx) ctx;
};

int virgl_res_fault(struct vm_zone *zone, off_t off, struct page **page);

static const struct vm_zone_op res_vm_op =
{
	.fault = virgl_res_fault,
};

int virgl_res_open(struct file *file, struct node *node);
int virgl_res_mmap(struct file *file, struct vm_zone *zone);
int virgl_res_ioctl(struct file *file, unsigned long request, uintptr_t data);
int virgl_res_release(struct file *file);

static const struct file_op res_fop =
{
	.open = virgl_res_open,
	.mmap = virgl_res_mmap,
	.ioctl = virgl_res_ioctl,
	.release = virgl_res_release,
};

int virgl_ctx_open(struct file *file, struct node *node);
int virgl_ctx_ioctl(struct file *file, unsigned long request, uintptr_t data);
int virgl_ctx_release(struct file *file);

static const struct file_op ctx_fop =
{
	.open = virgl_ctx_open,
	.ioctl = virgl_ctx_ioctl,
	.release = virgl_ctx_release,
};

int virgl_ioctl(struct file *file, unsigned long request, uintptr_t data);

static const struct file_op fop =
{
	.ioctl = virgl_ioctl,
};

static void virgl_res_free(struct virgl_res *res)
{
	if (!res)
		return;
	if (refcount_dec(&res->refcount))
		return;
	/* XXX take some lock to avoid race condition */
	TAILQ_REMOVE(&res->ctx->res, res, chain);
	if (res->id)
	{
		if (cmd_resource_unref(res->ctx->virgl->gpu, res->id))
			TRACE("virgl: resource unref failed");
	}
	if (res->pages)
	{
		for (size_t i = 0; i < res->pages_count; ++i)
			pm_free_page(res->pages[i]);
		free(res->pages);
	}
	cdev_free(res->cdev);
	free(res);
}

static int virgl_res_alloc(struct virgl_ctx *ctx, size_t bytes,
                           struct virgl_res **resp)
{
	struct virgl_res *res = NULL;
	char name[16];
	int ret;

	res = malloc(sizeof(*res), M_ZERO);
	if (!res)
	{
		TRACE("virgl: resource allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	refcount_init(&res->refcount, 1);
	res->ctx = ctx;
	res->bytes = bytes;
	TAILQ_INSERT_TAIL(&ctx->res, res, chain);
	if (bytes)
	{
		res->pages_count = (bytes + PAGE_SIZE - 1) / PAGE_SIZE;
		res->pages = malloc(sizeof(*res->pages) * res->pages_count, M_ZERO);
		if (!res->pages)
		{
			TRACE("virgl: pages allocation failed");
			goto err;
		}
		for (size_t i = 0; i < res->pages_count; ++i)
		{
			ret = pm_alloc_page(&res->pages[i]);
			if (ret)
			{
				TRACE("virgl: page allocation failed");
				goto err;
			}
		}
	}
	ret = get_resource_id(ctx->virgl->gpu, &res->id);
	if (ret)
	{
		TRACE("virgl: resource id allocation failed");
		goto err;
	}
	snprintf(name, sizeof(name), "virgl_res%" PRIu32, res->id);
	ret = cdev_alloc(name, 0, 0, 0000, makedev(14, res->id), &res_fop, &res->cdev);
	if (ret)
	{
		TRACE("virgl: failed to create resource cdev");
		goto err;
	}
	res->cdev->userdata = res;
	*resp = res;
	return 0;

err:
	virgl_res_free(res);
	return ret;
}

static uint32_t alloc_ctx_id(struct virgl *virgl)
{
	/* XXX bitmap; max 16 bits (because of cdev) */
	return __atomic_add_fetch(&virgl->ctx_id, 1, __ATOMIC_SEQ_CST);
}

void virgl_ctx_free(struct virgl_ctx *ctx)
{
	struct virgl_res *res;

	if (!ctx)
		return;
	if (refcount_dec(&ctx->refcount))
		return;
	/* XXX lock something to avoid race condition */
	TAILQ_REMOVE(&ctx->virgl->ctx, ctx, chain);
	if (ctx->id && cmd_ctx_destroy(ctx->virgl->gpu, ctx->id))
		TRACE("virgl: failed to destroy ctx");
	while ((res = TAILQ_FIRST(&ctx->res)))
		virgl_res_free(res);
	cdev_free(ctx->cdev);
	free(ctx);
}

int virgl_ctx_alloc(struct virgl *virgl, struct virgl_ctx **ctxp)
{
	struct virgl_ctx *ctx = NULL;
	char name[16];
	int ret;

	ctx = malloc(sizeof(*ctx), M_ZERO);
	if (!ctx)
	{
		TRACE("virgl: ctx allocation failed");
		return -ENOMEM;
	}
	refcount_init(&ctx->refcount, 1);
	ctx->virgl = virgl;
	ctx->id = alloc_ctx_id(virgl);
	TAILQ_INIT(&ctx->res);
	TAILQ_INSERT_TAIL(&virgl->ctx, ctx, chain);
	ret = cmd_ctx_create(virgl->gpu, ctx->id, virgl->capset_info.capset_id, "main");
	if (ret)
	{
		ctx->id = 0;
		TRACE("virgl: failed to create context");
		goto err;
	}
	snprintf(name, sizeof(name), "virgl_ctx%" PRIu32, ctx->id);
	ret = cdev_alloc(name, 0, 0, 0000, makedev(13, ctx->id), &ctx_fop, &ctx->cdev);
	if (ret)
	{
		TRACE("virgl: failed to create context cdev");
		goto err;
	}
	ctx->cdev->userdata = ctx;
	*ctxp = ctx;
	return 0;

err:
	virgl_ctx_free(ctx);
	return ret;
}

static struct virgl *getvirgl(struct file *file)
{
	if (file->cdev)
		return file->cdev->userdata;
	if (file->node && S_ISCHR(file->node->attr.mode))
		return file->node->cdev->userdata;
	return NULL;
}

static struct virgl_ctx *getctx(struct file *file)
{
	if (file->cdev)
		return file->cdev->userdata;
	if (file->node && S_ISCHR(file->node->attr.mode))
		return file->node->cdev->userdata;
	return NULL;
}

static struct virgl_res *getres(struct file *file)
{
	if (file->cdev)
		return file->cdev->userdata;
	if (file->node && S_ISCHR(file->node->attr.mode))
		return file->node->cdev->userdata;
	return NULL;
}

static int vgl_get_capset_info(struct virgl *virgl, void *udata)
{
	struct thread *thread = curcpu()->thread;
	struct vgl_capset_info req;

	req.id = virgl->capset_info.capset_id;
	req.max_version = virgl->capset_info.capset_max_version;
	req.max_size = virgl->capset_info.capset_max_size;
	return vm_copyout(thread->proc->vm_space, udata, &req, sizeof(req));
}

static int vgl_get_capset(struct virgl *virgl, void *udata)
{
	struct thread *thread = curcpu()->thread;

	return vm_copyout(thread->proc->vm_space, udata,
	                  &virgl->capset,
	                  virgl->capset_info.capset_max_size);
}

static int vgl_submit(struct virgl_ctx *ctx, void *udata)
{
	struct thread *thread = curcpu()->thread;
	struct vgl_submit req;
	int ret;

	ret = vm_copyin(thread->proc->vm_space, &req, udata, sizeof(req));
	if (ret)
		return ret;
	return cmd_submit_3d(ctx->virgl->gpu, ctx->id, req.data, req.size);
}

static int vgl_create_ctx(struct virgl *virgl, void *udata)
{
	struct thread *thread = curcpu()->thread;
	struct vgl_create_ctx req;
	struct virgl_ctx *ctx = NULL;
	struct file *file = NULL;
	int ret;

	ret = vm_copyin(thread->proc->vm_space, &req, udata, sizeof(req));
	if (ret)
		return ret;
	if (!req.fb)
	{
		req.fb = virgl->gpu->framebuffer.id;
		ret = vm_copyout(thread->proc->vm_space, udata, &req, sizeof(req));
		if (ret)
			goto err;
	}
	ret = virgl_ctx_alloc(virgl, &ctx);
	if (ret)
		goto err;
	ret = cmd_ctx_attach_resource(ctx->virgl->gpu, ctx->id, req.fb);
	if (ret)
	{
		TRACE("virgl: failed to attach framebuffer");
		goto err;
	}
	TAILQ_INSERT_TAIL(&virgl->ctx, ctx, chain);
	if (ret)
		goto err;
	ret = file_fromcdev(ctx->cdev, O_RDWR, &file);
	if (ret)
	{
		TRACE("virgl: failed to create file");
		goto err;
	}
	ret = proc_allocfd(thread->proc, file, 0);
	file_free(file);
	if (ret < 0)
	{
		TRACE("virgl: failed to allocate fd");
		goto err;
	}
	return ret;

err:
	virgl_ctx_free(ctx);
	return ret;
}

static int vgl_flush(struct virgl *virgl, void *udata)
{
	(void)udata;
	return cmd_resource_flush(virgl->gpu,
	                          virgl->gpu->framebuffer.id,
	                          0,
	                          0,
	                          virgl->gpu->fb->width,
	                          virgl->gpu->fb->height);
}

static int vgl_create_res(struct virgl_ctx *ctx, void *udata)
{
	struct thread *thread = curcpu()->thread;
	struct vgl_create_res req;
	struct virgl_res *res = NULL;
	struct file *file;
	int ret;

	ret = vm_copyin(thread->proc->vm_space, &req, udata, sizeof(req));
	if (ret)
		return ret;
	ret = virgl_res_alloc(ctx, req.bytes, &res);
	if (ret)
		return ret;
	ret = cmd_resource_create_3d(ctx->virgl->gpu,
	                             res->id,
	                             req.target,
	                             req.format,
	                             req.bind,
	                             req.width,
	                             req.height,
	                             req.depth,
	                             req.array_size,
	                             req.last_level,
	                             req.nr_samples,
	                             req.flags);
	if (ret)
		goto err;
	ret = cmd_resource_attach_backing(ctx->virgl->gpu, res->id,
	                                  res->pages, res->pages_count);
	if (ret)
		goto err;
	ret = cmd_ctx_attach_resource(ctx->virgl->gpu, ctx->id, res->id);
	if (ret)
		goto err;
	ret = file_fromcdev(res->cdev, O_RDWR, &file);
	if (ret)
	{
		TRACE("virgl: failed to create file");
		goto err;
	}
	ret = proc_allocfd(thread->proc, file, 0);
	file_free(file);
	if (ret < 0)
	{
		TRACE("virgl: failed to allocate fd");
		goto err;
	}
	return ret;

err:
	virgl_res_free(res);
	return ret;
}

static int vgl_transfer_in(struct virgl_res *res, void *udata)
{
	struct thread *thread = curcpu()->thread;
	struct vgl_transfer req;
	int ret;

	ret = vm_copyin(thread->proc->vm_space, &req, udata, sizeof(req));
	if (ret)
		return ret;
	return cmd_transfer_from_host_3d(res->ctx->virgl->gpu,
	                                 res->id,
	                                 req.x,
	                                 req.y,
	                                 req.z,
	                                 req.w,
	                                 req.h,
	                                 req.d,
	                                 req.offset,
	                                 req.level,
	                                 req.stride,
	                                 req.layer_stride);
}

static int vgl_transfer_out(struct virgl_res *res, void *udata)
{
	struct thread *thread = curcpu()->thread;
	struct vgl_transfer req;
	int ret;

	ret = vm_copyin(thread->proc->vm_space, &req, udata, sizeof(req));
	if (ret)
		return ret;
	return cmd_transfer_to_host_3d(res->ctx->virgl->gpu,
	                               res->id,
	                               req.x,
	                               req.y,
	                               req.z,
	                               req.w,
	                               req.h,
	                               req.d,
	                               req.offset,
	                               req.level,
	                               req.stride,
	                               req.layer_stride);
}

int virgl_res_open(struct file *file, struct node *node)
{
	struct virgl_res *res = getres(file);

	if (!res)
		return -EINVAL;
	refcount_inc(&res->refcount);
	return 0;
}

int virgl_res_fault(struct vm_zone *zone, off_t off, struct page **page)
{
	struct virgl_res *res = getres(zone->file);
	size_t poff;

	if (!res)
		return -EINVAL;
	if (off < 0)
		return -EINVAL;
	poff = off / PAGE_SIZE;
	if (poff >= res->pages_count)
		return -EINVAL;
	*page = res->pages[poff];
	pm_ref_page(*page);
	return 0;
}

int virgl_res_release(struct file *file)
{
	struct virgl_res *res = getres(file);

	if (!res)
	{
		TRACE("virgl: no res");
		return -EINVAL;
	}
	virgl_res_free(res);
	return 0;
}

int virgl_res_mmap(struct file *file, struct vm_zone *zone)
{
	(void)file;
	zone->op = &res_vm_op;
	return 0;
}

int virgl_res_ioctl(struct file *file, unsigned long request, uintptr_t data)
{
	struct virgl_res *res = getres(file);

	if (!res)
	{
		TRACE("virgl: no res");
		return -EINVAL;
	}
	switch (request)
	{
		case VGLTRANSFERIN:
			return vgl_transfer_in(res, (void*)data);
		case VGLTRANSFEROUT:
			return vgl_transfer_out(res, (void*)data);
		case VGLGID:
			return vm_copyout(curcpu()->thread->proc->vm_space,
			                  (void*)data, &res->id, sizeof(res->id));
	}
	return -EINVAL;
}

int virgl_ctx_release(struct file *file)
{
	struct virgl_ctx *ctx = getctx(file);

	if (!ctx)
	{
		TRACE("virgl: no ctx");
		return -EINVAL;
	}
	virgl_ctx_free(ctx);
	return 0;
}

int virgl_ctx_open(struct file *file, struct node *node)
{
	struct virgl_ctx *ctx = getctx(file);

	if (!ctx)
	{
		TRACE("virgl: no ctx");
		return -EINVAL;
	}
	refcount_inc(&ctx->refcount);
	return 0;
}

int virgl_ctx_ioctl(struct file *file, unsigned long request, uintptr_t data)
{
	struct virgl_ctx *ctx = getctx(file);

	if (!ctx)
	{
		TRACE("virgl: no ctx");
		return -EINVAL;
	}
	switch (request)
	{
		case VGLCREATERES:
			return vgl_create_res(ctx, (void*)data);
		case VGLSUBMIT:
			return vgl_submit(ctx, (void*)data);
		case VGLGID:
			return vm_copyout(curcpu()->thread->proc->vm_space,
			                  (void*)data, &ctx->id, sizeof(ctx->id));
	}
	TRACE("virgl: invalid ctx request");
	return -EINVAL;
}

int virgl_ioctl(struct file *file, unsigned long request, uintptr_t data)
{
	struct virgl *virgl = getvirgl(file);

	if (!virgl)
	{
		TRACE("virgl: no virgl");
		return -EINVAL;
	}
	switch (request)
	{
		case VGLGCAPSETINFO:
			return vgl_get_capset_info(virgl, (void*)data);
		case VGLGCAPSET:
			return vgl_get_capset(virgl, (void*)data);
		case VGLCREATECTX:
			return vgl_create_ctx(virgl, (void*)data);
		case VGLFLUSH:
			return vgl_flush(virgl, (void*)data);
	}
	TRACE("virgl: invalid request");
	return -EINVAL;
}

void virgl_free(struct virgl *virgl)
{
	struct virgl_ctx *ctx;

	if (!virgl)
		return;
	while ((ctx = TAILQ_FIRST(&virgl->ctx)))
		virgl_ctx_free(ctx);
	cdev_free(virgl->cdev);
	free(virgl->capset);
	free(virgl);
}

int virgl_init(struct virtio_gpu *gpu, struct virtio_gpu_resp_capset_info *info)
{
	struct virgl *virgl = NULL;
	int ret;

	virgl = malloc(sizeof(*virgl), M_ZERO);
	if (!virgl)
	{
		TRACE("virgl: allocation failed");
		return -ENOMEM;
	}
	TAILQ_INIT(&virgl->ctx);
	virgl->gpu = gpu;
	memcpy(&virgl->capset_info, info, sizeof(*info));
	virgl->capset = malloc(info->capset_max_size, 0);
	if (!virgl->capset)
	{
		TRACE("virgl: capset allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = cmd_get_capset(gpu, info->capset_id, info->capset_max_version,
	                     virgl->capset, info->capset_max_size);
	if (ret)
	{
		TRACE("virgl: failed to get capset");
		goto err;
	}
	ret = cdev_alloc("virgl", 0, 0, 0600, makedev(13, 0), &fop, &virgl->cdev);
	if (ret)
	{
		TRACE("virgl: failed to create cdev");
		goto err;
	}
	virgl->cdev->userdata = virgl;
	return 0;

err:
	virgl_free(virgl);
	return ret;
}
