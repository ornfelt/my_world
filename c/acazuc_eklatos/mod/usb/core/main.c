#define ENABLE_TRACE

#include "usb.h"

#include <kmod.h>
#include <mem.h>
#include <std.h>

static TAILQ_HEAD(, usb_device) devices = TAILQ_HEAD_INITIALIZER(devices); /* XXX lock */
static TAILQ_HEAD(, usb_itf_probe_list) itf_probes = TAILQ_HEAD_INITIALIZER(itf_probes); /* XXX lock */

ssize_t usb_ctrl_transfer(struct usb_device *device, uint8_t type,
                          uint8_t request, uint16_t value, uint16_t index,
                          void *data, size_t size)
{
	struct usb_request *req;
	uintptr_t req_addr;
	ssize_t ret;

	req = (struct usb_request*)device->control_buf->data;
	req->type = type;
	req->request = request;
	req->value = value;
	req->index = index;
	req->length = size;
	req_addr = pm_page_addr(device->control_buf->pages);
	ret = device->hcd->op->ctrl_transfer(device->hcd,
	                                     device,
	                                     (type & USB_REQ_DEV_TO_HOST) ? 0 : 1,
	                                     req_addr,
	                                     req_addr + sizeof(*req),
	                                     size);
	if (ret < 0)
		return ret;
	memcpy(data, &((uint8_t*)device->control_buf->data)[sizeof(*req)], ret);
	return ret;
}

static ssize_t get_descriptor(struct usb_device *device, uint16_t value,
                              uint16_t index, void *data, size_t size)
{
	return usb_ctrl_transfer(device,
	                         USB_REQ_DST_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_DEV_TO_HOST,
	                         USB_REQ_GET_DESCRIPTOR,
	                         value, index, data, size);
}

static ssize_t set_address(struct usb_device *device, uint8_t address)
{
	return usb_ctrl_transfer(device,
	                         USB_REQ_DST_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_HOST_TO_DEV,
	                         USB_REQ_SET_ADDRESS,
	                         address, 0, NULL, 0);
}

static ssize_t set_configuration(struct usb_device *device, uint8_t configuration)
{
	return usb_ctrl_transfer(device,
	                         USB_REQ_DST_DEVICE | USB_REQ_TYPE_STANDARD | USB_REQ_HOST_TO_DEV,
	                         USB_REQ_SET_CONFIGURATION,
	                         configuration, 0, NULL, 0);
}

ssize_t usb_use_interface(struct usb_device *device,
                          struct usb_interface *interface)
{
	return usb_ctrl_transfer(device,
	                         USB_REQ_DST_INTERFACE | USB_REQ_TYPE_STANDARD | USB_REQ_HOST_TO_DEV,
	                         USB_REQ_SET_INTERFACE,
	                         interface->desc.alternate_setting,
	                         interface->desc.interface, NULL, 0);
}

int usb_intr_transfer(struct usb_intr_pipe *pipe)
{
	return pipe->device->hcd->op->intr_transfer(pipe);
}

int usb_isoc_transfer(struct usb_isoc_pipe *pipe)
{
	return pipe->device->hcd->op->isoc_transfer(pipe);
}

static void print_usb_device_desc(struct uio *uio,
                                  const struct usb_device_desc *desc)
{
	uprintf(uio, "length: %u\n", desc->length);
	uprintf(uio, "type: %u\n", desc->type);
	uprintf(uio, "usb: %u\n", desc->usb);
	uprintf(uio, "class: %u\n", desc->class);
	uprintf(uio, "subclass: %u\n", desc->subclass);
	uprintf(uio, "protocol: %u\n", desc->protocol);
	uprintf(uio, "max_packet_size: %u\n", desc->max_packet_size);
	uprintf(uio, "vendor: %u\n", desc->vendor);
	uprintf(uio, "product: %u\n", desc->product);
	uprintf(uio, "device: %u\n", desc->device);
	uprintf(uio, "manufacturer_str: %u\n", desc->manufacturer_str);
	uprintf(uio, "product_str: %u\n", desc->product_str);
	uprintf(uio, "serial_str: %u\n", desc->serial_str);
	uprintf(uio, "configurations_count: %u\n", desc->configurations_count);
}

