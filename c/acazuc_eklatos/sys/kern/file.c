#include <net/local.h>

#include <errno.h>
#include <file.h>
#include <pipe.h>
#include <sock.h>
#include <std.h>
#include <vfs.h>
#include <uio.h>
#include <sma.h>

static struct sma file_sma;

void file_init(void)
{
	sma_init(&file_sma, sizeof(struct file), NULL, NULL, "file");
}

int file_fromnode(struct node *node, int flags, struct file **file)
{
	const struct file_op *fop;
	if (node)
	{
		if (S_ISSOCK(node->attr.mode))
			return -ENXIO;
		if (S_ISCHR(node->attr.mode))
		{
			if (!node->cdev)
			{
				node->cdev = cdev_find(node->rdev);
				if (!node->cdev)
					return -ENXIO;
			}
			fop = node->cdev->fop;
		}
		else if (S_ISBLK(node->attr.mode))
		{
			if (!node->bdev)
			{
				node->bdev = bdev_find(node->rdev);
				if (!node->bdev)
					return -ENXIO;
			}
			fop = node->bdev->fop;
		}
		else if (S_ISFIFO(node->attr.mode))
		{
			if (!node->pipe)
			{
				int ret = pipe_alloc(&node->pipe, node);
				if (ret)
					return ret;
			}
			fop = &g_pipe_fop;
		}
		else
		{
			fop = node->fop;
		}
	}
	else
	{
		fop = NULL;
	}
	*file = sma_alloc(&file_sma, M_ZERO);
	if (!*file)
		return -ENOMEM;
	(*file)->op = fop;
	(*file)->node = node;
	(*file)->flags = flags & ~O_CLOEXEC;
	refcount_init(&(*file)->refcount, 1);
	if (node)
		node_ref(node);
	return 0;
}

int file_fromsock(struct sock *sock, int flags, struct file **file)
{
	*file = sma_alloc(&file_sma, M_ZERO);
	if (!*file)
		return -ENOMEM;
	(*file)->op = &g_sock_fop;
	(*file)->sock = sock;
	(*file)->flags = flags | O_RDWR;
	refcount_init(&(*file)->refcount, 1);
	if (sock)
		sock_ref(sock);
	return 0;
}

int file_frombdev(struct bdev *bdev, int flags, struct file **file)
{
	*file = sma_alloc(&file_sma, M_ZERO);
	if (!*file)
		return -ENOMEM;
	(*file)->op = bdev->fop;
	(*file)->bdev = bdev;
	(*file)->flags = flags;
	refcount_init(&(*file)->refcount, 1);
	return 0;
}

int file_fromcdev(struct cdev *cdev, int flags, struct file **file)
{
	*file = sma_alloc(&file_sma, M_ZERO);
	if (!*file)
		return -ENOMEM;
	(*file)->op = cdev->fop;
	(*file)->cdev = cdev;
	(*file)->flags = flags;
	refcount_init(&(*file)->refcount, 1);
	return 0;
}

void file_ref(struct file *file)
{
	refcount_inc(&file->refcount);
}

void file_free(struct file *file)
{
	if (refcount_dec(&file->refcount))
		return;
	if (file->op && file->op->release)
		file->op->release(file);
	if (file->node)
		node_free(file->node);
	if (file->sock)
		sock_free(file->sock);
	sma_free(&file_sma, file);
}

int file_release(struct file *file)
{
	if (!file->op || !file->op->release)
		return 0;
	return file->op->release(file);
}

int file_open(struct file *file, struct node *node)
{
	if (!file->op || !file->op->open)
		return 0;
	return file->op->open(file, node);
}

ssize_t file_write(struct file *file, struct uio *uio)
{
	if (!file->op || !file->op->write)
		return -ENOSYS;
	return file->op->write(file, uio);
}

ssize_t file_read(struct file *file, struct uio *uio)
{
	if (!file->op || !file->op->read)
		return -ENOSYS;
	return file->op->read(file, uio);
}

ssize_t file_readseq(struct file *file, void *data, size_t count, off_t off)
{
	struct iovec iov;
	struct uio uio;
	uio_fromkbuf(&uio, &iov, data, count, off);
	return file_read(file, &uio);
}

int file_ioctl(struct file *file, unsigned long request, uintptr_t data)
{
	if (!file->op || !file->op->ioctl)
		return -ENOSYS;
	return file->op->ioctl(file, request, data);
}

int file_mmap(struct file *file, struct vm_zone *zone)
{
	if (!file->op || !file->op->mmap)
		return -ENOSYS;
	return file->op->mmap(file, zone);
}

int file_seek(struct file *file, off_t off, int whence)
{
	if (!file->op || !file->op->seek)
		return -ENOSYS;
	return file->op->seek(file, off, whence);
}

int file_poll(struct file *file, struct poll_entry *entry)
{
	if (!file->op || !file->op->poll)
		return -ENOSYS;
	return file->op->poll(file, entry);
}
