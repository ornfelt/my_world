#define ENABLE_TRACE

#include "evdev.h"
#include "ps2.h"

#include "arch/x86/x86.h"

#include <utf8.h>
#include <std.h>

/* qwerty scan code set 1 table */
static const enum kbd_key key_table[128] =
{
	/* 0x00 */ KBD_KEY_NONE       , KBD_KEY_ESCAPE,
	/* 0x02 */ KBD_KEY_1          , KBD_KEY_2,
	/* 0x04 */ KBD_KEY_3          , KBD_KEY_4,
	/* 0x06 */ KBD_KEY_5          , KBD_KEY_6,
	/* 0x08 */ KBD_KEY_7          , KBD_KEY_8,
	/* 0x0A */ KBD_KEY_9          , KBD_KEY_0,
	/* 0x0C */ KBD_KEY_MINUS      , KBD_KEY_EQUAL,
	/* 0x0E */ KBD_KEY_BACKSPACE  , KBD_KEY_TAB,
	/* 0x10 */ KBD_KEY_Q          , KBD_KEY_W,
	/* 0x12 */ KBD_KEY_E          , KBD_KEY_R,
	/* 0x14 */ KBD_KEY_T          , KBD_KEY_Y,
	/* 0x16 */ KBD_KEY_U          , KBD_KEY_I,
	/* 0x18 */ KBD_KEY_O          , KBD_KEY_P,
	/* 0x1A */ KBD_KEY_LBRACKET   , KBD_KEY_RBRACKET,
	/* 0x1C */ KBD_KEY_ENTER      , KBD_KEY_LCONTROL,
	/* 0x1E */ KBD_KEY_A          , KBD_KEY_S,
	/* 0x20 */ KBD_KEY_D          , KBD_KEY_F,
	/* 0x22 */ KBD_KEY_G          , KBD_KEY_H,
	/* 0x24 */ KBD_KEY_J          , KBD_KEY_K,
	/* 0x26 */ KBD_KEY_L          , KBD_KEY_SEMICOLON,
	/* 0x28 */ KBD_KEY_SQUOTE     , KBD_KEY_TILDE,
	/* 0x2A */ KBD_KEY_LSHIFT     , KBD_KEY_ANTISLASH,
	/* 0x2C */ KBD_KEY_Z          , KBD_KEY_X,
	/* 0x2E */ KBD_KEY_C          , KBD_KEY_V,
	/* 0x30 */ KBD_KEY_B          , KBD_KEY_N,
	/* 0x32 */ KBD_KEY_M          , KBD_KEY_COMMA,
	/* 0x34 */ KBD_KEY_DOT        , KBD_KEY_SLASH,
	/* 0x36 */ KBD_KEY_RSHIFT     , KBD_KEY_KP_MULT,
	/* 0x38 */ KBD_KEY_LALT       , KBD_KEY_SPACE,
	/* 0x3A */ KBD_KEY_CAPS_LOCK  , KBD_KEY_F1,
	/* 0x3C */ KBD_KEY_F2         , KBD_KEY_F3,
	/* 0x3E */ KBD_KEY_F4         , KBD_KEY_F5,
	/* 0x40 */ KBD_KEY_F6         , KBD_KEY_F7,
	/* 0x42 */ KBD_KEY_F8         , KBD_KEY_F9,
	/* 0x44 */ KBD_KEY_F10        , KBD_KEY_NUM_LOCK,
	/* 0x46 */ KBD_KEY_SCROLL_LOCK, KBD_KEY_KP_7,
	/* 0x48 */ KBD_KEY_KP_8       , KBD_KEY_KP_9,
	/* 0x4A */ KBD_KEY_KP_MINUS   , KBD_KEY_KP_4,
	/* 0x4C */ KBD_KEY_KP_5       , KBD_KEY_KP_6,
	/* 0x4E */ KBD_KEY_KP_PLUS    , KBD_KEY_KP_1,
	/* 0x50 */ KBD_KEY_KP_2       , KBD_KEY_KP_3,
	/* 0x52 */ KBD_KEY_KP_0       , KBD_KEY_KP_DOT,
	/* 0x54 */ KBD_KEY_NONE       , KBD_KEY_NONE,
	/* 0x56 */ KBD_KEY_LOWER      , KBD_KEY_F11,
	/* 0x58 */ KBD_KEY_F12,
};

