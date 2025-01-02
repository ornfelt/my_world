#ifndef EVDEV_H
#define EVDEV_H

#include <types.h>

enum kbd_key
{
	KBD_KEY_NONE,
	KBD_KEY_ESCAPE,
	KBD_KEY_0,
	KBD_KEY_1,
	KBD_KEY_2,
	KBD_KEY_3,
	KBD_KEY_4,
	KBD_KEY_5,
	KBD_KEY_6,
	KBD_KEY_7,
	KBD_KEY_8,
	KBD_KEY_9,
	KBD_KEY_MINUS,
	KBD_KEY_EQUAL,
	KBD_KEY_BACKSPACE,
	KBD_KEY_TAB,
	KBD_KEY_A,
	KBD_KEY_B,
	KBD_KEY_C,
	KBD_KEY_D,
	KBD_KEY_E,
	KBD_KEY_F,
	KBD_KEY_G,
	KBD_KEY_H,
	KBD_KEY_I,
	KBD_KEY_J,
	KBD_KEY_K,
	KBD_KEY_L,
	KBD_KEY_M,
	KBD_KEY_N,
	KBD_KEY_O,
	KBD_KEY_P,
	KBD_KEY_Q,
	KBD_KEY_R,
	KBD_KEY_S,
	KBD_KEY_T,
	KBD_KEY_U,
	KBD_KEY_V,
	KBD_KEY_W,
	KBD_KEY_X,
	KBD_KEY_Y,
	KBD_KEY_Z,
	KBD_KEY_LBRACKET,
	KBD_KEY_RBRACKET,
	KBD_KEY_ENTER,
	KBD_KEY_SEMICOLON,
	KBD_KEY_SQUOTE,
	KBD_KEY_TILDE,
	KBD_KEY_LOWER,
	KBD_KEY_LCONTROL,
	KBD_KEY_RCONTROL,
	KBD_KEY_LSHIFT,
	KBD_KEY_RSHIFT,
	KBD_KEY_LALT,
	KBD_KEY_RALT,
	KBD_KEY_LMETA,
	KBD_KEY_RMETA,
	KBD_KEY_ANTISLASH,
	KBD_KEY_COMMA,
	KBD_KEY_DOT,
	KBD_KEY_SLASH,
	KBD_KEY_SPACE,
	KBD_KEY_CAPS_LOCK,
	KBD_KEY_F1,
	KBD_KEY_F2,
	KBD_KEY_F3,
	KBD_KEY_F4,
	KBD_KEY_F5,
	KBD_KEY_F6,
	KBD_KEY_F7,
	KBD_KEY_F8,
	KBD_KEY_F9,
	KBD_KEY_F10,
	KBD_KEY_F11,
	KBD_KEY_F12,
	KBD_KEY_F13,
	KBD_KEY_F14,
	KBD_KEY_F15,
	KBD_KEY_F16,
	KBD_KEY_F17,
	KBD_KEY_F18,
	KBD_KEY_F19,
	KBD_KEY_F20,
	KBD_KEY_F21,
	KBD_KEY_F22,
	KBD_KEY_F23,
	KBD_KEY_F24,
	KBD_KEY_NUM_LOCK,
	KBD_KEY_SCROLL_LOCK,
	KBD_KEY_HOME,
	KBD_KEY_PGUP,
	KBD_KEY_PGDOWN,
	KBD_KEY_END,
	KBD_KEY_INSERT,
	KBD_KEY_DELETE,
	KBD_KEY_APPS,
	KBD_KEY_PRINT,
	KBD_KEY_PAUSE,
	KBD_KEY_KP_0,
	KBD_KEY_KP_1,
	KBD_KEY_KP_2,
	KBD_KEY_KP_3,
	KBD_KEY_KP_4,
	KBD_KEY_KP_5,
	KBD_KEY_KP_6,
	KBD_KEY_KP_7,
	KBD_KEY_KP_8,
	KBD_KEY_KP_9,
	KBD_KEY_KP_MULT,
	KBD_KEY_KP_MINUS,
	KBD_KEY_KP_PLUS,
	KBD_KEY_KP_DOT,
	KBD_KEY_KP_ENTER,
	KBD_KEY_KP_SLASH,
	KBD_KEY_KP_EQUAL,
	KBD_KEY_CURSOR_UP,
	KBD_KEY_CURSOR_LEFT,
	KBD_KEY_CURSOR_RIGHT,
	KBD_KEY_CURSOR_DOWN,
	KBD_KEY_MM_PREV_TRACK,
	KBD_KEY_MM_NEXT_TRACK,
	KBD_KEY_MM_MUTE,
	KBD_KEY_MM_CALC,
	KBD_KEY_MM_PLAY,
	KBD_KEY_MM_STOP,
	KBD_KEY_MM_VOLUME_UP,
	KBD_KEY_MM_VOLUME_DOWN,
	KBD_KEY_MM_EMAIL,
	KBD_KEY_MM_MEDIA_SELECT,
	KBD_KEY_MM_WWW,
	KBD_KEY_MM_WWW_SEARCH,
	KBD_KEY_MM_WWW_FAVORITES,
	KBD_KEY_MM_WWW_REFRESH,
	KBD_KEY_MM_WWW_STOP,
	KBD_KEY_MM_WWW_FORWARD,
	KBD_KEY_MM_WWW_BACK,
	KBD_KEY_MM_WWW_COMPUTER,
	KBD_KEY_ACPI_POWER,
	KBD_KEY_ACPI_SLEEP,
	KBD_KEY_ACPI_WAKE,
	KBD_KEY_LAST,
};

