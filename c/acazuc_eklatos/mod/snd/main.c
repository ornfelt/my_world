#include "snd.h"

#include <random.h>
#include <errno.h>
#include <file.h>
#include <kmod.h>
#include <sma.h>
#include <vfs.h>
#include <uio.h>
#include <std.h>
#include <mem.h>

static struct sma snd_sma;

void
snd_read(struct snd *snd, void *data, size_t size)
{
	struct uio uio;
	struct iovec iov;
	ssize_t rd;

	uio_fromkbuf(&uio, &iov, data, size, 0);
	rd = pipebuf_read(&snd->pipebuf, &uio, 0, NULL);
	if (rd <= 0) /* XXX ? */
		memset(data, 0, size);
	else
		memset(&((uint8_t*)data)[rd], 0, size - rd);
}

void
snd_fill_buf(struct snd *snd, struct dma_buf *buf)
{
	snd_read(snd, buf->data, PAGE_SIZE);
}

static int
snd_cdev_open(struct file *file, struct node *node)
{
	struct cdev *cdev = node->cdev;

	if (!cdev)
		return -EINVAL;
	file->userdata = cdev->userdata;
	return 0;
}

static ssize_t
snd_cdev_write(struct file *file, struct uio *uio)
{
	struct snd *snd = file->userdata;

	return pipebuf_write(&snd->pipebuf, uio, uio->count, NULL);
}

static const struct file_op
fop =
{
	.open = snd_cdev_open,
	.write = snd_cdev_write,
};

int
snd_alloc(struct snd **sndp)
{
	struct snd *snd;
	int ret;

	snd = sma_alloc(&snd_sma, M_ZERO);
	if (!snd)
		return -ENOMEM;
	refcount_init(&snd->refcount, 1);
	snd->nbufs = SND_NBUFS;
	mutex_init(&snd->mutex, 0);
	waitq_init(&snd->waitq);
	ret = pipebuf_init(&snd->pipebuf, PAGE_SIZE * 2, &snd->mutex,
	                   &snd->waitq, &snd->waitq);
	if (ret)
	{
		mutex_destroy(&snd->mutex);
		waitq_destroy(&snd->waitq);
		sma_free(&snd_sma, snd);
		return ret;
	}
	snd->pipebuf.nreaders++;
	snd->pipebuf.nwriters++;
	for (size_t i = 0; ; ++i)
	{
		if (i == 4096)
		{
			snd_free(snd);
			return -ENOMEM;
		}
		snprintf(snd->name, sizeof(snd->name), "snd%zu", i);
		ret = cdev_alloc(snd->name, 0, 0, 0600, makedev(5, i), &fop, &snd->cdev);
		if (!ret)
			break;
		if (ret != -EEXIST)
		{
			snd_free(snd);
			return ret;
		}
	}
	snd->cdev->userdata = snd;
	for (size_t i = 0; i < snd->nbufs; ++i)
	{
		ret = dma_buf_alloc(PAGE_SIZE, 0, &snd->bufs[i]);
		if (ret)
		{
			snd_free(snd);
			return ret;
		}
#if 1
		memset(snd->bufs[i]->data, 0, PAGE_SIZE);
#endif
#if 0
		random_get(snd->bufs[i]->data, PAGE_SIZE);
#endif
#if 0
		int16_t *dst = (int16_t*)snd->bufs[i]->data;
		for (size_t n = 0; n < PAGE_SIZE / 2; ++n)
		{
			if (n & 1)
			{
				if (n % 96 < 48)
					dst[n] = INT16_MIN;
				else
					dst[n] = INT16_MAX;
			}
			else
			{
				if (n % 192 < 96)
					dst[n] = INT16_MIN;
				else
					dst[n] = INT16_MAX;
			}
		}
#endif
	}
	*sndp = snd;
	return 0;
}

void
snd_free(struct snd *snd)
{
	if (!snd)
		return;
	if (refcount_dec(&snd->refcount))
		return;
	for (size_t i = 0; i < snd->nbufs; ++i)
		dma_buf_free(snd->bufs[i]);
	pipebuf_destroy(&snd->pipebuf);
	mutex_destroy(&snd->mutex);
	waitq_destroy(&snd->waitq);
	sma_free(&snd_sma, snd);
}

int
init(void)
{
	sma_init(&snd_sma, sizeof(struct snd), NULL, NULL, "snd");
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
	.name = "snd",
	.init = init,
	.fini = fini,
};
