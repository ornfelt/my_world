#define ENABLE_TRACE

#include "virtio.h"
#include "pci.h"

#include <errno.h>
#include <disk.h>
#include <kmod.h>
#include <uio.h>
#include <std.h>
#include <sg.h>

#define VIRTIO_BLK_F_BARRIER      0
#define VIRTIO_BLK_F_SIZE_MAX     1
#define VIRTIO_BLK_F_SEG_MAX      2
#define VIRTIO_BLK_F_GEOMETRY     4
#define VIRTIO_BLK_F_RO           5
#define VIRTIO_BLK_F_BLK_SIZE     6
#define VIRTIO_BLK_F_SCSI         7
#define VIRTIO_BLK_F_FLUSH        9
#define VIRTIO_BLK_F_TOPOLOGY     10
#define VIRTIO_BLK_F_CONFIG_WCE   11
#define VIRTIO_BLK_F_MQ           12
#define VIRTIO_BLK_F_DISCARD      13
#define VIRTIO_BLK_F_WRITE_ZEROES 14
#define VIRTIO_BLK_F_LIFETIME     15
#define VIRTIO_BLK_F_SECURE_ERASE 16

#define VIRTIO_BLK_F_DISCARD_UNMAP 1

#define VIRTIO_BLK_T_IN           0
#define VIRTIO_BLK_T_OUT          1
#define VIRTIO_BLK_T_FLUSH        4
#define VIRTIO_BLK_T_GET_ID       8
#define VIRTIO_BLK_T_GET_LIFETIME 10
#define VIRTIO_BLK_T_DISCARD      11
#define VIRTIO_BLK_T_WRITE_ZEROES 13
#define VIRTIO_BLK_T_SECURE_ERASE 14

#define VIRTIO_BLK_S_OK     0
#define VIRTIO_BLK_S_IOERR  1
#define VIRTIO_BLK_S_UNSUPP 2

#define VIRTIO_BLK_C_CAPACITY             0x00
#define VIRTIO_BLK_C_SIZE_MAX             0x08
#define VIRTIO_BLK_C_SEG_MAX              0x0C
#define VIRTIO_BLK_C_CYLINDERS            0x10
#define VIRTIO_BLK_C_HEADS                0x12
#define VIRTIO_BLK_C_SECTORS              0x13
#define VIRTIO_BLK_C_BLK_SIZE             0x14
#define VIRTIO_BLK_C_PHYSICAL_BLOCK_EXP   0x18
#define VIRTIO_BLK_C_ALIGN_OFF            0x19
#define VIRTIO_BLK_C_MIN_IO_SIZE          0x1A
#define VIRTIO_BLK_C_MAX_IO_SIZE          0x1C
#define VIRTIO_BLK_C_WRITEBACK            0x20
#define VIRTIO_BLK_C_MAX_DISCARD_SECTORS  0x24
#define VIRTIO_BLK_C_MAX_DISCARD_SEG      0x28
#define VIRTIO_BLK_C_DISCARD_SECTOR_ALIGN 0x2C
#define VIRTIO_BLK_C_MAX_ZEROES_SECTORS   0x30
#define VIRTIO_BLK_C_MAX_ZEROES_SEG       0x34
#define VIRTIO_BLK_C_ZEROES_MAY_UNMAP     0x38

#define BLOCK_SIZE 512

struct virtio_blk_req
{
	uint32_t type;
	uint32_t reserved;
	uint64_t sector;
};

struct virtio_blk_discard_write_zeroes
{
	uint64_t sectors;
	uint32_t num_sectors;
	uint32_t flags;
};

struct virtio_blk
{
	struct virtio_dev dev;
	struct pci_map *blk_cfg;
	struct disk *disk;
	struct mutex mutex;
	struct dma_buf *buf;
	struct waitq waitq;
	struct spinlock waitq_lock;
};

static ssize_t dread(struct disk *disk, struct uio *uio);
static ssize_t dwrite(struct disk *disk, struct uio *uio);

static const struct disk_op g_op =
{
	.read = dread,
	.write = dwrite,
};

static void on_msg(struct virtq *queue, uint16_t id, uint32_t len)
{
	struct virtio_blk *blk = (struct virtio_blk*)queue->dev;

	(void)id; /* XXX used for parallel requests */
	(void)len;
	spinlock_lock(&blk->waitq_lock);
	waitq_signal(&blk->waitq, 0);
	spinlock_unlock(&blk->waitq_lock);
}

