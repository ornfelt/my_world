#define ENABLE_TRACE

#include "virtio.h"
#include "pci.h"

#include <kmod.h>
#include <std.h>

static inline void print_common_cfg(struct uio *uio, struct pci_map *common_cfg)
{
	uprintf(uio, "msix_config: 0x%" PRIx16 "\n",
	        pci_r16(common_cfg, VIRTIO_C_MSIX_CONFIG));
	uprintf(uio, "num_queues: 0x%" PRIx16 "\n",
	        pci_r16(common_cfg, VIRTIO_C_NUM_QUEUES));
	uprintf(uio, "device_status: 0x%" PRIx8 "\n",
	        pci_r8(common_cfg, VIRTIO_C_DEVICE_STATUS));
	uprintf(uio, "config_generation: 0x%" PRIx8 "\n",
	        pci_r8(common_cfg, VIRTIO_C_CONFIG_GENERATION));
}

static int get_pci_cap(struct pci_device *device, uint8_t cfg_type, uint8_t *ptr)
{
	if (!(device->header.status & (1 << 4)))
		return -EXDEV;
	*ptr = device->header0.capabilities & ~0x3;
	while (1)
	{
		if (!*ptr)
			return -EINVAL;
		uint32_t value = pci_dev_read(device, *ptr);
		if (((value >>  0) & 0xFF) == PCI_CAP_VNDR
		 && ((value >> 24) & 0xFF) == cfg_type)
			break;
		*ptr = (value >> 8) & 0xFF;
	}
	return 0;
}

int virtio_dev_has_feature(struct virtio_dev *dev, uint32_t feature)
{
	uint32_t off = feature / 32;
	uint32_t idx = feature % 32;
	pci_w32(dev->common_cfg, VIRTIO_C_DEVICE_FEATURE_SELECT, off);
	return !!(pci_r32(dev->common_cfg, VIRTIO_C_DEVICE_FEATURE) & (1 << idx));
}

static void enable_feature(struct virtio_dev *dev, uint32_t feature)
{
	uint32_t off = feature / 32;
	uint32_t idx = feature % 32;
	pci_w32(dev->common_cfg, VIRTIO_C_DRIVER_FEATURE_SELECT, off);
	uint32_t value = pci_r32(dev->common_cfg, VIRTIO_C_DRIVER_FEATURE);
	pci_w32(dev->common_cfg, VIRTIO_C_DRIVER_FEATURE, value | (1 << idx));
}

int virtio_get_cfg(struct pci_device *device, uint8_t cfg_type,
                   struct pci_map **cfg, size_t len, uint8_t *ptrp)
{
	uint8_t ptr;
	int ret = get_pci_cap(device, cfg_type, &ptr);
	if (ret)
	{
		TRACE("virtio: failed to get pci cap %" PRIu8, cfg_type);
		return ret;
	}
	union pci_cap_virtio cap;
	cap.v0 = pci_dev_read(device, ptr + 0x0);
	cap.v4 = pci_dev_read(device, ptr + 0x4);
	cap.v8 = pci_dev_read(device, ptr + 0x8);
	cap.vC = pci_dev_read(device, ptr + 0xC);
	if (cap.bar > 5)
	{
		TRACE("virtio: invalid cap bar: %" PRIu8, cap.bar);
		return -EINVAL;
	}
	if (cap.length < len)
	{
		TRACE("virtio: invalid cap length: %" PRIu32, cap.length);
		return -EINVAL;
	}
	ret = pci_map_bar(device, cap.bar, cap.length, cap.offset, cfg);
	if (ret)
		return ret;
	if (ptrp)
		*ptrp = ptr;
	return 0;
}

static int init_queues(struct virtio_dev *dev)
{
	dev->queues_nb = pci_r16(dev->common_cfg, VIRTIO_C_NUM_QUEUES);
	if (!dev->queues_nb)
	{
		dev->queues = NULL;
		return 0;
	}
	dev->queues = malloc(sizeof(*dev->queues) * dev->queues_nb, M_ZERO);
	for (size_t i = 0; i < dev->queues_nb; ++i)
	{
		int ret = virtq_init(&dev->queues[i], dev, i);
		if (ret)
			return ret;
	}
	return 0;
}

static int setup_notify(struct virtio_dev *dev)
{
	uint8_t ptr;
	int ret = virtio_get_cfg(dev->device, VIRTIO_PCI_CAP_NOTIFY_CFG,
	                         &dev->notify_cfg, 0, &ptr);
	if (ret)
		return ret;
	dev->notify_multiplier = pci_dev_read(dev->device, ptr + 0x10);
	return 0;
}

