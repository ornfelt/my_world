#define ENABLE_TRACE

#include "evdev.h"
#include "usb.h"

#include <kmod.h>
#include <std.h>
#include <tty.h>
#include <mem.h>

static const struct usb_itf_probe
probes[] =
{
	{USB_CLASS_HID, 1, 2},
};

struct usb_mouse
{
	struct usb_device *device;
	struct usb_interface *interface;
	struct usb_endpoint *endpoint;
	struct usb_intr_pipe *intr_pipe;
	struct evdev *evdev;
};

static void
intr_pipe_fn(struct usb_intr_pipe *pipe, ssize_t ret)
{
	struct usb_mouse *mouse = pipe->userdata;
	uint8_t *data = pipe->buf->data;

	if (ret < 0)
	{
		TRACE("usb_mouse: interrupt err: %d", ret);
		return;
	}
	if (ret != 4)
	{
		TRACE("usb_mouse: interrupt pipe not complete");
		return;
	}
	if (data[0])
	{
		ev_send_mouse_event(mouse->evdev, data[0] - 1, 1);
		ev_send_mouse_event(mouse->evdev, data[0] - 1, 0);
	}
	if (data[1] || data[2])
		ev_send_pointer_event(mouse->evdev, (int8_t)data[1], (int8_t)data[2]);
	if (data[3])
		ev_send_scroll_event(mouse->evdev, 0, (int8_t)data[3]);
}

static void
usb_mouse_free(struct usb_mouse *mouse)
{
	if (!mouse)
		return;
	usb_intr_pipe_free(mouse->intr_pipe);
	evdev_free(mouse->evdev);
	free(mouse);
}

static int
usb_init(struct usb_device *device,
         struct usb_interface *interface,
         void *userdata)
{
	struct usb_mouse *mouse = NULL;
	ssize_t ret;

	(void)userdata;
	mouse = malloc(sizeof(*mouse), M_ZERO);
	if (!mouse)
	{
		TRACE("usb_mouse: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	mouse->device = device;
	mouse->interface = interface;
	mouse->endpoint = TAILQ_FIRST(&interface->endpoints);
	if (!mouse->endpoint)
	{
		TRACE("usb_mouse: no endpoint");
		ret = -EINVAL;
		goto err;
	}
	ret = usb_ctrl_transfer(device,
	                        USB_REQ_DST_INTERFACE | USB_REQ_TYPE_CLASS | USB_REQ_HOST_TO_DEV,
	                        HID_SET_IDLE,
	                        0, interface->desc.interface, NULL, 0);
	if (ret < 0)
	{
		TRACE("usb_mouse: failed to set idle");
		goto err;
	}
	ret = evdev_alloc(&mouse->evdev);
	if (ret)
	{
		TRACE("usb_mouse: evdev allocation failed");
		goto err;
	}
	ret = usb_intr_pipe_alloc(device, mouse->endpoint, 4,
	                          intr_pipe_fn, mouse,
	                          &mouse->intr_pipe);
	if (ret)
	{
		TRACE("usb_mouse: interrupt pipe allocation failed");
		goto err;
	}
	ret = usb_intr_transfer(mouse->intr_pipe);
	if (ret)
	{
		TRACE("usb_mouse: interrupt pipe setup failed");
		goto err;
	}
	interface->userdata = mouse;
	return 0;

err:
	usb_mouse_free(mouse);
	return ret;
}

int
init(void)
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

void
fini(void)
{
}

struct kmod_info
kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "usb/mouse",
	.init = init,
	.fini = fini,
};
