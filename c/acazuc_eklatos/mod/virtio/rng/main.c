#define ENABLE_TRACE

#include "virtio.h"
#include "pci.h"

#include <random.h>
#include <errno.h>
#include <kmod.h>
#include <std.h>
#include <sg.h>

struct virtio_rng
{
	struct virtio_dev dev;
	struct dma_buf *buf;
	struct waitq waitq;
	struct spinlock waitq_lock;
	uint32_t collect_len;
};

static void
on_msg(struct virtq *queue, uint16_t id, uint32_t len)
{
	struct virtio_rng *rng = (struct virtio_rng*)queue->dev;

	(void)id;
	spinlock_lock(&rng->waitq_lock);
	rng->collect_len = len;
	waitq_signal(&rng->waitq, 0);
	spinlock_unlock(&rng->waitq_lock);
}

static ssize_t
random_collect(void *buf, size_t size, void *userdata)
{
	struct virtio_rng *rng = userdata;
	struct sg_head sg;
	int ret;

	sg_init(&sg);
	if (size > PAGE_SIZE)
		size = PAGE_SIZE;
	ret = sg_add_dma_buf(&sg, rng->buf, size, 0);
	if (ret)
		goto end;
	ret = virtq_send(&rng->dev.queues[0], NULL, &sg);
	if (ret < 0)
		goto end;
	virtq_notify(&rng->dev.queues[0]);
	/* XXX should not sleep */
	spinlock_lock(&rng->waitq_lock);
	ret = waitq_wait_head(&rng->waitq, &rng->waitq_lock, NULL);
	spinlock_unlock(&rng->waitq_lock);
	if (ret)
		goto end;
	memcpy(buf, rng->buf->data, size);
	ret = size;

end:
	sg_free(&sg);
	return ret;
}

static void
virtio_rng_delete(struct virtio_rng *rng)
{
	if (!rng)
		return;
	dma_buf_free(rng->buf);
	virtio_dev_destroy(&rng->dev);
	waitq_destroy(&rng->waitq);
	spinlock_destroy(&rng->waitq_lock);
	free(rng);
}

int
init_pci(struct pci_device *device, void *userdata)
{
	struct virtio_rng *rng;
	uint8_t features[1];
	int ret;

	(void)userdata;
	rng = malloc(sizeof(*rng), M_ZERO);
	if (!rng)
	{
		TRACE("virtio_rng: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	waitq_init(&rng->waitq);
	spinlock_init(&rng->waitq_lock);
	ret = virtio_dev_init(&rng->dev, device, features, 0);
	if (ret)
	{
		virtio_rng_delete(rng);
		return ret;
	}
	if (rng->dev.queues_nb < 1)
	{
		TRACE("virtio_rng: no queues");
		ret = -EINVAL;
		goto err;
	}
	ret = dma_buf_alloc(PAGE_SIZE, 0, &rng->buf);
	if (ret)
	{
		TRACE("virtio_rng: buffer allocation failed");
		goto err;
	}
	rng->dev.queues[0].on_msg = on_msg;
	ret = virtq_setup_irq(&rng->dev.queues[0]);
	if (ret)
	{
		TRACE("virtio_rng: failed to setup irq");
		goto err;
	}
	virtio_dev_init_end(&rng->dev);
	random_register(random_collect, rng);
	return 0;

err:
	virtio_rng_delete(rng);
	return ret;
}

static int
init(void)
{
	pci_probe(0x1AF4, 0x1005, init_pci, NULL);
	return 0;
}

static void
fini(void)
{
}

struct kmod_info
kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "virtio/rng",
	.init = init,
	.fini = fini,
};