static void print_usb_endpoint_desc(struct uio *uio,
                                    const struct usb_endpoint_desc *desc)
{
	uprintf(uio, "length: %u\n", desc->length);
	uprintf(uio, "type: %u\n", desc->type);
	uprintf(uio, "address: %u\n", desc->address);
	uprintf(uio, "attributes: %u\n", desc->attributes);
	uprintf(uio, "max_packet_size: %u\n", desc->max_packet_size);
	uprintf(uio, "interval: %u\n", desc->interval);
}

static int get_language(struct usb_device *device, char *str, uint8_t index,
                        uint16_t langid)
{
	uint8_t buf[256];
	uint8_t length;
	ssize_t ret;

	ret = get_descriptor(device, (USB_DESC_STRING << 8) | index,
	                     langid, &length, sizeof(length));
	if (ret < 0)
	{
		TRACE("usb: failed to get string descriptor length");
		return ret;
	}
	if (ret != sizeof(length))
	{
		TRACE("usb: string descriptor length not complete");
		return -EINVAL;
	}
	if (length < 2)
	{
		TRACE("usb: invalid string descriptor length: %u", length);
		return -EINVAL;
	}
	ret = get_descriptor(device, (USB_DESC_STRING << 8) | index,
	                     langid, buf, length);
	if (ret < 0)
	{
		TRACE("usb: failed to get full string descriptor");
		return ret;
	}
	if (ret != length)
	{
		TRACE("usb: string descriptor not complete");
		return ret;
	}
	/* XXX convert utf16 to utf8 */
	for (size_t i = 2; i < length; i += 2)
		str[(i - 2) / 2] = buf[i];
	str[(length - 2) / 2] = '\0';
	return 0;
}

static int get_languages(struct usb_device *device)
{
	struct
	{
		uint8_t length;
		uint8_t type;
		uint16_t langs[127];
	} data;
	uint8_t length;
	ssize_t ret;

	ret = get_descriptor(device, USB_DESC_STRING << 8, 0,
	                     &length, sizeof(length));
	if (ret < 0)
	{
		TRACE("usb: failed to get strings descriptor length");
		return ret;
	}
	if (ret != sizeof(length))
	{
		TRACE("usb: languages descriptor length not complete");
		return -EINVAL;
	}
	if (length & 1 || length < 4)
	{
		TRACE("usb: invalid strings descriptor length: %u", length);
		return -EINVAL;
	}
	ret = get_descriptor(device, USB_DESC_STRING << 8, 0,
	                     &data, length);
	if (ret < 0)
	{
		TRACE("usb: failed to get full strings descriptor");
		return ret;
	}
	if (ret != length)
	{
		TRACE("usb: string descriptors not complete");
		return ret;
	}
	device->langs_count = (length - 2) / 2;
	device->langs = malloc(sizeof(*device->langs) * device->langs_count, 0);
	if (!device->langs)
	{
		TRACE("usb: failed to allocate device langs");
		return -ENOMEM;
	}
	device->lang = 0;
	memcpy(device->langs, data.langs, length - 2);
	ret = get_language(device, device->manufacturer,
	                   device->desc.manufacturer_str,
	                   device->langs[device->lang]);
	if (ret)
		return ret;
	ret = get_language(device, device->product,
	                   device->desc.product_str,
	                   device->langs[device->lang]);
	if (ret)
		return ret;
	ret = get_language(device, device->serial,
	                   device->desc.serial_str,
	                   device->langs[device->lang]);
	if (ret)
		return ret;
	return 0;
}

static int get_device_descriptor(struct usb_device *device)
{
	ssize_t ret;

	ret = get_descriptor(device, USB_DESC_DEVICE << 8, 0,
	                     &device->desc, 8);
	if (ret < 0)
	{
		TRACE("usb: failed to get device descriptor header");
		return ret;
	}
	if (ret != 8)
	{
		TRACE("usb: device descriptor header not complete");
		return -EINVAL;
	}
	ret = get_descriptor(device, USB_DESC_DEVICE << 8, 0,
	                     &device->desc, sizeof(device->desc));
	if (ret < 0)
	{
		TRACE("usb: failed to get device descriptor");
		return ret;
	}
	if (ret != sizeof(device->desc))
	{
		TRACE("usb: devie descriptor not complete");
		return -EINVAL;
	}
	return 0;
}

