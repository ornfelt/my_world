#define ENABLE_TRACE

#include "virtio.h"
#include "pci.h"

#include <errno.h>
#include <time.h>
#include <std.h>
#include <sg.h>

#define VIRTQ_DESC_F_NEXT     1
#define VIRTQ_DESC_F_WRITE    2
#define VIRTQ_DESC_F_INDIRECT 3

#define VIRTQ_AVAIL_F_NO_INTERRUPT 1

#define VIRTQ_USED_F_NO_NOTIFY 1

struct virtq_desc
{
	uint64_t addr;
	uint32_t size;
	uint16_t flags;
	uint16_t next;
};

struct virtq_avail
{
	uint16_t flags;
	uint16_t index;
	uint16_t ring[];
};

struct virtq_used_elem
{
	uint32_t id;
	uint32_t len;
};

struct virtq_used
{
	uint16_t flags;
	uint16_t index;
	struct virtq_used_elem ring[];
};

static inline void
print(struct uio *uio, struct pci_map *cmn_cfg)
{
	uprintf(uio, "queue_size: 0x%" PRIx16 "\n",
	        pci_r16(cmn_cfg, VIRTIO_C_QUEUE_SIZE));
	uprintf(uio, "queue_msix_vector: 0x%" PRIx16 "\n",
	        pci_r16(cmn_cfg, VIRTIO_C_QUEUE_MSIX_VECTOR));
	uprintf(uio, "queue_enable: 0x%" PRIx16 "\n",
	        pci_r16(cmn_cfg, VIRTIO_C_QUEUE_ENABLE));
	uprintf(uio, "queue_notify_off: 0x%" PRIx16 "\n",
	        pci_r16(cmn_cfg, VIRTIO_C_QUEUE_NOTIFY_OFF));
	uprintf(uio, "queue_desc: 0x%" PRIx64 "\n",
	        pci_r64(cmn_cfg, VIRTIO_C_QUEUE_DESC));
	uprintf(uio, "queue_driver: 0x%" PRIx64 "\n",
	        pci_r64(cmn_cfg, VIRTIO_C_QUEUE_DRIVER));
	uprintf(uio, "queue_device: 0x%" PRIx64 "\n",
	        pci_r64(cmn_cfg, VIRTIO_C_QUEUE_DEVICE));
}

int
virtq_poll(struct virtq *queue, uint16_t *id, uint32_t *len)
{
	struct virtq_used *used = queue->used->data;
	struct virtq_used_elem *elem;
	uint16_t index;

	index = __atomic_load_n(&used->index, __ATOMIC_ACQUIRE) % queue->size;
	if (queue->used_tail == index)
		return -EAGAIN;
	elem = &used->ring[queue->used_tail];
	*id = elem->id;
	*len = elem->len;
	queue->used_tail = (queue->used_tail + 1) % queue->size;
	return 0;
}

void
virtq_on_irq(struct virtq *queue)
{
	struct virtq_used *used = queue->used->data;
	struct virtq_used_elem *elem;
	uint16_t index;

	if (!queue->on_msg)
		return;
	index = __atomic_load_n(&used->index, __ATOMIC_ACQUIRE) % queue->size;
	while (queue->used_tail != index)
	{
		elem = &used->ring[queue->used_tail];
		queue->on_msg(queue, elem->id, elem->len);
		queue->used_tail = (queue->used_tail + 1) % queue->size;
	}
}

static void
int_handler(void *userptr)
{
	struct virtq *queue = userptr;
	virtq_on_irq(queue);
}

int
virtq_setup_irq(struct virtq *queue)
{
	uint16_t vector;
	int ret;

	if (queue->dev->irq_handle.type == IRQ_MSIX)
	{
		ret = pci_register_irq(queue->dev->device, int_handler,
		                       queue, &queue->irq_handle);
		if (ret)
			return ret;
		vector = queue->irq_handle.msix.vector;
	}
	else
	{
		vector = 0xFFFF;
	}
	pci_w16(queue->dev->common_cfg, VIRTIO_C_QUEUE_SELECT, queue->id);
	pci_w16(queue->dev->common_cfg, VIRTIO_C_QUEUE_MSIX_VECTOR, vector);
	return 0;
}

void
virtq_notify(struct virtq *queue)
{
	__atomic_thread_fence(__ATOMIC_RELEASE);
	pci_w32(queue->dev->notify_cfg,
	        queue->dev->notify_multiplier * queue->id,
	        queue->id);
}

