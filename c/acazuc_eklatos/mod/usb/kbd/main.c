#define ENABLE_TRACE

#include "evdev.h"
#include "usb.h"

#include <kmod.h>
#include <std.h>
#include <mem.h>

static const struct usb_itf_probe probes[] =
{
	{USB_CLASS_HID, 1, 1},
};

struct usb_kbd
{
	struct usb_device *device;
	struct usb_interface *interface;
	struct usb_endpoint *endpoint;
	struct usb_intr_pipe *intr_pipe;
	struct evdev *evdev;
	uint8_t data[8];
};

static const enum kbd_key keys[256] =
{
	/* 0x00 */ KBD_KEY_NONE       , KBD_KEY_NONE,
	/* 0x02 */ KBD_KEY_NONE       , KBD_KEY_NONE,
	/* 0x04 */ KBD_KEY_A          , KBD_KEY_B,
	/* 0x06 */ KBD_KEY_C          , KBD_KEY_D,
	/* 0x08 */ KBD_KEY_E          , KBD_KEY_F,
	/* 0x0A */ KBD_KEY_G          , KBD_KEY_H,
	/* 0x0C */ KBD_KEY_I          , KBD_KEY_J,
	/* 0x0E */ KBD_KEY_K          , KBD_KEY_L,
	/* 0x10 */ KBD_KEY_M          , KBD_KEY_N,
	/* 0x12 */ KBD_KEY_O          , KBD_KEY_P,
	/* 0x14 */ KBD_KEY_Q          , KBD_KEY_R,
	/* 0x16 */ KBD_KEY_S          , KBD_KEY_T,
	/* 0x18 */ KBD_KEY_U          , KBD_KEY_V,
	/* 0x1A */ KBD_KEY_W          , KBD_KEY_X,
	/* 0x1C */ KBD_KEY_Y          , KBD_KEY_Z,
	/* 0x1E */ KBD_KEY_1          , KBD_KEY_2,
	/* 0x20 */ KBD_KEY_3          , KBD_KEY_4,
	/* 0x22 */ KBD_KEY_5          , KBD_KEY_6,
	/* 0x24 */ KBD_KEY_7          , KBD_KEY_8,
	/* 0x26 */ KBD_KEY_9          , KBD_KEY_0,
	/* 0x28 */ KBD_KEY_ENTER      , KBD_KEY_ESCAPE,
	/* 0x2A */ KBD_KEY_BACKSPACE  , KBD_KEY_TAB,
	/* 0x2C */ KBD_KEY_SPACE      , KBD_KEY_MINUS,
	/* 0x2E */ KBD_KEY_EQUAL      , KBD_KEY_LBRACKET,
	/* 0x30 */ KBD_KEY_RBRACKET   , KBD_KEY_ANTISLASH,
	/* 0x32 */ KBD_KEY_NONE       , KBD_KEY_SEMICOLON,
	/* 0x34 */ KBD_KEY_SQUOTE     , KBD_KEY_TILDE,
	/* 0x36 */ KBD_KEY_COMMA      , KBD_KEY_DOT,
	/* 0x38 */ KBD_KEY_SLASH      , KBD_KEY_CAPS_LOCK,
	/* 0x3A */ KBD_KEY_F1         , KBD_KEY_F2,
	/* 0x3C */ KBD_KEY_F3         , KBD_KEY_F4,
	/* 0x3E */ KBD_KEY_F5         , KBD_KEY_F6,
	/* 0x40 */ KBD_KEY_F7         , KBD_KEY_F8,
	/* 0x42 */ KBD_KEY_F9         , KBD_KEY_F10,
	/* 0x44 */ KBD_KEY_F11        , KBD_KEY_F12,
	/* 0x46 */ KBD_KEY_PRINT      , KBD_KEY_SCROLL_LOCK,
	/* 0x48 */ KBD_KEY_PAUSE      , KBD_KEY_INSERT,
	/* 0x4A */ KBD_KEY_HOME       , KBD_KEY_PGUP,
	/* 0x4C */ KBD_KEY_DELETE     , KBD_KEY_END,
	/* 0x4E */ KBD_KEY_PGDOWN     , KBD_KEY_CURSOR_RIGHT,
	/* 0x50 */ KBD_KEY_CURSOR_LEFT, KBD_KEY_CURSOR_DOWN,
	/* 0x52 */ KBD_KEY_CURSOR_UP  , KBD_KEY_NUM_LOCK,
	/* 0x54 */ KBD_KEY_KP_SLASH   , KBD_KEY_KP_MULT,
	/* 0x56 */ KBD_KEY_KP_MINUS   , KBD_KEY_KP_PLUS,
	/* 0x58 */ KBD_KEY_KP_ENTER   , KBD_KEY_KP_1,
	/* 0x5A */ KBD_KEY_KP_2       , KBD_KEY_KP_3,
	/* 0x5C */ KBD_KEY_KP_4       , KBD_KEY_KP_5,
	/* 0x5E */ KBD_KEY_KP_6       , KBD_KEY_KP_7,
	/* 0x60 */ KBD_KEY_KP_8       , KBD_KEY_KP_9,
	/* 0x62 */ KBD_KEY_KP_0       , KBD_KEY_KP_DOT,
	/* 0x64 */ KBD_KEY_NONE       , KBD_KEY_APPS,
	/* 0x66 */ KBD_KEY_ACPI_POWER , KBD_KEY_KP_EQUAL,
	/* 0x68 */ KBD_KEY_F13        , KBD_KEY_F14,
	/* 0x6A */ KBD_KEY_F15        , KBD_KEY_F16,
	/* 0x6C */ KBD_KEY_F17        , KBD_KEY_F18,
	/* 0x6E */ KBD_KEY_F19        , KBD_KEY_F20,
	/* 0x70 */ KBD_KEY_F21        , KBD_KEY_F22,
	/* 0x72 */ KBD_KEY_F23        , KBD_KEY_F24,
};