static int get_configurations(struct usb_device *device)
{
	ssize_t ret;

	if (!device->desc.configurations_count)
	{
		TRACE("usb: no device configurations");
		return -EINVAL;
	}
	device->configurations = malloc(sizeof(*device->configurations) * device->desc.configurations_count, 0);
	if (!device->configurations)
	{
		TRACE("usb: configrations allocation failed");
		return -ENOMEM;
	}
	for (size_t i = 0; i < device->desc.configurations_count; ++i)
	{
		ret = get_descriptor(device,
		                     (USB_DESC_CONFIGURATION << 8) | i, 0,
		                     &device->configurations[i],
		                     sizeof(*device->configurations));
		if (ret < 0)
		{
			TRACE("usb; failed to get configuration");
			return ret;
		}
		if (ret != sizeof(*device->configurations))
		{
			TRACE("usb: configurations not complete");
			return ret;
		}
	}
	return 0;
}

static void interface_free(struct usb_interface *interface)
{
	struct usb_descriptor *descriptor;
	while ((descriptor = TAILQ_FIRST(&interface->descriptors)))
	{
		TAILQ_REMOVE(&interface->descriptors, descriptor, chain);
		free(descriptor);
	}
	struct usb_endpoint *endpoint;
	while ((endpoint = TAILQ_FIRST(&interface->endpoints)))
	{
		TAILQ_REMOVE(&interface->endpoints, endpoint, chain);
		free(endpoint);
	}
	free(interface);
}

static void interfaces_free(struct usb_interface_head *interfaces)
{
	struct usb_interface *interface;
	while ((interface = TAILQ_FIRST(interfaces)))
	{
		TAILQ_REMOVE(interfaces, interface, chain);
		interface_free(interface);
	}
}

static int parse_interface(struct usb_device *device, const uint8_t *buf,
                           size_t size, size_t *buf_pos,
                           struct usb_interface **interfacep)
{
	struct usb_interface *interface = NULL;
	struct usb_interface_desc *desc;
	char str_buf[182];
	ssize_t ret;