int
virtq_init(struct virtq *queue, struct virtio_dev *dev, uint16_t id)
{
	int ret;

	queue->id = id;
	queue->dev = dev;
	pci_w16(dev->common_cfg, VIRTIO_C_QUEUE_SELECT, id);
#if 0
	print(NULL, dev->common_cfg);
#endif
	queue->size = pci_r16(dev->common_cfg, VIRTIO_C_QUEUE_SIZE);
	if (!queue->size)
	{
		TRACE("virtq: empty queue");
		return -EINVAL;
	}
	if (queue->size > 0x100)
	{
		pci_w16(dev->common_cfg, VIRTIO_C_QUEUE_SIZE, 0x100);
		queue->size = 0x100;
	}
	ret = dma_buf_alloc(PAGE_SIZE, 0, &queue->desc);
	if (ret)
	{
		TRACE("virtq: desc allocation failed");
		return ret;
	}
	memset(queue->desc->data, 0, PAGE_SIZE);
	pci_w64(dev->common_cfg,
	        VIRTIO_C_QUEUE_DESC,
	        pm_page_addr(queue->desc->pages));
	ret = dma_buf_alloc(PAGE_SIZE, 0, &queue->avail);
	if (ret)
	{
		TRACE("virtq: avail allocation failed");
		return ret;
	}
	memset(queue->avail->data, 0, PAGE_SIZE);
	pci_w64(dev->common_cfg,
	        VIRTIO_C_QUEUE_DRIVER,
	        pm_page_addr(queue->avail->pages));
	ret = dma_buf_alloc(PAGE_SIZE, 0, &queue->used);
	if (ret)
	{
		TRACE("virtq: used allocation failed");
		return ret;
	}
	memset(queue->used->data, 0, PAGE_SIZE);
	pci_w64(dev->common_cfg,
	        VIRTIO_C_QUEUE_DEVICE,
	        pm_page_addr(queue->used->pages));
	pci_w16(dev->common_cfg, VIRTIO_C_QUEUE_ENABLE, 1);
	((struct virtq_avail*)queue->avail->data)->flags = 0;
	((struct virtq_used*)queue->used->data)->flags = VIRTQ_USED_F_NO_NOTIFY;
#if 0
	print(NULL, dev->common_cfg);
#endif
	return 0;
}

void
virtq_destroy(struct virtq *queue)
{
	if (!queue->size)
		return;
	dma_buf_free(queue->desc);
	dma_buf_free(queue->avail);
	dma_buf_free(queue->used);
	if (queue->dev->irq_handle.type != IRQ_MSIX)
		pci_unregister_irq(queue->dev->device, &queue->irq_handle);
}

static void
add_sg_list(struct virtq *queue,
            const struct sg_head *sg_head,
            int write,
            int last)
{
	struct virtq_desc *desc;
	struct sg *sg;
	uint16_t flags;
	uint16_t next;

	flags = write ? VIRTQ_DESC_F_WRITE : 0;
	TAILQ_FOREACH(sg, &sg_head->sg, chain)
	{
		desc = &((struct virtq_desc*)queue->desc->data)[queue->desc_head];
		desc->addr = pm_page_addr(sg->page) + sg->offset;
		desc->size = sg->size;
		desc->flags = flags;
		next = (queue->desc_head + 1) % queue->size;
		if (!last || TAILQ_NEXT(sg, chain))
		{
			desc->next = next;
			desc->flags |= VIRTQ_DESC_F_NEXT;
		}
		else
		{
			desc->next = 0;
		}
		queue->desc_head = next;
	}
}

int
virtq_send(struct virtq *queue,
           const struct sg_head *sg_read_head,
           const struct sg_head *sg_write_head)
{
	struct virtq_avail *avail = queue->avail->data;
	size_t total = 0;
	uint16_t base = queue->desc_head;

	if (sg_read_head)
		total += sg_read_head->count;
	if (sg_write_head)
		total += sg_write_head->count;
	if (!total)
		return 0;
	if (sg_read_head)
		add_sg_list(queue, sg_read_head, 0, !sg_write_head || TAILQ_EMPTY(&sg_write_head->sg));
	if (sg_write_head)
		add_sg_list(queue, sg_write_head, 1, 1);
	avail->ring[avail->index % queue->size] = base;
	__atomic_add_fetch(&avail->index, 1, __ATOMIC_RELEASE);
	return 0;
}