static int wait_buf(struct virtio_blk *blk)
{
	int ret;

	spinlock_lock(&blk->waitq_lock);
	ret = waitq_wait_head(&blk->waitq, &blk->waitq_lock, NULL);
	spinlock_unlock(&blk->waitq_lock);
	return ret;
}

static ssize_t dread(struct disk *disk, struct uio *uio)
{
	struct virtio_blk *blk = disk->userdata;
	struct virtio_blk_req *req;
	struct sg_head sg;
	size_t numsect = uio->count / BLOCK_SIZE;
	ssize_t ret;
	size_t rd = 0;

	sg_init(&sg);
	mutex_lock(&blk->mutex);
	for (size_t i = 0; i < numsect; ++i)
	{
		req = blk->buf->data;
		req->type = VIRTIO_BLK_T_IN;
		req->reserved = 0;
		req->sector = uio->off / BLOCK_SIZE;
		sg_reset(&sg);
		ret = sg_add_dma_buf(&sg, blk->buf, 16, 0);
		if (ret)
			goto end;
		ret = sg_add_uio(&sg, uio, BLOCK_SIZE);
		if (ret)
			goto end;
		ret = sg_add_dma_buf(&sg, blk->buf, 1, 16);
		if (ret)
			goto end;
		ret = virtq_send(&blk->dev.queues[0], &sg, 1, sg.count - 1);
		if (ret < 0)
			goto end;
		virtq_notify(&blk->dev.queues[0]);
		ret = wait_buf(blk);
		if (ret)
			goto end;
		if (((uint8_t*)blk->buf->data)[16] != VIRTIO_BLK_S_OK)
		{
			TRACE("virtio_blk: read request failure");
			ret = -ENXIO;
			goto end;
		}
		uio_advance(uio, BLOCK_SIZE);
		rd += BLOCK_SIZE;
	}
	ret = rd;

end:
	mutex_unlock(&blk->mutex);
	sg_free(&sg);
	return ret;
}

static ssize_t dwrite(struct disk *disk, struct uio *uio)
{
	struct virtio_blk *blk = disk->userdata;
	struct virtio_blk_req *req;
	struct sg_head sg;
	size_t numsect = uio->count / BLOCK_SIZE;
	ssize_t ret;
	size_t wr = 0;

	sg_init(&sg);
	mutex_lock(&blk->mutex);
	for (size_t i = 0; i < numsect; ++i)
	{
		req = (struct virtio_blk_req*)blk->buf->data;
		req->type = VIRTIO_BLK_T_OUT;
		req->reserved = 0;
		req->sector = uio->off / BLOCK_SIZE;
		sg_reset(&sg);
		ret = sg_add_dma_buf(&sg, blk->buf, 16, 0);
		if (ret)
			goto end;
		ret = sg_add_uio(&sg, uio, BLOCK_SIZE);
		if (ret)
			goto end;
		ret = sg_add_dma_buf(&sg, blk->buf, 1, 16);
		if (ret)
			goto end;
		ret = virtq_send(&blk->dev.queues[0], &sg, sg.count - 1, 1);
		if (ret < 0)
			goto end;
		virtq_notify(&blk->dev.queues[0]);
		ret = wait_buf(blk);
		if (ret)
			goto end;
		if (((uint8_t*)blk->buf->data)[16] != VIRTIO_BLK_S_OK)
		{
			TRACE("virtio_blk: write request failure");
			ret = -ENXIO;
			goto end;
		}
		uio_advance(uio, BLOCK_SIZE);
		wr += BLOCK_SIZE;
	}
	ret = wr;

end:
	mutex_unlock(&blk->mutex);
	sg_free(&sg);
	return ret;
}

