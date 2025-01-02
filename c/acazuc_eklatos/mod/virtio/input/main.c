#define ENABLE_TRACE

#include "virtio.h"
#include "evdev.h"
#include "pci.h"

#include <errno.h>
#include <kmod.h>
#include <std.h>
#include <sg.h>

#define VIRTIO_INPUT_CFG_UNSET     0x00
#define VIRTIO_INPUT_CFG_ID_NAME   0x01
#define VIRTIO_INPUT_CFG_ID_SERIAL 0x02
#define VIRTIO_INPUT_CFG_ID_DEVIDS 0x03
#define VIRTIO_INPUT_CFG_PROP_BITS 0x10
#define VIRTIO_INPUT_CFG_EV_BITS   0x11
#define VIRTIO_INPUT_CFG_ABS_INFO  0x12

#define VIRTIO_INPUT_C_SELECT 0x1
#define VIRTIO_INPUT_C_SUBSEL 0x2
#define VIRTIO_INPUT_C_SIZE   0x3
#define VIRTIO_INPUT_C_STRING 0x8
#define VIRTIO_INPUT_C_BITMAP 0x8

#define VIRTIO_INPUT_C_ABSINFO_MIN  0x08
#define VIRTIO_INPUT_C_ASBINFO_MAX  0x0C
#define VIRTIO_INPUT_C_ABSINFO_FUZZ 0x10
#define VIRTIO_INPUT_C_ABSINFO_FLAT 0x14
#define VIRTIO_INPUT_C_ABSINFO_RES  0x18

#define VIRTIO_INPUT_C_DEVIDS_BUSTYPE 0x8
#define VIRTIO_INPUT_C_DEVIDS_VENDOR  0xA
#define VIRTIO_INPUT_C_DEVIDS_PRODUCT 0xC
#define VIRTIO_INPUT_C_DEVIDS_VERSION 0xE

#define EVENT_SYNC    0x0
#define EVENT_KEY     0x1
#define EVENT_POINTER 0x2

#define MOUSE_BUTTON_FIRST 0x110
#define MOUSE_BUTTON_LAST  0x11F

#define WHEEL_BUTTON 0x150

#define POINTER_X     0x0
#define POINTER_Y     0x1
#define POINTER_WHEEL 0x8

