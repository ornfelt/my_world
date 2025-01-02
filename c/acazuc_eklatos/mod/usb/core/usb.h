#ifndef USB_H
#define USB_H

#include <queue.h>
#include <std.h>

#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE     0x0A
#define USB_REQ_SET_INTERFACE     0x0B
#define USB_REQ_SYNCH_FRAME       0x0C

#define USB_REQ_DST_DEVICE    (0 << 0)
#define USB_REQ_DST_INTERFACE (1 << 0)
#define USB_REQ_DST_ENDPOINT  (2 << 0)
#define USB_REQ_DST_OTHER     (3 << 0)

#define USB_REQ_TYPE_STANDARD (0 << 5)
#define USB_REQ_TYPE_CLASS    (1 << 5)
#define USB_REQ_TYPE_VENDOR   (2 << 5)

#define USB_REQ_HOST_TO_DEV (0 << 7)
#define USB_REQ_DEV_TO_HOST (1 << 7)

#define USB_PORT_CONNECTION  0
#define USB_PORT_ENABLE      1
#define USB_PORT_SUSPEND     2
#define USB_PORT_OVERCURRENT 3
#define USB_PORT_RESET       4
#define USB_PORT_POWER       8

#define HID_SET_IDLE 0x0A

#define USB_PID_IN    0x69
#define USB_PID_OUT   0xE1
#define USB_PID_SETUP 0x2D

#define USB_CLASS_AUDIO        1
#define USB_CLASS_COMM         2
#define USB_CLASS_HID          3
#define USB_CLASS_PHYSICAL     5
#define USB_CLASS_STILL_IMAGE  6
#define USB_CLASS_PRINTER      7
#define USB_CLASS_MASS_STORAGE 8
#define USB_CLASS_HUB          9

#define USB_SUBCLASS_AUDIO_CONTROL       1
#define USB_SUBCLASS_AUDIO_STREAMING     2
#define USB_SUBCLASS_AUDIO_MIDISTREAMING 3

#define USB_DESC_DEVICE        0x01
#define USB_DESC_CONFIGURATION 0x02
#define USB_DESC_STRING        0x03
#define USB_DESC_INTERFACE     0x04
#define USB_DESC_ENDPOINT      0x05
#define USB_DESC_HUB           0x29

enum usb_speed
{
	USB_SPEED_LOW,
	USB_SPEED_FULL,
	USB_SPEED_HIGH,
	USB_SPEED_SUPER,
};

struct usb_request
{
	uint8_t type;
	uint8_t request;
	uint16_t value;
	uint16_t index;
	uint16_t length;
};

struct usb_device_desc
{
	uint8_t length;
	uint8_t type;
	uint16_t usb;
	uint8_t class;
	uint8_t subclass;
	uint8_t protocol;
	uint8_t max_packet_size;
	uint16_t vendor;
	uint16_t product;
	uint16_t device;
	uint8_t manufacturer_str;
	uint8_t product_str;
	uint8_t serial_str;
	uint8_t configurations_count;
};

struct usb_configuration_desc
{
	uint8_t length;
	uint8_t type;
	uint16_t total_length;
	uint8_t interfaces_count;
	uint8_t configuration_value;
	uint8_t configuration_str;
	uint8_t attributes;
	uint8_t max_power;
};

struct usb_interface_desc
{
	uint8_t length;
	uint8_t type;
	uint8_t interface;
	uint8_t alternate_setting;
	uint8_t endpoints_count;
	uint8_t class;
	uint8_t subclass;
	uint8_t protocol;
	uint8_t interface_str;
};

struct usb_endpoint_desc
{
	uint8_t length;
	uint8_t type;
	uint8_t address;
	uint8_t attributes;
	uint16_t max_packet_size;
	uint8_t interval;
	uint8_t data[];
} __attribute__((packed));

struct usb_string_desc
{
	uint8_t length;
	uint8_t type;
	uint16_t langs[];
};

struct usb_hub_desc
{
	uint8_t length;
	uint8_t type;
	uint8_t ports;
	uint16_t characteristics;
	uint8_t power_delay;
	uint8_t control_current;
	/* XXX remove and power mask */
} __attribute__((packed));

struct usb_descriptor
{
	TAILQ_ENTRY(usb_descriptor) chain;
	struct
	{
		uint8_t length;
		uint8_t type;
		uint8_t data[];
	} desc;
};