static void int_handler(void *userptr)
{
	struct virtio_dev *dev = userptr;
	if (dev->irq_handle.type != IRQ_MSIX)
	{
		uint8_t status = pci_r8(dev->isr_cfg, 0);
		if (status & 1)
		{
			for (size_t i = 0; i < dev->queues_nb; ++i)
				virtq_on_irq(&dev->queues[i]);
		}
		if (status & 2)
		{
			/* XXX configuration change callback */
		}
	}
	else
	{
		/* XXX configuration change callback */
	}
}

void virtio_dev_init_end(struct virtio_dev *dev)
{
	pci_w8(dev->common_cfg, VIRTIO_C_DEVICE_STATUS,
	       VIRTIO_S_ACKNOWLEDGE | VIRTIO_S_DRIVER | VIRTIO_S_FEATURES_OK | VIRTIO_S_DRIVER_OK);
}

int virtio_dev_init(struct virtio_dev *dev, struct pci_device *device,
                    const uint8_t *features, size_t features_count)
{
	int ret;

	dev->device = device;
	ret = virtio_get_cfg(device, VIRTIO_PCI_CAP_COMMON_CFG,
	                     &dev->common_cfg, 0x34, NULL);
	if (ret)
		return ret;
	pci_w8(dev->common_cfg, VIRTIO_C_DEVICE_STATUS, 0);
	if (pci_r8(dev->common_cfg, VIRTIO_C_DEVICE_STATUS) & VIRTIO_S_FAILED)
	{
		TRACE("virtio: reset failed");
		return -EINVAL;
	}
	pci_w8(dev->common_cfg, VIRTIO_C_DEVICE_STATUS,
	       VIRTIO_S_ACKNOWLEDGE | VIRTIO_S_DRIVER);
	if (pci_r8(dev->common_cfg, VIRTIO_C_DEVICE_STATUS) & VIRTIO_S_FAILED)
	{
		TRACE("virtio: reset failed");
		return -EINVAL;
	}
	for (size_t i = 0; i <= features_count; ++i)
	{
		if (!(features[i / 8] & (1 << (i % 8))))
			continue;
		if (!virtio_dev_has_feature(dev, i))
			continue;
		enable_feature(dev, i);
	}
	enable_feature(dev, VIRTIO_F_VERSION_1);
	pci_w8(dev->common_cfg, VIRTIO_C_DEVICE_STATUS,
	       VIRTIO_S_ACKNOWLEDGE | VIRTIO_S_DRIVER | VIRTIO_S_FEATURES_OK);
	if (!(pci_r8(dev->common_cfg, VIRTIO_C_DEVICE_STATUS) & VIRTIO_S_FEATURES_OK))
	{
		TRACE("virtio: feature not supported");
		return -EINVAL;
	}
#if 0
	print_common_cfg(NULL, &dev->common_cfg);
#endif
	ret = setup_notify(dev);
	if (ret)
		return ret;
	ret = virtio_get_cfg(device, VIRTIO_PCI_CAP_ISR_CFG,
	                     &dev->isr_cfg, 1, NULL);
	if (ret)
		return ret;
	ret = pci_register_irq(device, int_handler, dev, &dev->irq_handle);
	if (ret)
	{
		TRACE("virtio: failed to setup irq");
		return ret;
	}
	uint16_t vector;
	if (dev->irq_handle.type == IRQ_MSIX)
		vector = dev->irq_handle.msix.vector;
	else
		vector = 0xFFFF;
	pci_w16(dev->common_cfg, VIRTIO_C_MSIX_CONFIG, vector);
	ret = init_queues(dev);
	if (ret)
		return ret;
	return 0;
}

void virtio_dev_destroy(struct virtio_dev *dev)
{
	for (size_t i = 0; i < dev->queues_nb; ++i)
		virtq_destroy(&dev->queues[i]);
	pci_unregister_irq(dev->device, &dev->irq_handle);
	pci_unmap(dev->device, dev->common_cfg);
	pci_unmap(dev->device, dev->notify_cfg);
	pci_unmap(dev->device, dev->isr_cfg);
	free(dev->queues);
}

static int init(void)
{
	return 0;
}

static void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "virtio/core",
	.init = init,
	.fini = fini,
};