static void input_key(struct usb_kbd *kbd, enum kbd_key key, enum kbd_mod mods,
                      int pressed)
{
	ev_send_key_event(kbd->evdev, key, mods, pressed);
}

static void intr_pipe_fn(struct usb_intr_pipe *pipe, ssize_t ret)
{
	struct usb_kbd *kbd = pipe->userdata;
	uint8_t *data = pipe->buf->data;
	enum kbd_mod mods = 0;

	if (ret < 0)
	{
		TRACE("usb_kbd: interrupt err: %d", ret);
		return;
	}
	if (ret != 8)
	{
		TRACE("usb_kbd: interrupt pipe not complete");
		return;
	}
	if (data[0] & (1 << 0))
		mods |= KBD_MOD_LCONTROL;
	if (data[0] & (1 << 1))
		mods |= KBD_MOD_LSHIFT;
	if (data[0] & (1 << 2))
		mods |= KBD_MOD_LALT;
	if (data[0] & (1 << 3))
		mods |= KBD_MOD_LMETA;
	if (data[0] & (1 << 4))
		mods |= KBD_MOD_RCONTROL;
	if (data[0] & (1 << 5))
		mods |= KBD_MOD_RSHIFT;
	if (data[0] & (1 << 6))
		mods |= KBD_MOD_RALT;
	if (data[0] & (1 << 7))
		mods |= KBD_MOD_RMETA;
	if ((data[0] ^ kbd->data[0]) & (1 << 0))
		input_key(kbd, KBD_KEY_LCONTROL, mods, data[0] & (1 << 0));
	if ((data[0] ^ kbd->data[0]) & (1 << 1))
		input_key(kbd, KBD_KEY_LSHIFT, mods, data[0] & (1 << 1));
	if ((data[0] ^ kbd->data[0]) & (1 << 2))
		input_key(kbd, KBD_KEY_LALT, mods, data[0] & (1 << 2));
	if ((data[0] ^ kbd->data[0]) & (1 << 3))
		input_key(kbd, KBD_KEY_LMETA, mods, data[0] & (1 << 3));
	if ((data[0] ^ kbd->data[0]) & (1 << 4))
		input_key(kbd, KBD_KEY_RCONTROL, mods, data[0] & (1 << 4));
	if ((data[0] ^ kbd->data[0]) & (1 << 5))
		input_key(kbd, KBD_KEY_RSHIFT, mods, data[0] & (1 << 5));
	if ((data[0] ^ kbd->data[0]) & (1 << 6))
		input_key(kbd, KBD_KEY_RALT, mods, data[0] & (1 << 6));
	if ((data[0] ^ kbd->data[0]) & (1 << 7))
		input_key(kbd, KBD_KEY_RMETA, mods, data[0] & (1 << 7));
	if (data[2] != 1
	 && data[3] != 1
	 && data[4] != 1
	 && data[5] != 1
	 && data[6] != 1
	 && data[7] != 1)
	{
		/* that's ugly.. */
		for (size_t i = 2; i < 8 && data[i]; ++i)
		{
			int found = 0;
			for (size_t j = 2; j < 8 && kbd->data[j]; ++j)
			{
				if (data[i] == kbd->data[j])
				{
					found = 1;
					break;
				}
			}
			if (!found)
				input_key(kbd, keys[data[i]], mods, 1);
		}
		for (size_t i = 2; i < 8 && kbd->data[i]; ++i)
		{
			int found = 0;
			for (size_t j = 2; j < 8 && data[j]; ++j)
			{
				if (data[i] == kbd->data[j])
				{
					found = 1;
					break;
				}
			}
			if (!found)
				input_key(kbd, keys[kbd->data[i]], mods, 0);
		}
	}
	memcpy(kbd->data, data, 8);
}