	if (size - *buf_pos < sizeof(*desc))
	{
		TRACE("usb: interface descriptor overflow");
		ret = -EINVAL;
		goto err;
	}
	desc = (struct usb_interface_desc*)&buf[*buf_pos];
	*buf_pos += sizeof(*desc);
	if (desc->length != sizeof(*desc)
	 || desc->type != USB_DESC_INTERFACE)
	{
		TRACE("usb: malformed interface descriptor");
		ret = -EINVAL;
		goto err;
	}
	interface = malloc(sizeof(*interface), 0);
	if (!interface)
	{
		TRACE("usb: interface allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	TAILQ_INIT(&interface->descriptors);
	TAILQ_INIT(&interface->endpoints);
	interface->desc = *desc;
	while (size - *buf_pos >= 2)
	{
		uint8_t length = buf[*buf_pos];
		uint8_t type = buf[*buf_pos + 1];
		if (type == USB_DESC_INTERFACE)
			break;
		if (length < 2)
		{
			TRACE("usb: malformed entry");
			ret = -EINVAL;
			goto err;
		}
		if (size - *buf_pos < length)
		{
			TRACE("usb: entry overflow"),
			ret = -EINVAL;
			goto err;
		}
		if (type == USB_DESC_ENDPOINT)
		{
			struct usb_endpoint *endpoint;
			if (length < sizeof(endpoint->desc))
			{
				TRACE("usb: malformed endpoint descriptor: %u / %zu",
				      length, sizeof(endpoint->desc));
				ret = -EINVAL;
				goto err;
			}
			endpoint = malloc(sizeof(*endpoint) - sizeof(endpoint->desc) + length, 0);
			if (!endpoint)
			{
				TRACE("usb; endpoint allocation failed");
				ret = -ENOMEM;
				goto err;
			}
			memcpy(&endpoint->desc, &buf[*buf_pos], length);
#if 0
			print_usb_endpoint_desc(NULL, &endpoint->desc);
#endif
			TAILQ_INSERT_TAIL(&interface->endpoints, endpoint, chain);
		}
		else
		{
			struct usb_descriptor *descriptor = malloc(sizeof(*descriptor) + length - 2, 0);
			if (!descriptor)
			{
				TRACE("usb: descriptor allocation failed");
				ret = -ENOMEM;
				goto err;
			}
			descriptor->desc.length = length;
			descriptor->desc.type = type;
			memcpy(descriptor->desc.data, &buf[*buf_pos + 2], length - 2);
			TAILQ_INSERT_TAIL(&interface->descriptors, descriptor, chain);
		}
		*buf_pos += length;
	}
	*interfacep = interface;
	return 0;

err:
	interface_free(interface);
	return ret;
}

static int select_configuration(struct usb_device *device, uint8_t confid)
{
	struct usb_configuration_desc *configuration;
	struct usb_interface_head interfaces;
	uint8_t buf[PAGE_SIZE]; /* XXX fix this */
	char str_buf[182];
	size_t buf_pos = 0;
	ssize_t ret;

	TAILQ_INIT(&interfaces);
	configuration = &device->configurations[confid];
	/* XXX use another way */
	if (configuration->total_length > PAGE_SIZE)
	{
		TRACE("usb: configuration too big");
		ret = -EINVAL;
		goto err;
	}
	ret = get_descriptor(device, (USB_DESC_CONFIGURATION << 8) | confid,
	                     0, buf, configuration->total_length);
	if (ret < 0)
	{
		TRACE("usb: failed to get full configuration");
		goto err;
	}
	if (ret != configuration->total_length)
	{
		TRACE("usb:; full configuration not complete");
		goto err;
	}
	for (buf_pos = configuration->length; buf_pos < configuration->total_length;)
	{
		struct usb_interface *interface;
		ret = parse_interface(device, buf, configuration->total_length,
		                      &buf_pos, &interface);
		if (ret)
			goto err;
		TAILQ_INSERT_TAIL(&interfaces, interface, chain);
	}
	if (buf_pos != configuration->total_length)
	{
		TRACE("usb: configurations underflow: %u / %u",
		      buf_pos, configuration->total_length);
		ret = -EINVAL;
		goto err;
	}
	ret = set_configuration(device, configuration->configuration_value);
	if (ret)
	{
		TRACE("usb: failed to set configuration");
		goto err;
	}
	interfaces_free(&device->interfaces);
	device->interfaces = interfaces;
	device->configuration = confid;
	return 0;

err:
	interfaces_free(&interfaces);
	return ret;
}

static int itf_probe_list_matches(const struct usb_interface *interface,
                                  const struct usb_itf_probe *probes,
                                  size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		if (interface->desc.class == probes[i].class
		 && interface->desc.subclass == probes[i].subclass
		 && interface->desc.protocol == probes[i].protocol)
			return 0;
	}
	return -EINVAL;
}

void usb_device_free(struct usb_device *device)
{
	if (!device)
		return;
	dma_buf_free(device->control_buf);
	interfaces_free(&device->interfaces);
	free(device->configurations);
	free(device->langs);
	free(device);
}

int usb_device_alloc(struct usb_hcd *hcd, enum usb_speed speed,
                     struct usb_device **devicep)
{
	struct usb_device *device;
	uint8_t addr;
	int ret;

	device = malloc(sizeof(*device), M_ZERO); /* XXX sma */
	if (!device)
	{
		TRACE("usb: device allocation failed");
		return -ENOMEM;
	}
	device->hcd = hcd;
	device->speed = speed;
	switch (speed)
	{
		case USB_SPEED_LOW:
		case USB_SPEED_FULL:
			device->desc.max_packet_size = 8;
			break;
		case USB_SPEED_HIGH:
			device->desc.max_packet_size = 16;
			break;
		case USB_SPEED_SUPER:
			device->desc.max_packet_size = 128; /* XXX should be 512? */
			break;
	}
	TAILQ_INIT(&device->interfaces);
	ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &device->control_buf); /* XXX hcd-specific flags */
	if (ret)
	{
		TRACE("usb: device control buf allocation failed");
		goto err;
	}
	ret = get_device_descriptor(device);
	if (ret)
	{
		TRACE("usb: failed to get device descriptor");
		goto err;
	}
#if 0
	print_usb_device_desc(NULL, &device->desc);
#endif
	ret = hcd->op->get_addr(hcd, &addr);
	if (ret)
	{
		TRACE("usb: failed to get addr");
		goto err;
	}
	ret = set_address(device, addr);
	if (ret)
	{
		TRACE("usb: failed to set address");
		goto err;
	}
	device->addr = addr;
	ret = get_languages(device);
	if (ret)
	{
		TRACE("usb: failed to get languages");
		goto err;
	}
	ret = get_configurations(device);
	if (ret)
	{
		TRACE("usb: failed to get configurations");
		goto err;
	}
	ret = select_configuration(device, 0); /* XXX allow configuration change */
	if (ret)
	{
		TRACE("usb; failed to select configuration");
		goto err;
	}
	*devicep = device;
	TAILQ_INSERT_TAIL(&devices, device, chain);
	return 0;

err:
	usb_device_free(device);
	return ret;
}