static inline void print_blk_cfg(struct uio *uio, struct pci_map *blk_cfg)
{
	uprintf(uio, "capacity: 0x%" PRIx64 "\n",
	        pci_r64(blk_cfg, VIRTIO_BLK_C_CAPACITY));
	uprintf(uio, "size_max: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_SIZE_MAX));
	uprintf(uio, "seg_max: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_SEG_MAX));
	uprintf(uio, "cylinders: 0x%" PRIx16 "\n",
	        pci_r16(blk_cfg, VIRTIO_BLK_C_CYLINDERS));
	uprintf(uio, "heads: 0x%" PRIx16 "\n",
	        pci_r16(blk_cfg, VIRTIO_BLK_C_HEADS));
	uprintf(uio, "sectors: 0x%" PRIx16 "\n",
	        pci_r16(blk_cfg, VIRTIO_BLK_C_SECTORS));
	uprintf(uio, "blk_size: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_BLK_SIZE));
	uprintf(uio, "physical_block_exp: 0x%" PRIx8 "\n",
	        pci_r8(blk_cfg, VIRTIO_BLK_C_PHYSICAL_BLOCK_EXP));
	uprintf(uio, "align_off: 0x%" PRIx8 "\n",
	        pci_r8(blk_cfg, VIRTIO_BLK_C_ALIGN_OFF));
	uprintf(uio, "min_io_size: 0x%" PRIx16 "\n",
	        pci_r16(blk_cfg, VIRTIO_BLK_C_MIN_IO_SIZE));
	uprintf(uio, "max_io_size: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_MAX_IO_SIZE));
	uprintf(uio, "writeback: 0x%" PRIx8 "\n",
	        pci_r8(blk_cfg, VIRTIO_BLK_C_WRITEBACK));
	uprintf(uio, "max_discard_sectors: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_MAX_DISCARD_SECTORS));
	uprintf(uio, "max_discard_seg: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_MAX_DISCARD_SEG));
	uprintf(uio, "discard_sector_alignment: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_DISCARD_SECTOR_ALIGN));
	uprintf(uio, "max_write_zeroes_sectors: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_MAX_ZEROES_SECTORS));
	uprintf(uio, "max_write_zeroes_seg: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_MAX_ZEROES_SEG));
	uprintf(uio, "write_zeroes_may_unmap: 0x%" PRIx32 "\n",
	        pci_r32(blk_cfg, VIRTIO_BLK_C_ZEROES_MAY_UNMAP));
}

static void virtio_blk_delete(struct virtio_blk *blk)
{
	if (!blk)
		return;
	dma_buf_free(blk->buf);
	pci_unmap(blk->dev.device, blk->blk_cfg);
	virtio_dev_destroy(&blk->dev);
	mutex_destroy(&blk->mutex);
	waitq_destroy(&blk->waitq);
	spinlock_destroy(&blk->waitq_lock);
	free(blk);
}

int init_pci(struct pci_device *device, void *userdata)
{
	struct virtio_blk *blk = NULL;
	uint8_t features[(VIRTIO_F_RING_RESET + 7) / 8];
	int ret;

	(void)userdata;
	blk = malloc(sizeof(*blk), M_ZERO);
	if (!blk)
	{
		TRACE("virtio_blk: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	mutex_init(&blk->mutex, 0);
	waitq_init(&blk->waitq);
	spinlock_init(&blk->waitq_lock);
	memset(features, 0, sizeof(features));
	ret = virtio_dev_init(&blk->dev, device, features, VIRTIO_F_RING_RESET);
	if (ret)
		goto err;
	if (blk->dev.queues_nb < 1)
	{
		TRACE("virtio_blk: no queues");
		ret = -EINVAL;
		goto err;
	}
	ret = virtio_get_cfg(device, VIRTIO_PCI_CAP_DEVICE_CFG,
	                     &blk->blk_cfg, 0x40, NULL);
	if (ret)
		goto err;
#if 0
	print_blk_cfg(NULL, &blk->blk_cfg);
#endif
	ret = dma_buf_alloc(PAGE_SIZE, 0, &blk->buf);
	if (ret)
	{
		TRACE("virtio_blk: buffer allocation failed");
		goto err;
	}
	blk->dev.queues[0].on_msg = on_msg;
	ret = virtq_setup_irq(&blk->dev.queues[0]);
	if (ret)
	{
		TRACE("virtio_blk: failed to setup irq");
		goto err;
	}
	virtio_dev_init_end(&blk->dev);
	uint64_t capacity = pci_r64(blk->blk_cfg, VIRTIO_BLK_C_CAPACITY);
	ret = disk_new("vbd", makedev(97, 0), capacity * BLOCK_SIZE,
	               &g_op, &blk->disk);
	if (ret)
	{
		TRACE("virtio_blk: failed to create disk");
		goto err;
	}
	blk->disk->userdata = blk;
	ret = disk_load(blk->disk);
	if (ret)
	{
		TRACE("virtio_blk: failed to load disk");
		goto err;
	}
	return 0;

err:
	virtio_blk_delete(blk);
	return ret;
}

static int init(void)
{
	pci_probe(0x1AF4, 0x1001, init_pci, NULL);
	return 0;
}

static void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "virtio/blk",
	.init = init,
	.fini = fini,
};