static const uint16_t keycodes[] =
{
	/* 0x00 */ KBD_KEY_NONE        , KBD_KEY_ESCAPE,
	/* 0x02 */ KBD_KEY_1           , KBD_KEY_2,
	/* 0x04 */ KBD_KEY_3           , KBD_KEY_4,
	/* 0x06 */ KBD_KEY_5           , KBD_KEY_6,
	/* 0x08 */ KBD_KEY_7           , KBD_KEY_8,
	/* 0x0A */ KBD_KEY_9           , KBD_KEY_0,
	/* 0x0C */ KBD_KEY_MINUS       , KBD_KEY_EQUAL,
	/* 0x0E */ KBD_KEY_BACKSPACE   , KBD_KEY_TAB,
	/* 0x10 */ KBD_KEY_Q           , KBD_KEY_W,
	/* 0x12 */ KBD_KEY_E           , KBD_KEY_R,
	/* 0x14 */ KBD_KEY_T           , KBD_KEY_Y,
	/* 0x16 */ KBD_KEY_U           , KBD_KEY_I,
	/* 0x18 */ KBD_KEY_O           , KBD_KEY_P,
	/* 0x1A */ KBD_KEY_LBRACKET    , KBD_KEY_RBRACKET,
	/* 0x1C */ KBD_KEY_ENTER       , KBD_KEY_LCONTROL,
	/* 0x1E */ KBD_KEY_A           , KBD_KEY_S,
	/* 0x20 */ KBD_KEY_D           , KBD_KEY_F,
	/* 0x22 */ KBD_KEY_G           , KBD_KEY_H,
	/* 0x24 */ KBD_KEY_J           , KBD_KEY_K,
	/* 0x26 */ KBD_KEY_L           , KBD_KEY_SEMICOLON,
	/* 0x28 */ KBD_KEY_SQUOTE      , KBD_KEY_TILDE,
	/* 0x2A */ KBD_KEY_LSHIFT      , KBD_KEY_ANTISLASH,
	/* 0x2C */ KBD_KEY_Z           , KBD_KEY_X,
	/* 0x2E */ KBD_KEY_C           , KBD_KEY_V,
	/* 0x30 */ KBD_KEY_B           , KBD_KEY_N,
	/* 0x32 */ KBD_KEY_M           , KBD_KEY_COMMA,
	/* 0x34 */ KBD_KEY_DOT         , KBD_KEY_SLASH,
	/* 0x36 */ KBD_KEY_RSHIFT      , KBD_KEY_KP_MULT,
	/* 0x38 */ KBD_KEY_LALT        , KBD_KEY_SPACE,
	/* 0x3A */ KBD_KEY_CAPS_LOCK   , KBD_KEY_F1,
	/* 0x3C */ KBD_KEY_F2          , KBD_KEY_F3,
	/* 0x3E */ KBD_KEY_F4          , KBD_KEY_F5,
	/* 0x40 */ KBD_KEY_F6          , KBD_KEY_F7,
	/* 0x42 */ KBD_KEY_F8          , KBD_KEY_F9,
	/* 0x44 */ KBD_KEY_F10         , KBD_KEY_NUM_LOCK,
	/* 0x46 */ KBD_KEY_SCROLL_LOCK , KBD_KEY_KP_7,
	/* 0x48 */ KBD_KEY_KP_8        , KBD_KEY_KP_9,
	/* 0x4A */ KBD_KEY_KP_MINUS    , KBD_KEY_KP_4,
	/* 0x4C */ KBD_KEY_KP_5        , KBD_KEY_KP_6,
	/* 0x4E */ KBD_KEY_KP_PLUS     , KBD_KEY_KP_1,
	/* 0x50 */ KBD_KEY_KP_2        , KBD_KEY_KP_3,
	/* 0x52 */ KBD_KEY_KP_0        , KBD_KEY_KP_DOT,
	/* 0x54 */ KBD_KEY_NONE        , KBD_KEY_NONE,
	/* 0x56 */ KBD_KEY_LOWER       , KBD_KEY_F11,
	/* 0x58 */ KBD_KEY_F12         , KBD_KEY_NONE,
	/* 0x5A */ KBD_KEY_NONE        , KBD_KEY_NONE,
	/* 0x5C */ KBD_KEY_NONE        , KBD_KEY_NONE,
	/* 0x5E */ KBD_KEY_NONE        , KBD_KEY_NONE,
	/* 0x60 */ KBD_KEY_KP_ENTER    , KBD_KEY_RCONTROL,
	/* 0x62 */ KBD_KEY_KP_SLASH    , KBD_KEY_NONE,
	/* 0x64 */ KBD_KEY_RALT        , KBD_KEY_NONE,
	/* 0x66 */ KBD_KEY_HOME        , KBD_KEY_CURSOR_UP,
	/* 0x68 */ KBD_KEY_PGUP        , KBD_KEY_CURSOR_LEFT,
	/* 0x6A */ KBD_KEY_CURSOR_RIGHT, KBD_KEY_END,
	/* 0x6C */ KBD_KEY_CURSOR_DOWN , KBD_KEY_PGDOWN,
	/* 0x6E */ KBD_KEY_INSERT      , KBD_KEY_DELETE,
	/* 0x70 */ KBD_KEY_NONE        , KBD_KEY_MM_MUTE,
	/* 0x72 */ KBD_KEY_MM_VOLUME_UP, KBD_KEY_MM_VOLUME_DOWN,
	/* 0x74 */ KBD_KEY_ACPI_POWER  , KBD_KEY_NONE,
	/* 0x76 */ KBD_KEY_NONE        , KBD_KEY_NONE,
	/* 0x78 */ KBD_KEY_KP_DOT      , KBD_KEY_NONE,
	/* 0x7A */ KBD_KEY_NONE        , KBD_KEY_NONE,
	/* 0x7C */ KBD_KEY_NONE        , KBD_KEY_LMETA,
	/* 0x7E */ KBD_KEY_RMETA       , KBD_KEY_NONE,
};

struct virtio_input_event
{
	uint16_t type;
	uint16_t code;
	uint32_t value;
};

struct virtio_input
{
	struct virtio_dev dev;
	struct pci_map *input_cfg;
	struct dma_buf *events;
	uint32_t kbd_mods;
	uint32_t mouse_state;
	struct evdev *evdev;
};

static int add_rx_buf(struct virtio_input *input, uint16_t id)
{
	struct sg_head sg;
	int ret;

	sg_init(&sg);
	ret = sg_add_dma_buf(&sg, input->events,
	                     sizeof(struct virtio_input_event),
	                     sizeof(struct virtio_input_event) * id);
	if (ret)
		goto end;
	ret = virtq_send(&input->dev.queues[0], &sg, 0, 1);

end:
	sg_free(&sg);
	return ret;
}

