#ifndef GFX_EVENTS_H
#define GFX_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

enum gfx_key_code
{
	GFX_KEY_UNKNOWN,

	GFX_KEY_A,
	GFX_KEY_B,
	GFX_KEY_C,
	GFX_KEY_D,
	GFX_KEY_E,
	GFX_KEY_F,
	GFX_KEY_G,
	GFX_KEY_H,
	GFX_KEY_I,
	GFX_KEY_J,
	GFX_KEY_K,
	GFX_KEY_L,
	GFX_KEY_M,
	GFX_KEY_N,
	GFX_KEY_O,
	GFX_KEY_P,
	GFX_KEY_Q,
	GFX_KEY_R,
	GFX_KEY_S,
	GFX_KEY_T,
	GFX_KEY_U,
	GFX_KEY_V,
	GFX_KEY_W,
	GFX_KEY_X,
	GFX_KEY_Y,
	GFX_KEY_Z,

	GFX_KEY_0,
	GFX_KEY_1,
	GFX_KEY_2,
	GFX_KEY_3,
	GFX_KEY_4,
	GFX_KEY_5,
	GFX_KEY_6,
	GFX_KEY_7,
	GFX_KEY_8,
	GFX_KEY_9,

	GFX_KEY_KP_0,
	GFX_KEY_KP_1,
	GFX_KEY_KP_2,
	GFX_KEY_KP_3,
	GFX_KEY_KP_4,
	GFX_KEY_KP_5,
	GFX_KEY_KP_6,
	GFX_KEY_KP_7,
	GFX_KEY_KP_8,
	GFX_KEY_KP_9,
	GFX_KEY_KP_DIVIDE,
	GFX_KEY_KP_MULTIPLY,
	GFX_KEY_KP_SUBTRACT,
	GFX_KEY_KP_ADD,
	GFX_KEY_KP_EQUAL,
	GFX_KEY_KP_DECIMAL,
	GFX_KEY_KP_ENTER,

	GFX_KEY_F1,
	GFX_KEY_F2,
	GFX_KEY_F3,
	GFX_KEY_F4,
	GFX_KEY_F5,
	GFX_KEY_F6,
	GFX_KEY_F7,
	GFX_KEY_F8,
	GFX_KEY_F9,
	GFX_KEY_F10,
	GFX_KEY_F11,
	GFX_KEY_F12,
	GFX_KEY_F13,
	GFX_KEY_F14,
	GFX_KEY_F15,
	GFX_KEY_F16,
	GFX_KEY_F17,
	GFX_KEY_F18,
	GFX_KEY_F19,
	GFX_KEY_F20,
	GFX_KEY_F21,
	GFX_KEY_F22,
	GFX_KEY_F23,
	GFX_KEY_F24,

	GFX_KEY_LSHIFT,
	GFX_KEY_RSHIFT,
	GFX_KEY_LCONTROL,
	GFX_KEY_RCONTROL,
	GFX_KEY_LALT,
	GFX_KEY_RALT,
	GFX_KEY_LSUPER,
	GFX_KEY_RSUPER,

	GFX_KEY_LEFT,
	GFX_KEY_RIGHT,
	GFX_KEY_UP,
	GFX_KEY_DOWN,

	GFX_KEY_SPACE,
	GFX_KEY_BACKSPACE,
	GFX_KEY_ENTER,
	GFX_KEY_TAB,

	GFX_KEY_ESCAPE,
	GFX_KEY_PAUSE,
	GFX_KEY_DELETE,
	GFX_KEY_INSERT,
	GFX_KEY_HOME,
	GFX_KEY_PAGE_UP,
	GFX_KEY_PAGE_DOWN,
	GFX_KEY_END,

	GFX_KEY_COMMA,
	GFX_KEY_PERIOD,
	GFX_KEY_SLASH,
	GFX_KEY_APOSTROPHE,
	GFX_KEY_SEMICOLON,
	GFX_KEY_GRAVE,
	GFX_KEY_LBRACKET,
	GFX_KEY_RBRACKET,
	GFX_KEY_BACKSLASH,
	GFX_KEY_EQUAL,
	GFX_KEY_SUBTRACT,

	GFX_KEY_SCROLL_LOCK,
	GFX_KEY_NUM_LOCK,
	GFX_KEY_CAPS_LOCK,

	GFX_KEY_PRINT,

	GFX_KEY_LAST,
};

enum gfx_mouse_button
{
	GFX_MOUSE_BUTTON_LEFT,
	GFX_MOUSE_BUTTON_RIGHT,
	GFX_MOUSE_BUTTON_MIDDLE,
	GFX_MOUSE_BUTTON_4,
	GFX_MOUSE_BUTTON_5,
	GFX_MOUSE_BUTTON_6,
	GFX_MOUSE_BUTTON_7,
	GFX_MOUSE_BUTTON_8,
	GFX_MOUSE_BUTTON_LAST,
};

enum gfx_key_mods
{
	GFX_KEY_MOD_CONTROL = 0x1,
	GFX_KEY_MOD_SHIFT = 0x2,
	GFX_KEY_MOD_ALT = 0x4,
	GFX_KEY_MOD_SUPER = 0x8,
	GFX_KEY_MOD_CAPS_LOCK = 0x10,
	GFX_KEY_MOD_NUM_LOCK = 0x20,
};

struct gfx_key_event
{
	bool used;
	enum gfx_key_code key;
	uint32_t mods;
};

struct gfx_char_event
{
	bool used;
	uint32_t codepoint;
	char utf8[5];
};

struct gfx_mouse_event
{
	bool used;
	int32_t x;
	int32_t y;
	enum gfx_mouse_button button;
	uint32_t mods;
};

struct gfx_scroll_event
{
	bool used;
	int32_t mouse_x;
	int32_t mouse_y;
	int32_t x;
	int32_t y;
};

struct gfx_pointer_event
{
	bool used;
	int32_t x;
	int32_t y;
};

struct gfx_resize_event
{
	bool used;
	int32_t width;
	int32_t height;
};

struct gfx_move_event
{
	bool used;
	int32_t x;
	int32_t y;
};

#ifdef __cplusplus
}
#endif

#endif
