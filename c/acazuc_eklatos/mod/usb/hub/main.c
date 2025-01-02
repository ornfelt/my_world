#define ENABLE_TRACE

#include "usb.h"

#include <kmod.h>
#include <mem.h>

#define PORT_CONNECTION   (1 << 0)
#define PORT_ENABLE       (1 << 1)
#define PORT_SUSPEND      (1 << 2)
#define PORT_OVER_CURRENT (1 << 3)
#define PORT_RESET        (1 << 4)
#define PORT_POWER        (1 << 8)
#define PORT_LOW_SPEED    (1 << 9)
#define PORT_HIGH_SPEED   (1 << 10)
#define PORT_TEST         (1 << 11)
#define PORT_INDICATOR    (1 << 12)

#define MAX_PORTS 8

static const struct usb_itf_probe probes[] =
{
	{USB_CLASS_HUB, 0, 0},
};

struct usb_hub
{
	struct usb_device *device;
	struct usb_interface *interface;
	struct usb_endpoint *endpoint;
	struct usb_hub_desc desc;
	struct usb_device *devices[MAX_PORTS];
};

static int reset_port(struct usb_hub *hub, size_t port)
{
	enum usb_speed speed;
	uint32_t status;
	ssize_t ret;

	ret = usb_ctrl_transfer(hub->device,
	                        USB_REQ_DST_OTHER | USB_REQ_TYPE_CLASS | USB_REQ_HOST_TO_DEV,
	                        USB_REQ_SET_FEATURE, USB_PORT_RESET,
	                        port + 1, NULL, 0);
	if (ret < 0)
	{
		TRACE("usb_hub: failed to reset port");
		return ret;
	}
	for (size_t i = 0; i < 10; ++i)
	{
		ret = usb_ctrl_transfer(hub->device,
		                        USB_REQ_DST_OTHER | USB_REQ_TYPE_CLASS | USB_REQ_DEV_TO_HOST,
		                        USB_REQ_GET_STATUS, 0, port + 1,
		                        &status, sizeof(status));
		if (ret < 0)
		{
			TRACE("usb_hub: failed to get port status");
			return ret;
		}
		if (ret != sizeof(status))
		{
			TRACE("usb_hub: status transfer not complete");
			return -EINVAL;
		}
		if (status & PORT_RESET)
		{
			static const struct timespec delay = {0, 10000000};
			spinsleep(&delay);
			continue;
		}
		if ((status & (PORT_CONNECTION | PORT_ENABLE)) != (PORT_CONNECTION | PORT_ENABLE))
			return 0;
		if (status & PORT_LOW_SPEED)
			speed = USB_SPEED_LOW;
		else if (status & PORT_HIGH_SPEED)
			speed = USB_SPEED_HIGH;
		else
			speed = USB_SPEED_FULL;
		ret = usb_device_alloc(hub->device->hcd, speed, &hub->devices[port]);
		if (ret)
		{
			TRACE("usb_hub: failed to create device");
			return ret;
		}
		usb_device_probe(hub->devices[port]);
		return 0;
	}
	return -ETIMEDOUT;
}

static void usb_hub_free(struct usb_hub *hub)
{
	if (!hub)
		return;
	free(hub);
}

static int usb_init(struct usb_device *device, struct usb_interface *interface,
                    void *userdata)
{
	struct usb_hub *hub = NULL;
	int ret;

	(void)userdata;
	hub = malloc(sizeof(*hub), M_ZERO);
	if (!hub)
	{
		TRACE("usb_hub: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	hub->device = device;
	hub->interface = interface;
	hub->endpoint = TAILQ_FIRST(&interface->endpoints);
	if (!hub->endpoint)
	{
		TRACE("usb_hub: no endpoint");
		ret = -EINVAL;
		goto err;
	}
	ret = usb_ctrl_transfer(hub->device,
	                        USB_REQ_DST_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_DEV_TO_HOST,
	                        USB_REQ_GET_DESCRIPTOR,
	                        USB_DESC_HUB << 8, 0,
	                        &hub->desc, sizeof(hub->desc));
	if (ret < 0)
	{
		TRACE("usb_hub: failed to get descriptor");
		goto err;
	}
	if (ret != sizeof(hub->desc))
	{
		TRACE("usb_hub: hub descriptor not complete");
		goto err;
	}
	for (size_t i = 0; i < hub->desc.ports && i < MAX_PORTS; ++i)
		reset_port(hub, i);
	interface->userdata = hub;
	return 0;

err:
	usb_hub_free(hub);
	return ret;
}

int init(void)
{
	int ret;

	ret = usb_register_itf_probes(probes,
	                              sizeof(probes) / sizeof(*probes),
	                              usb_init, NULL);
	if (ret)
	{
		TRACE("usb_mouse: failed to register probes");
		return ret;
	}
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "usb/hub",
	.init = init,
	.fini = fini,
};