void usb_device_probe(struct usb_device *device)
{
	if (TAILQ_EMPTY(&itf_probes))
		return;
	struct usb_interface *interface;
	TAILQ_FOREACH(interface, &device->interfaces, chain)
	{
		struct usb_itf_probe_list *probe_list;
		TAILQ_FOREACH(probe_list, &itf_probes, chain)
		{
			if (itf_probe_list_matches(interface,
			                            probe_list->probes,
			                            probe_list->count))
				continue;
			if (!probe_list->probe(device,
			                       interface,
			                       probe_list->userdata))
				return;
		}
	}
}

int usb_register_itf_probes(const struct usb_itf_probe *probes, size_t count,
                            usb_itf_probe_t probe, void *userdata)
{
	struct usb_itf_probe_list *probe_list = NULL;
	struct usb_device *device;

	probe_list = malloc(sizeof(*probe_list), M_ZERO);
	if (!probe_list)
		return -ENOMEM;
	probe_list->probes = probes;
	probe_list->count = count;
	probe_list->probe = probe;
	probe_list->userdata = userdata;
	TAILQ_INSERT_TAIL(&itf_probes, probe_list, chain);
	TAILQ_FOREACH(device, &devices, chain)
	{
		struct usb_interface *interface;
		TAILQ_FOREACH(interface, &device->interfaces, chain)
		{
			if (interface->userdata)
				continue;
			if (itf_probe_list_matches(interface,
			                            probe_list->probes,
			                            probe_list->count))
				continue;
			if (!probe_list->probe(device,
			                       interface,
			                       probe_list->userdata))
				return 0;
		}
	}
	return 0;
}

int usb_intr_pipe_alloc(struct usb_device *device,
                        struct usb_endpoint *endpoint,
                        size_t size, usb_intr_pipe_fn_t fn,
                        void *userdata,
                        struct usb_intr_pipe **pipep)
{
	struct usb_intr_pipe *pipe = NULL;
	int ret;

	if (!size)
		return -EINVAL;
	pipe = malloc(sizeof(*pipe), M_ZERO); /* XXX sma */
	if (!pipe)
	{
		TRACE("usb: interrupt pipe allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = dma_buf_alloc(size, DMA_32BIT, &pipe->buf); /* XXX hcd-specific flags */
	if (ret)
	{
		TRACE("usb: interrupt pipe page allocation failed");
		goto err;
	}
	pipe->device = device;
	pipe->endpoint = endpoint;
	pipe->userdata = userdata;
	pipe->fn = fn;
	*pipep = pipe;
	return 0;

err:
	usb_intr_pipe_free(pipe);
	return ret;
}

void usb_intr_pipe_free(struct usb_intr_pipe *pipe)
{
	if (!pipe)
		return;
	dma_buf_free(pipe->buf);
	free(pipe);
}

int usb_isoc_pipe_alloc(struct usb_device *device,
                        struct usb_endpoint *endpoint,
                        usb_isoc_pipe_fn_t fn,
                        void *userdata,
                        struct usb_isoc_pipe **pipep)
{
	struct usb_isoc_pipe *pipe = NULL;
	int ret;

	pipe = malloc(sizeof(*pipe), M_ZERO); /* XXX sma */
	if (!pipe)
	{
		TRACE("usb: isochronous pipe allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = dma_buf_alloc(PAGE_SIZE, DMA_32BIT, &pipe->buf); /* XXX flags hcd-specific */
	if (ret)
	{
		TRACE("usb: isochronous pipe buf allocation failed");
		goto err;
	}
	pipe->device = device;
	pipe->endpoint = endpoint;
	pipe->userdata = userdata;
	pipe->fn = fn;
	*pipep = pipe;
	return 0;

err:
	usb_isoc_pipe_free(pipe);
	return ret;
}

void usb_isoc_pipe_free(struct usb_isoc_pipe *pipe)
{
	if (!pipe)
		return;
	dma_buf_free(pipe->buf);
	free(pipe);
}

int init(void)
{
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "usb/core",
	.init = init,
	.fini = fini,
};