TAILQ_HEAD(usb_descriptor_head, usb_descriptor);

struct usb_endpoint
{
	TAILQ_ENTRY(usb_endpoint) chain;
	struct usb_endpoint_desc desc;
};

TAILQ_HEAD(usb_endpoint_head, usb_endpoint);

struct usb_interface
{
	struct usb_interface_desc desc;
	struct usb_descriptor_head descriptors;
	struct usb_endpoint_head endpoints;
	void *userdata;
	TAILQ_ENTRY(usb_interface) chain;
};

TAILQ_HEAD(usb_interface_head, usb_interface);

struct usb_device
{
	struct dma_buf *control_buf;
	struct usb_device_desc desc;
	enum usb_speed speed;
	uint8_t addr;
	char manufacturer[128];
	char product[128];
	char serial[128];
	struct usb_hcd *hcd;
	uint16_t *langs;
	uint8_t langs_count;
	uint8_t lang;
	struct usb_configuration_desc *configurations;
	uint8_t configuration;
	struct usb_interface_head interfaces;
	TAILQ_ENTRY(usb_device) chain;
};

struct usb_hcd;

struct usb_intr_pipe;

typedef void (*usb_intr_pipe_fn_t)(struct usb_intr_pipe *pipe,
                                   ssize_t ret);

struct usb_intr_pipe
{
	struct usb_device *device;
	struct usb_endpoint *endpoint;
	struct dma_buf *buf;
	void *userdata;
	usb_intr_pipe_fn_t fn;
};

struct usb_isoc_pipe;

typedef void (*usb_isoc_pipe_fn_t)(struct usb_isoc_pipe *pipe,
                                   void *data, size_t *sizes, size_t nbufs);

struct usb_isoc_pipe
{
	struct usb_device *device;
	struct usb_endpoint *endpoint;
	struct dma_buf *buf;
	void *userdata;
	usb_isoc_pipe_fn_t fn;
};

struct usb_hcd_op
{
	int (*get_addr)(struct usb_hcd *hcd, uint8_t *addr);
	ssize_t (*ctrl_transfer)(struct usb_hcd *hcd,
	                         struct usb_device *device,
	                         int in_out, uint32_t req,
	                         uint32_t data, size_t size);
	int (*intr_transfer)(struct usb_intr_pipe *pipe);
	int (*isoc_transfer)(struct usb_isoc_pipe *pipe);
};

struct usb_hcd
{
	const struct usb_hcd_op *op;
	void *userdata;
};

struct usb_itf_probe
{
	uint8_t class;
	uint8_t subclass;
	uint8_t protocol;
};

typedef int (*usb_itf_probe_t)(struct usb_device *device,
                               struct usb_interface *interface,
                               void *userdata);

struct usb_itf_probe_list
{
	const struct usb_itf_probe *probes;
	size_t count;
	usb_itf_probe_t probe;
	void *userdata;
	TAILQ_ENTRY(usb_itf_probe_list) chain;
};

int usb_device_alloc(struct usb_hcd *hcd, enum usb_speed speed,
                     struct usb_device **device);
void usb_device_probe(struct usb_device *device);
int usb_register_itf_probes(const struct usb_itf_probe *probes, size_t count,
                            usb_itf_probe_t probe, void *userdata);
ssize_t usb_use_interface(struct usb_device *device,
                          struct usb_interface *interface);

ssize_t usb_ctrl_transfer(struct usb_device *device, uint8_t type,
                          uint8_t request, uint16_t value, uint16_t index,
                          void *data, size_t size);
int usb_intr_transfer(struct usb_intr_pipe *pipe);
int usb_isoc_transfer(struct usb_isoc_pipe *pipe);

int usb_intr_pipe_alloc(struct usb_device *device,
                        struct usb_endpoint *endpoint,
                        size_t size, usb_intr_pipe_fn_t fn,
                        void *userdata,
                        struct usb_intr_pipe **pipep);
void usb_intr_pipe_free(struct usb_intr_pipe *pipe);

int usb_isoc_pipe_alloc(struct usb_device *device,
                        struct usb_endpoint *endpoint,
                        usb_isoc_pipe_fn_t fn,
                        void *userdata,
                        struct usb_isoc_pipe **pipep);
void usb_isoc_pipe_free(struct usb_isoc_pipe *pipe);

#endif
