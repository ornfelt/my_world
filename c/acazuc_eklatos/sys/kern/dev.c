#if defined(__i386__) || defined(__x86_64__)
#include "arch/x86/asm.h"
#endif

#include <random.h>
#include <errno.h>
#include <file.h>
#include <std.h>
#include <uio.h>
#include <sma.h>
#include <vfs.h>

static struct spinlock cdev_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static TAILQ_HEAD(, cdev) cdev_list = TAILQ_HEAD_INITIALIZER(cdev_list);
static struct spinlock bdev_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static TAILQ_HEAD(, bdev) bdev_list = TAILQ_HEAD_INITIALIZER(bdev_list);

static struct sma cdev_sma;
static struct sma bdev_sma;

void bcdev_init(void)
{
	sma_init(&cdev_sma, sizeof(struct cdev), NULL, NULL, "cdev");
	sma_init(&bdev_sma, sizeof(struct bdev), NULL, NULL, "bdev");
}

int cdev_alloc(const char *name, uid_t uid, gid_t gid, mode_t mode, dev_t rdev,
               const struct file_op *fop, struct cdev **cdev)
{
	*cdev = sma_alloc(&cdev_sma, 0);
	if (!*cdev)
		return -ENOMEM;
	(*cdev)->fop = fop;
	(*cdev)->rdev = rdev;
	spinlock_lock(&cdev_lock);
	TAILQ_INSERT_TAIL(&cdev_list, *cdev, chain);
	spinlock_unlock(&cdev_lock);
	return devfs_mkcdev(name, uid, gid, mode, rdev, *cdev);
}

int bdev_alloc(const char *name, uid_t uid, gid_t gid, mode_t mode, dev_t rdev,
               const struct file_op *fop, struct bdev **bdev)
{
	*bdev = sma_alloc(&bdev_sma, 0);
	if (!*bdev)
		return -ENOMEM;
	(*bdev)->fop = fop;
	(*bdev)->rdev = rdev;
	spinlock_lock(&bdev_lock);
	TAILQ_INSERT_TAIL(&bdev_list, *bdev, chain);
	spinlock_unlock(&bdev_lock);
	return devfs_mkbdev(name, uid, gid, mode, rdev, *bdev);
}

struct cdev *cdev_find(dev_t dev)
{
	spinlock_lock(&cdev_lock);
	struct cdev *cdev;
	TAILQ_FOREACH(cdev, &cdev_list, chain)
	{
		if (cdev->rdev == dev)
			break;
	}
	spinlock_unlock(&cdev_lock);
	return cdev;
}

struct bdev *bdev_find(dev_t dev)
{
	spinlock_lock(&bdev_lock);
	struct bdev *bdev;
	TAILQ_FOREACH(bdev, &bdev_list, chain)
	{
		if (bdev->rdev == dev)
			break;
	}
	spinlock_unlock(&bdev_lock);
	return bdev;
}

void cdev_free(struct cdev *cdev)
{
	spinlock_lock(&cdev_lock);
	TAILQ_REMOVE(&cdev_list, cdev, chain);
	spinlock_unlock(&cdev_lock);
	/* XXX remove from devfs ? it should somehow be refcounted and marked as deleted*/
	sma_free(&cdev_sma, cdev);
}

void bdev_free(struct bdev *bdev)
{
	spinlock_lock(&bdev_lock);
	TAILQ_REMOVE(&bdev_list, bdev, chain);
	spinlock_unlock(&bdev_lock);
	/* XXX remove from devfs ? it should somehow be refcounted and marked as deleted*/
	sma_free(&bdev_sma, bdev);
}

static const struct file_op g_mem_fop =
{
	/* XXX */
};

static const struct file_op g_kmem_fop =
{
	/* XXX */
};

static ssize_t null_write(struct file *file, struct uio *uio)
{
	(void)file;
	return uio->count;
}

static ssize_t null_read(struct file *file, struct uio *uio)
{
	(void)file;
	(void)uio;
	return 0;
}

static const struct file_op g_null_fop =
{
	.write = null_write,
	.read = null_read,
};

#if defined(__i386__) || defined(__x86_64__)
static ssize_t port_write(struct file *file, struct uio *uio)
{
	(void)file;
	uint8_t buf[4096];
	size_t wr = 0;
	while (uio->count)
	{
		off_t port = uio->off;
		if (port < 0 || port > 0xFFFF)
			break;
		ssize_t ret = uio_copyout(buf, uio, sizeof(buf));
		if (ret < 0)
			return ret;
		for (ssize_t i = 0; i < ret; ++i)
		{
			if (port < 0 || port > 0xFFFF)
				break;
			outb(port++, buf[i]);
		}
		wr += ret;
	}
	return wr;
}