static void usb_kbd_free(struct usb_kbd *kbd)
{
	if (!kbd)
		return;
	usb_intr_pipe_free(kbd->intr_pipe);
	evdev_free(kbd->evdev);
	free(kbd);
}

static int usb_init(struct usb_device *device, struct usb_interface *interface,
                    void *userdata)
{
	struct usb_kbd *kbd = NULL;
	ssize_t ret;

	(void)userdata;
	kbd = malloc(sizeof(*kbd), M_ZERO);
	if (!kbd)
	{
		TRACE("usb_kbd: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	kbd->device = device;
	kbd->interface = interface;
	kbd->endpoint = TAILQ_FIRST(&interface->endpoints);
	if (!kbd->endpoint)
	{
		TRACE("usb_kbd: no endpoint");
		ret = -EINVAL;
		goto err;
	}
	ret = usb_ctrl_transfer(device,
	                        USB_REQ_DST_INTERFACE | USB_REQ_TYPE_CLASS | USB_REQ_HOST_TO_DEV,
	                        HID_SET_IDLE,
	                        0, interface->desc.interface, NULL, 0);
	if (ret < 0)
	{
		TRACE("usb_kbd: failed to set idle");
		goto err;
	}
	ret = evdev_alloc(&kbd->evdev);
	if (ret)
	{
		TRACE("usb_kbd: evdev allocation failed");
		goto err;
	}
	ret = usb_intr_pipe_alloc(device, kbd->endpoint, 8,
	                          intr_pipe_fn, kbd,
	                          &kbd->intr_pipe);
	if (ret)
	{
		TRACE("usb_kbd: interrupt pipe allocation failed");
		goto err;
	}
	ret = usb_intr_transfer(kbd->intr_pipe);
	if (ret)
	{
		TRACE("usb_kbd: interrupt pipe setup failed");
		goto err;
	}
	interface->userdata = kbd;
	return 0;

err:
	usb_kbd_free(kbd);
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
		TRACE("usb_kbd: failed to register probes");
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
	.name = "usb/kbd",
	.init = init,
	.fini = fini,
};