enum kbd_mod
{
	KBD_MOD_LSHIFT   = (1 << 0),
	KBD_MOD_RSHIFT   = (1 << 1),
	KBD_MOD_LCONTROL = (1 << 2),
	KBD_MOD_RCONTROL = (1 << 3),
	KBD_MOD_LALT     = (1 << 4),
	KBD_MOD_RALT     = (1 << 5),
	KBD_MOD_LMETA    = (1 << 6),
	KBD_MOD_RMETA    = (1 << 7),
};

enum mouse_button
{
	MOUSE_BUTTON_LEFT   = 1,
	MOUSE_BUTTON_RIGHT  = 2,
	MOUSE_BUTTON_MIDDLE = 3,
	MOUSE_BUTTON_4      = 4,
	MOUSE_BUTTON_5      = 5,
};

enum event_type
{
	EVENT_KEY,
	EVENT_MOUSE,
	EVENT_POINTER,
	EVENT_SCROLL,
};

struct key_event
{
	uint32_t key;
	uint32_t mod;
	uint32_t pressed;
	uint32_t pad0;
};

struct mouse_event
{
	uint32_t button;
	uint32_t pressed;
	uint32_t pad0;
	uint32_t pad1;
};

struct pointer_event
{
	int32_t x;
	int32_t y;
	uint32_t pad0;
	uint32_t pad1;
};

struct scroll_event
{
	int32_t x;
	int32_t y;
	uint32_t pad0;
	uint32_t pad1;
};

struct event
{
	uint32_t type;
	union
	{
		struct key_event key;
		struct mouse_event mouse;
		struct pointer_event pointer;
		struct scroll_event scroll;
	};
};

struct evdev;

int evdev_alloc(struct evdev **evdev);
void evdev_free(struct evdev *evdev);
void evdev_ref(struct evdev *evdev);

void ev_send_key_event(struct evdev *evdev, enum kbd_key key,
                       enum kbd_mod mods, int pressed);
void ev_send_mouse_event(struct evdev *evdev, enum mouse_button button,
                         int pressed);
void ev_send_pointer_event(struct evdev *evdev, int x, int y);
void ev_send_scroll_event(struct evdev *evdev, int x, int y);

#endif