static void key_event(struct virtio_input *input,
                      struct virtio_input_event *event)
{
	if (event->code >= MOUSE_BUTTON_FIRST
	 && event->code <= MOUSE_BUTTON_LAST)
	{
		uint32_t button = event->code - MOUSE_BUTTON_FIRST;
		/* double-clicks are sent weirdly, patch them using
		 * the current mouse state
		 */
		if (event->value)
		{
			if ((input->mouse_state & (1 << button)))
				ev_send_mouse_event(input->evdev, event->code, 0);
			else
				input->mouse_state |= (1 << button);
		}
		else
		{
			if (!(input->mouse_state & (1 << button)))
				return;
			input->mouse_state &= ~(1 << button);
		}
		ev_send_mouse_event(input->evdev, event->code, !!event->value);
		return;
	}
	if (event->code > sizeof(keycodes) / sizeof(*keycodes)
	 || !keycodes[event->code])
		return;
	switch (keycodes[event->code])
	{
		case KBD_KEY_LCONTROL:
			if (event->value)
				input->kbd_mods |= KBD_MOD_LCONTROL;
			else
				input->kbd_mods &= ~KBD_MOD_LCONTROL;
			break;
		case KBD_KEY_RCONTROL:
			if (event->value)
				input->kbd_mods |= KBD_MOD_RCONTROL;
			else
				input->kbd_mods &= ~KBD_MOD_RCONTROL;
			break;
		case KBD_KEY_LSHIFT:
			if (event->value)
				input->kbd_mods |= KBD_MOD_LSHIFT;
			else
				input->kbd_mods &= ~KBD_MOD_LSHIFT;
			break;
		case KBD_KEY_RSHIFT:
			if (event->value)
				input->kbd_mods |= KBD_MOD_RSHIFT;
			else
				input->kbd_mods &= ~KBD_MOD_RSHIFT;
			break;
		case KBD_KEY_LALT:
			if (event->value)
				input->kbd_mods |= KBD_MOD_LALT;
			else
				input->kbd_mods &= ~KBD_MOD_LALT;
			break;
		case KBD_KEY_RALT:
			if (event->value)
				input->kbd_mods |= KBD_MOD_RALT;
			else
				input->kbd_mods &= ~KBD_MOD_RALT;
			break;
		case KBD_KEY_LMETA:
			if (event->value)
				input->kbd_mods |= KBD_MOD_LMETA;
			else
				input->kbd_mods &= ~KBD_MOD_LMETA;
			break;
		case KBD_KEY_RMETA:
			if (event->value)
				input->kbd_mods |= KBD_MOD_RMETA;
			else
				input->kbd_mods &= ~KBD_MOD_RMETA;
			break;
		default:
			break;
	}
	ev_send_key_event(input->evdev, keycodes[event->code],
	                  input->kbd_mods, !!event->value);
}

static void pointer_event(struct virtio_input *input,
                          struct virtio_input_event *event)
{
	switch (event->code)
	{
		case POINTER_X:
			ev_send_pointer_event(input->evdev,
			                      (int32_t)event->value, 0);
			break;
		case POINTER_Y:
			ev_send_pointer_event(input->evdev, 0,
			                      (int32_t)event->value);
			break;
		case POINTER_WHEEL:
			ev_send_scroll_event(input->evdev, 0,
			                     (int32_t)event->value);
			break;
	}
}

static void on_eventq_msg(struct virtq *queue, uint16_t id, uint32_t len)
{
	struct virtio_input *input = (struct virtio_input*)queue->dev;
	struct virtio_input_event *event = &((struct virtio_input_event*)input->events->data)[id];
	if (len >= sizeof(*event))
	{
#if 0
		TRACE("type: %" PRIu16 ", code: %" PRIu16 ", value: %" PRIu32,
		      event->type, event->code, event->value);
#endif
		switch (event->type)
		{
			case EVENT_SYNC:
				/* XXX use to merge mouse pointer events */
				break;
			case EVENT_KEY:
				key_event(input, event);
				break;
			case EVENT_POINTER:
				pointer_event(input, event);
				break;
		}
	}
	else
	{
		TRACE("virtio_input: invalid event length: %" PRIu32 " / %u",
		      len, (unsigned)sizeof(*event));
	}
	int ret = add_rx_buf(input, id);
	if (ret)
		TRACE("virtio_input: failed to add rx buf");
}

static void virtio_input_delete(struct virtio_input *input)
{
	if (!input)
		return;
	dma_buf_free(input->events);
	evdev_free(input->evdev);
	virtio_dev_destroy(&input->dev);
	free(input);
}

int init_pci(struct pci_device *device, void *userdata)
{
	struct virtio_input *input;
	uint8_t features[1];
	int ret;

	(void)userdata;
	input = malloc(sizeof(*input), M_ZERO);
	if (!input)
	{
		TRACE("virtio_input: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = evdev_alloc(&input->evdev);
	if (ret)
	{
		TRACE("virtio_input: failed to allocate evdev");
		goto err;
	}
	ret = virtio_dev_init(&input->dev, device, features, 0);
	if (ret)
	{
		TRACE("virtio_input: failed to initialize device");
		goto err;
	}
	if (input->dev.queues_nb < 2)
	{
		TRACE("virtio_input: no queues");
		ret = -EINVAL;
		goto err;
	}
	ret = dma_buf_alloc(PAGE_SIZE, 0, &input->events);
	if (ret)
	{
		TRACE("virtio_input: events allocation failed");
		goto err;
	}
	for (size_t i = 0; i < input->dev.queues[0].size; ++i)
	{
		ret = add_rx_buf(input, i);
		if (ret)
		{
			TRACE("virtio_input: failed to set rx buf");
			goto err;
		}
	}
	input->dev.queues[0].on_msg = on_eventq_msg;
	ret = virtq_setup_irq(&input->dev.queues[0]);
	if (ret)
	{
		TRACE("virtio_input: failed to setup irq");
		goto err;
	}
	virtio_dev_init_end(&input->dev);
	return 0;

err:
	virtio_input_delete(input);
	return ret;
}

static int init(void)
{
	pci_probe(0x1AF4, 0x1052, init_pci, NULL);
	return 0;
}

static void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "virtio/input",
	.init = init,
	.fini = fini,
};
