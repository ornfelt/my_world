#ifndef VIRTIO_H
#define VIRTIO_H

#include <types.h>
#include <irq.h>

#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
#define VIRTIO_PCI_CAP_ISR_CFG    3
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_PCI_CAP_PCI_CFG    5

#define VIRTIO_C_DEVICE_FEATURE_SELECT 0x00
#define VIRTIO_C_DEVICE_FEATURE        0x04
#define VIRTIO_C_DRIVER_FEATURE_SELECT 0x08
#define VIRTIO_C_DRIVER_FEATURE        0x0C
#define VIRTIO_C_MSIX_CONFIG           0x10
#define VIRTIO_C_NUM_QUEUES            0x12
#define VIRTIO_C_DEVICE_STATUS         0x14
#define VIRTIO_C_CONFIG_GENERATION     0x15
#define VIRTIO_C_QUEUE_SELECT          0x16
#define VIRTIO_C_QUEUE_SIZE            0x18
#define VIRTIO_C_QUEUE_MSIX_VECTOR     0x1A
#define VIRTIO_C_QUEUE_ENABLE          0x1C
#define VIRTIO_C_QUEUE_NOTIFY_OFF      0x1E
#define VIRTIO_C_QUEUE_DESC            0x20
#define VIRTIO_C_QUEUE_DRIVER          0x28
#define VIRTIO_C_QUEUE_DEVICE          0x30

#define VIRTIO_F_NOTIFY_ON_EMPTY   24
#define VIRTIO_F_ANY_LAYOUT        27
#define VIRTIO_F_INDIRECT_DESC     28
#define VIRTIO_F_EVENT_IDX         29
#define VIRTIO_F_VERSION_1         32
#define VIRTIO_F_ACCESS_PLATFORM   33
#define VIRTIO_F_RING_PACKED       34
#define VIRTIO_F_IN_ORDER          35
#define VIRTIO_F_ORDER_PLATFORM    36
#define VIRTIO_F_SR_IOV            37
#define VIRTIO_F_NOTIFICATION_DATA 38
#define VIRTIO_F_NOTIF_CONFIG_DATA 39
#define VIRTIO_F_RING_RESET        40

#define VIRTIO_S_ACKNOWLEDGE        1
#define VIRTIO_S_DRIVER             2
#define VIRTIO_S_DRIVER_OK          4
#define VIRTIO_S_FEATURES_OK        8
#define VIRTIO_S_DEVICE_NEEDS_RESET 64
#define VIRTIO_S_FAILED             128

struct virtq_avail;
struct virtq_desc;
struct virtq_used;
struct virtio_dev;
struct pci_device;
struct pci_map;
struct sg_head;
struct virtq;

typedef void (*virtq_on_msg_t)(struct virtq *queue, uint16_t id, uint32_t len);

struct virtq
{
	struct virtio_dev *dev;
	size_t desc_size;
	size_t avail_size;
	size_t used_size;
	uint16_t id;
	uint16_t size;
	uint16_t desc_head;
	uint16_t used_tail;
	struct dma_buf *desc;
	struct dma_buf *avail;
	struct dma_buf *used;
	int has_msix;
	struct irq_handle irq_handle;
	virtq_on_msg_t on_msg;
};

struct virtio_dev
{
	struct pci_device *device;
	struct pci_map *common_cfg;
	struct pci_map *notify_cfg;
	struct pci_map *isr_cfg;
	struct virtq *queues;
	uint32_t notify_multiplier;
	uint16_t queues_nb;
	struct irq_handle irq_handle;
};

int virtio_dev_init(struct virtio_dev *dev,
                    struct pci_device *device,
                    const uint8_t *features,
                    size_t features_count);
void virtio_dev_init_end(struct virtio_dev *dev);
void virtio_dev_destroy(struct virtio_dev *dev);
int virtio_dev_has_feature(struct virtio_dev *dev, uint32_t feature);

int virtio_get_cfg(struct pci_device *device,
                   uint8_t cfg_type,
                   struct pci_map **cfg,
                   size_t len,
                   uint8_t *ptr);

int virtq_init(struct virtq *queue, struct virtio_dev *dev, uint16_t id);
void virtq_destroy(struct virtq *queue);
int virtq_send(struct virtq *queue,
               const struct sg_head *sg_read,
               const struct sg_head *sg_write);
void virtq_notify(struct virtq *queue);
int virtq_setup_irq(struct virtq *queue);
void virtq_on_irq(struct virtq *queue);
int virtq_poll(struct virtq *queue, uint16_t *id, uint32_t *len);

#endif