/* extended qwerty scan code 1 table (0xE0 prefixed) */
static const enum kbd_key key_table_ext[128] =
{
	[0x10] = KBD_KEY_MM_PREV_TRACK,
	[0x19] = KBD_KEY_MM_NEXT_TRACK,
	[0x1C] = KBD_KEY_KP_ENTER,
	[0x1D] = KBD_KEY_RCONTROL,
	[0x20] = KBD_KEY_MM_MUTE,
	[0x21] = KBD_KEY_MM_CALC,
	[0x22] = KBD_KEY_MM_PLAY,
	[0x24] = KBD_KEY_MM_STOP,
	[0x2E] = KBD_KEY_MM_VOLUME_DOWN,
	[0x30] = KBD_KEY_MM_VOLUME_UP,
	[0x32] = KBD_KEY_MM_WWW,
	[0x35] = KBD_KEY_KP_SLASH,
	[0x38] = KBD_KEY_RALT,
	[0x47] = KBD_KEY_HOME,
	[0x48] = KBD_KEY_CURSOR_UP,
	[0x49] = KBD_KEY_PGUP,
	[0x4B] = KBD_KEY_CURSOR_LEFT,
	[0x4D] = KBD_KEY_CURSOR_RIGHT,
	[0x4F] = KBD_KEY_END,
	[0x50] = KBD_KEY_CURSOR_DOWN,
	[0x51] = KBD_KEY_PGDOWN,
	[0x52] = KBD_KEY_INSERT,
	[0x53] = KBD_KEY_DELETE,
	[0x5B] = KBD_KEY_LMETA,
	[0x5C] = KBD_KEY_RMETA,
	[0x5D] = KBD_KEY_APPS,
	[0x5E] = KBD_KEY_ACPI_POWER,
	[0x5F] = KBD_KEY_ACPI_SLEEP,
	[0x63] = KBD_KEY_ACPI_WAKE,
	[0x65] = KBD_KEY_MM_WWW_SEARCH,
	[0x66] = KBD_KEY_MM_WWW_FAVORITES,
	[0x67] = KBD_KEY_MM_WWW_REFRESH,
	[0x68] = KBD_KEY_MM_WWW_STOP,
	[0x69] = KBD_KEY_MM_WWW_FORWARD,
	[0x6A] = KBD_KEY_MM_WWW_BACK,
	[0x6B] = KBD_KEY_MM_WWW_COMPUTER,
	[0x6C] = KBD_KEY_MM_EMAIL,
	[0x6D] = KBD_KEY_MM_MEDIA_SELECT,
};

static uint8_t kbd_buffer[6];
static size_t kbd_buffer_size;
static enum kbd_mod kbd_mods;
static struct irq_handle kbd_irq_handle;
static struct evdev *kbd_evdev;

void ps2_kbd_input(uint8_t value)
{
#if 0
	TRACE("ps2 keyboard 0x%02x", value);
#endif

	if (value == 0xE0)
	{
		if (kbd_buffer_size != 0)
		{
			TRACE("ps2: invalid kbd escape code (too long)");
			return;
		}
		kbd_buffer[kbd_buffer_size++] = value;
		return;
	}
	int release = 0;
	if (value & 0x80)
		release = 1;
	enum kbd_key key = KBD_KEY_NONE;
	kbd_buffer[kbd_buffer_size++] = value;
	if (kbd_buffer_size == 1)
	{
		key = key_table[kbd_buffer[0] & 0x7F];
		kbd_buffer_size = 0;
	}
	else if (kbd_buffer_size == 2 && kbd_buffer[0] == 0xE0)
	{
		key = key_table_ext[kbd_buffer[1] & 0x7F];
		kbd_buffer_size = 0;
	}
	if (key == KBD_KEY_NONE)
		return;
	switch (key)
	{
		case KBD_KEY_LCONTROL:
			if (release)
				kbd_mods &= ~KBD_MOD_LCONTROL;
			else
				kbd_mods |= KBD_MOD_LCONTROL;
			break;
		case KBD_KEY_RCONTROL:
			if (release)
				kbd_mods &= ~KBD_MOD_RCONTROL;
			else
				kbd_mods |= KBD_MOD_RCONTROL;
			break;
		case KBD_KEY_LSHIFT:
			if (release)
				kbd_mods &= ~KBD_MOD_LSHIFT;
			else
				kbd_mods |= KBD_MOD_LSHIFT;
			break;
		case KBD_KEY_RSHIFT:
			if (release)
				kbd_mods &= ~KBD_MOD_RSHIFT;
			else
				kbd_mods |= KBD_MOD_RSHIFT;
			break;
		case KBD_KEY_LALT:
			if (release)
				kbd_mods &= ~KBD_MOD_LALT;
			else
				kbd_mods |= KBD_MOD_LALT;
			break;
		case KBD_KEY_RALT:
			if (release)
				kbd_mods &= ~KBD_MOD_RALT;
			else
				kbd_mods |= KBD_MOD_RALT;
			break;
		case KBD_KEY_LMETA:
			if (release)
				kbd_mods &= ~KBD_MOD_LMETA;
			else
				kbd_mods |= KBD_MOD_LMETA;
			break;
		case KBD_KEY_RMETA:
			if (release)
				kbd_mods &= ~KBD_MOD_RMETA;
			else
				kbd_mods |= KBD_MOD_RMETA;
			break;
		default:
			break;
	}
	ev_send_key_event(kbd_evdev, key, kbd_mods, !release);
}

void ps2_kbd_init(void)
{
	uint8_t tmp;

	if (ps2_wr(PS2_CMD, PS2_CMD_TEST_P1)
	 || ps2_rd(PS2_DATA, &tmp)
	 || tmp != 0x00)
		goto err;

	if (ps2_wr(PS2_CMD, PS2_CMD_READ_STATUS)
	 || ps2_rd(PS2_DATA, &tmp))
		goto err;

	if (ps2_wr(PS2_CMD, PS2_CMD_WRITE_STATUS)
	 || ps2_wr(PS2_DATA, tmp | 0x1))
		goto err;

	if (evdev_alloc(&kbd_evdev))
	{
		TRACE("ps2: failed to alloc kdb evdev");
		return;
	}

	if (register_isa_irq(ISA_IRQ_KBD, ps2_interrupt, NULL, &kbd_irq_handle))
	{
		TRACE("ps2: failed to enable kbd IRQ");
		return;
	}

	return;

err:
	TRACE("ps2: keyboard not found");
}