static ssize_t port_read(struct file *file, struct uio *uio)
{
	(void)file;
	uint8_t buf[4096];
	size_t rd = 0;
	size_t n = sizeof(buf);
	if (n > uio->count)
		n = uio->count;
	while (uio->count)
	{
		off_t port = uio->off;
		if (port < 0 || port > 0xffff)
			break;
		size_t i;
		for (i = 0; i < n; ++i)
		{
			if (port < 0 || port > 0xffff)
				break;
			buf[i] = inb(port++);
		}
		ssize_t ret = uio_copyin(uio, buf, i);
		if (ret < 0)
			return ret;
		rd += ret;
	}
	return rd;
}

static const struct file_op g_port_fop =
{
	.write = port_write,
	.read = port_read,
};
#endif

static ssize_t zero_write(struct file *file, struct uio *uio)
{
	(void)file;
	return uio->count;
}

static ssize_t zero_read(struct file *file, struct uio *uio)
{
	(void)file;
	return uio_copyz(uio, uio->count);
}

static const struct file_op g_zero_fop =
{
	.write = zero_write,
	.read = zero_read,
};

static ssize_t random_write(struct file *file, struct uio *uio)
{
	/* XXX use as random source ?*/
	(void)file;
	return uio->count;
}

static ssize_t random_read(struct file *file, struct uio *uio)
{
	(void)file;
	uint8_t buf[4096];
	size_t rd = 0;
	size_t n = sizeof(buf);
	if (n > uio->count)
		n = uio->count;
	while (uio->count)
	{
		ssize_t ret = random_get(buf, n);
		if (ret < 0)
			return ret;
		ret = uio_copyin(uio, buf, ret);
		if (ret < 0)
			return ret;
		rd += ret;
	}
	return rd;
}

static const struct file_op g_random_fop =
{
	.write = random_write,
	.read = random_read,
};

static ssize_t urandom_write(struct file *file, struct uio *uio)
{
	/* XXX use as random source ?*/
	(void)file;
	return uio->count;
}

static ssize_t urandom_read(struct file *file, struct uio *uio)
{
	(void)file;
	uint8_t buf[4096];
	size_t rd = 0;
	size_t n = sizeof(buf);
	if (n > uio->count)
		n = uio->count;
	while (uio->count)
	{
		ssize_t ret = random_get(buf, n);
		if (ret < 0)
			return ret;
		ret = uio_copyin(uio, buf, ret);
		if (ret < 0)
			return ret;
		rd += ret;
	}
	return rd;
}

static const struct file_op g_urandom_fop =
{
	.write = urandom_write,
	.read = urandom_read,
};

static ssize_t kmsg_write(struct file *file, struct uio *uio)
{
	(void)file;
	char buf[4096];
	size_t wr = 0;
	while (uio->count)
	{
		ssize_t ret = uio_copyout(buf, uio, sizeof(buf));
		if (ret < 0)
			return ret;
		printf("%.*s", (int)ret, buf);
		wr += ret;
	}
	return wr;
}

static const struct file_op g_kmsg_fop =
{
	.write = kmsg_write,
};

static struct cdev *dev_mem;
static struct cdev *dev_kmem;
static struct cdev *dev_null;
#if defined(__i386__) || defined(__x86_64__)
static struct cdev *dev_port;
#endif
static struct cdev *dev_zero;
static struct cdev *dev_random;
static struct cdev *dev_urandom;
static struct cdev *dev_kmsg;

int cdev_init(void)
{
#define MKCDEV(name, uid, gid, mode, rdev, fop, cdev) \
do \
{ \
	int ret = cdev_alloc(name, uid, gid, mode, rdev, fop, cdev); \
	if (ret) \
		printf("failed to create " name ": %s\n", strerror(ret)); \
} while (0)
	MKCDEV("mem", 0, 0, 0400, makedev(1, 1), &g_mem_fop, &dev_mem);
	MKCDEV("kmem", 0, 0, 0400, makedev(1, 2), &g_kmem_fop, &dev_kmem);
	MKCDEV("null", 0, 0, 0666, makedev(1, 3), &g_null_fop, &dev_null);
#if defined(__i386__) || defined(__x86_64__)
	MKCDEV("port", 0, 0, 0600, makedev(1, 4), &g_port_fop, &dev_port);
#endif
	MKCDEV("zero", 0, 0, 0666, makedev(1, 5), &g_zero_fop, &dev_zero);
	MKCDEV("random", 0, 0, 0666, makedev(1, 8), &g_random_fop, &dev_random);
	MKCDEV("urandom", 0, 0, 0666, makedev(1, 9), &g_urandom_fop, &dev_urandom);
	MKCDEV("kmsg", 0, 0, 0600, makedev(1, 12), &g_kmsg_fop, &dev_kmsg);
#undef MKCDEV
	return 0;
}
