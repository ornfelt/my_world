#include "windows/glfw.h"

#include "window_vtable.h"
#include "config.h"

#include <jks/utf8.h>
#include <stdlib.h>
#include <string.h>

static const int cursors[GFX_CURSOR_LAST] =
{
	[GFX_CURSOR_ARROW] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_CROSS] = GLFW_CROSSHAIR_CURSOR,
	[GFX_CURSOR_HAND] = GLFW_HAND_CURSOR,
	[GFX_CURSOR_IBEAM] = GLFW_IBEAM_CURSOR,
	[GFX_CURSOR_NO] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_SIZEALL] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_VRESIZE] = GLFW_VRESIZE_CURSOR,
	[GFX_CURSOR_HRESIZE] = GLFW_HRESIZE_CURSOR,
	[GFX_CURSOR_WAIT] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_BLANK] = 0,
};

static void on_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void on_character_callback(GLFWwindow *window, unsigned int codepoint);
static void on_cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
static void on_cursor_enter_callback(GLFWwindow *window, int entered);
static void on_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
static void on_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
static void on_window_focus_callback(GLFWwindow *window, int focused);
static void on_framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void on_window_pos_callback(GLFWwindow *window, int xpos, int ypos);
static void on_window_close_callback(GLFWwindow *window);
static void on_window_refresh_callback(GLFWwindow *window);
static enum gfx_key_code get_key_code(int key_code);
static enum gfx_mouse_button get_button(int button);
static uint32_t get_mods(int state);

void *gfx_glfw_get_proc_address(const char *name)
{
	return (void*)glfwGetProcAddress(name);
}

bool gfx_glfw_create_window(struct gfx_glfw_window *window, const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_DEPTH_BITS, properties->depth_bits);
	glfwWindowHint(GLFW_STENCIL_BITS, properties->stencil_bits);
	glfwWindowHint(GLFW_RED_BITS, properties->red_bits);
	glfwWindowHint(GLFW_GREEN_BITS, properties->green_bits);
	glfwWindowHint(GLFW_BLUE_BITS, properties->blue_bits);
	glfwWindowHint(GLFW_ALPHA_BITS, properties->alpha_bits);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	switch (properties->device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GL3
		case GFX_DEVICE_GL3:
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		case GFX_DEVICE_GL4:
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_GLES3
		case GFX_DEVICE_GLES3:
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_VK
		case GFX_DEVICE_VK:
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			break;
#endif
		default:
			GFX_ERROR_CALLBACK("unknown glfw backend");
			return false;
	}
	window->window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window->window)
	{
		GFX_ERROR_CALLBACK("failed to create glfw window");
		return false;
	}
	glfwSetWindowUserPointer(window->window, window);
	glfwSetKeyCallback(window->window, on_key_callback);
	glfwSetCharCallback(window->window, on_character_callback);
	glfwSetCursorPosCallback(window->window, on_cursor_position_callback);
	glfwSetCursorEnterCallback(window->window, on_cursor_enter_callback);
	glfwSetMouseButtonCallback(window->window, on_mouse_button_callback);
	glfwSetScrollCallback(window->window, on_scroll_callback);
	glfwSetWindowFocusCallback(window->window, on_window_focus_callback);
	glfwSetFramebufferSizeCallback(window->window, on_framebuffer_size_callback);
	glfwSetWindowPosCallback(window->window, on_window_pos_callback);
	glfwSetWindowCloseCallback(window->window, on_window_close_callback);
	glfwSetWindowRefreshCallback(window->window, on_window_refresh_callback);
	return true;
}

bool gfx_glfw_ctr(struct gfx_glfw_window *window, struct gfx_window *winref)
{
	if (!glfwInit())
	{
		GFX_ERROR_CALLBACK("failed to initialize glfw");
		return false;
	}
	window->winref = winref;
	window->window = NULL;
	return true;
}

void gfx_glfw_dtr(struct gfx_glfw_window *window)
{
	glfwDestroyWindow(window->window);
}

void gfx_glfw_show(struct gfx_glfw_window *window)
{
	glfwShowWindow(window->window);
}

void gfx_glfw_hide(struct gfx_glfw_window *window)
{
	glfwHideWindow(window->window);
}

void gfx_glfw_poll_events(struct gfx_glfw_window *window)
{
	(void)window;
	glfwPollEvents();
}

void gfx_glfw_wait_events(struct gfx_glfw_window *window)
{
	(void)window;
	glfwWaitEvents();
}

void gfx_glfw_set_title(struct gfx_glfw_window *window, const char *title)
{
	glfwSetWindowTitle(window->window, title);
}

void gfx_glfw_set_icon(struct gfx_glfw_window *window, const void *data, uint32_t width, uint32_t height)
{
	GLFWimage image;
	image.width = width;
	image.height = height;
	image.pixels = (unsigned char*)data;
	glfwSetWindowIcon(window->window, 1, &image);
}

void gfx_glfw_resize(struct gfx_glfw_window *window, uint32_t width, uint32_t height)
{
	glfwSetWindowSize(window->window, width, height);
}

void gfx_glfw_grab_cursor(struct gfx_glfw_window *window)
{
	glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	window->winref->grabbed = true;
}

void gfx_glfw_ungrab_cursor(struct gfx_glfw_window *window)
{
	glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	window->winref->grabbed = false;
}

char *gfx_glfw_get_clipboard(struct gfx_glfw_window *window)
{
	return strdup(glfwGetClipboardString(window->window));
}

void gfx_glfw_set_clipboard(struct gfx_glfw_window *window, const char *text)
{
	glfwSetClipboardString(window->window, text);
}

gfx_cursor_t gfx_glfw_create_native_cursor(struct gfx_glfw_window *window, enum gfx_native_cursor cursor)
{
	(void)window;
	return glfwCreateStandardCursor(cursors[cursor]);
}

gfx_cursor_t gfx_glfw_create_cursor(struct gfx_glfw_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	(void)window;
	GLFWimage image;
	image.width = width;
	image.height = height;
	image.pixels = (unsigned char*)data;
	return glfwCreateCursor(&image, xhot, yhot);
}

void gfx_glfw_delete_cursor(struct gfx_glfw_window *window, gfx_cursor_t cursor)
{
	(void)window;
	if (!cursor)
		return;
	glfwDestroyCursor((GLFWcursor*)cursor);
}

void gfx_glfw_set_cursor(struct gfx_glfw_window *window, gfx_cursor_t cursor)
{
	glfwSetCursor(window->window, cursor);
}

void gfx_glfw_set_mouse_position(struct gfx_glfw_window *window, int32_t x, int32_t y)
{
	glfwSetCursorPos(window->window, x, y);
}

static void on_key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	(void)scancode;
	enum gfx_key_code key_code = get_key_code(key);
	switch (action)
	{
		case GLFW_PRESS:
			if (key_code < sizeof(window->winref->keys) * 8)
				window->winref->keys[key_code / 8] |= 1 << (key_code % 8);
			if (window->winref->key_down_callback)
			{
				struct gfx_key_event event;
				event.used = false;
				event.key = key_code;
				event.mods = get_mods(mods);
				window->winref->key_down_callback(window->winref, &event);
			}
			break;
		case GLFW_RELEASE:
			if (key_code < sizeof(window->winref->keys) * 8)
				window->winref->keys[key_code / 8] &= ~(1 << (key_code % 8));
			if (window->winref->key_up_callback)
			{
				struct gfx_key_event event;
				event.used = false;
				event.key = key_code;
				event.mods = get_mods(mods);
				window->winref->key_up_callback(window->winref, &event);
			}
			break;
		case GLFW_REPEAT:
			if (window->winref->key_press_callback)
			{
				struct gfx_key_event event;
				event.used = false;
				event.key = key_code;
				event.mods = get_mods(mods);
				window->winref->key_press_callback(window->winref, &event);
			}
			break;
	}
}

static void on_character_callback(GLFWwindow *win, unsigned int codepoint)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	if (window->winref->char_callback)
	{
		struct gfx_char_event event;
		event.used = false;
		event.codepoint = codepoint;
		memset(event.utf8, 0, sizeof(event.utf8));
		char *pos = &event.utf8[0];
		if (utf8_encode(&pos, codepoint))
		{
			window->winref->char_callback(window->winref, &event);
		}
		else
		{
			GFX_ERROR_CALLBACK("invalid input codepoint");
		}
	}
}

static void on_cursor_position_callback(GLFWwindow *win, double xpos, double ypos)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	if (window->winref->grabbed)
	{
		window->winref->virtual_x = xpos;
		window->winref->virtual_y = ypos;
	}
	else
	{
		window->winref->mouse_x = xpos;
		window->winref->mouse_y = ypos;
	}
	if (window->winref->mouse_move_callback)
	{
		struct gfx_pointer_event event;
		event.used = false;
		event.x = xpos;
		event.y = ypos;
		window->winref->mouse_move_callback(window->winref, &event);
	}
}

static void on_cursor_enter_callback(GLFWwindow *win, int entered)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	if (entered)
	{
		if (window->winref->cursor_enter_callback)
			window->winref->cursor_enter_callback(window->winref);
	}
	else
	{
		if (window->winref->cursor_leave_callback)
			window->winref->cursor_leave_callback(window->winref);
	}
}

static void on_mouse_button_callback(GLFWwindow *win, int button, int action, int mods)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	switch (action)
	{
		case GLFW_PRESS:
			if (button >= 0 && button < GFX_MOUSE_BUTTON_LAST)
				window->winref->mouse_buttons |= 1 << button;
			if (window->winref->mouse_down_callback)
			{
				struct gfx_mouse_event event;
				event.used = false;
				event.x = gfx_get_mouse_x(window->winref);
				event.y = gfx_get_mouse_y(window->winref);
				event.button = get_button(button);
				event.mods = get_mods(mods);
				window->winref->mouse_down_callback(window->winref, &event);
			}
			break;
		case GLFW_RELEASE:
			if (button >= 0 && button < GFX_MOUSE_BUTTON_LAST)
				window->winref->mouse_buttons &= ~(1 << button);
			if (window->winref->mouse_up_callback)
			{
				struct gfx_mouse_event event;
				event.used = false;
				event.x = gfx_get_mouse_x(window->winref);
				event.y = gfx_get_mouse_y(window->winref);
				event.button = get_button(button);
				event.mods = get_mods(mods);
				window->winref->mouse_up_callback(window->winref, &event);
			}
			break;
	}
}

static void on_scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	if (window->winref->scroll_callback)
	{
		struct gfx_scroll_event event;
		event.used = false;
		event.mouse_x = gfx_get_mouse_x(window->winref);
		event.mouse_y = gfx_get_mouse_y(window->winref);
		event.x = xoffset;
		event.y = yoffset;
		window->winref->scroll_callback(window->winref, &event);
	}
}

static void on_window_focus_callback(GLFWwindow *win, int focused)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	if (focused)
	{
		if (window->winref->focus_in_callback)
			window->winref->focus_in_callback(window->winref);
	}
	else
	{
		if (window->winref->focus_out_callback)
			window->winref->focus_out_callback(window->winref);
	}
}

static void on_framebuffer_size_callback(GLFWwindow *win, int width, int height)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	window->winref->width = width;
	window->winref->height = height;
	if (window->winref->resize_callback)
	{
		struct gfx_resize_event event;
		event.used = false;
		event.width = width;
		event.height = height;
		window->winref->resize_callback(window->winref, &event);
	}
}

static void on_window_pos_callback(GLFWwindow *win, int xpos, int ypos)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	window->winref->x = xpos;
	window->winref->y = ypos;
	if (window->winref->move_callback)
	{
		struct gfx_move_event event;
		event.used = false;
		event.x = xpos;
		event.y = ypos;
		window->winref->move_callback(window->winref, &event);
	}
}

static void on_window_close_callback(GLFWwindow *win)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	window->winref->close_requested = true;
	if (window->winref->close_callback)
		window->winref->close_callback(window->winref);
}

static void on_window_refresh_callback(GLFWwindow *win)
{
	struct gfx_glfw_window *window = glfwGetWindowUserPointer(win);
	if (window->winref->expose_callback)
		window->winref->expose_callback(window->winref);
}

static enum gfx_key_code get_key_code(int key_code)
{
	switch (key_code)
	{
		case GLFW_KEY_A:             return GFX_KEY_A;
		case GLFW_KEY_B:             return GFX_KEY_B;
		case GLFW_KEY_C:             return GFX_KEY_C;
		case GLFW_KEY_D:             return GFX_KEY_D;
		case GLFW_KEY_E:             return GFX_KEY_E;
		case GLFW_KEY_F:             return GFX_KEY_F;
		case GLFW_KEY_G:             return GFX_KEY_G;
		case GLFW_KEY_H:             return GFX_KEY_H;
		case GLFW_KEY_I:             return GFX_KEY_I;
		case GLFW_KEY_J:             return GFX_KEY_J;
		case GLFW_KEY_K:             return GFX_KEY_K;
		case GLFW_KEY_L:             return GFX_KEY_L;
		case GLFW_KEY_M:             return GFX_KEY_M;
		case GLFW_KEY_N:             return GFX_KEY_N;
		case GLFW_KEY_O:             return GFX_KEY_O;
		case GLFW_KEY_P:             return GFX_KEY_P;
		case GLFW_KEY_Q:             return GFX_KEY_Q;
		case GLFW_KEY_R:             return GFX_KEY_R;
		case GLFW_KEY_S:             return GFX_KEY_S;
		case GLFW_KEY_T:             return GFX_KEY_T;
		case GLFW_KEY_U:             return GFX_KEY_U;
		case GLFW_KEY_V:             return GFX_KEY_V;
		case GLFW_KEY_W:             return GFX_KEY_W;
		case GLFW_KEY_X:             return GFX_KEY_X;
		case GLFW_KEY_Y:             return GFX_KEY_Y;
		case GLFW_KEY_Z:             return GFX_KEY_Z;
		case GLFW_KEY_0:             return GFX_KEY_0;
		case GLFW_KEY_1:             return GFX_KEY_1;
		case GLFW_KEY_2:             return GFX_KEY_2;
		case GLFW_KEY_3:             return GFX_KEY_3;
		case GLFW_KEY_4:             return GFX_KEY_4;
		case GLFW_KEY_5:             return GFX_KEY_5;
		case GLFW_KEY_6:             return GFX_KEY_6;
		case GLFW_KEY_7:             return GFX_KEY_7;
		case GLFW_KEY_8:             return GFX_KEY_8;
		case GLFW_KEY_9:             return GFX_KEY_9;
		case GLFW_KEY_KP_0:          return GFX_KEY_KP_0;
		case GLFW_KEY_KP_1:          return GFX_KEY_KP_1;
		case GLFW_KEY_KP_2:          return GFX_KEY_KP_2;
		case GLFW_KEY_KP_3:          return GFX_KEY_KP_3;
		case GLFW_KEY_KP_4:          return GFX_KEY_KP_4;
		case GLFW_KEY_KP_5:          return GFX_KEY_KP_5;
		case GLFW_KEY_KP_6:          return GFX_KEY_KP_6;
		case GLFW_KEY_KP_7:          return GFX_KEY_KP_7;
		case GLFW_KEY_KP_8:          return GFX_KEY_KP_8;
		case GLFW_KEY_KP_9:          return GFX_KEY_KP_9;
		case GLFW_KEY_KP_DIVIDE:     return GFX_KEY_KP_DIVIDE;
		case GLFW_KEY_KP_MULTIPLY:   return GFX_KEY_KP_MULTIPLY;
		case GLFW_KEY_KP_SUBTRACT:   return GFX_KEY_KP_SUBTRACT;
		case GLFW_KEY_KP_ADD:        return GFX_KEY_KP_ADD;
		case GLFW_KEY_KP_EQUAL:      return GFX_KEY_KP_EQUAL;
		case GLFW_KEY_KP_DECIMAL:    return GFX_KEY_KP_DECIMAL;
		case GLFW_KEY_KP_ENTER:      return GFX_KEY_KP_ENTER;
		case GLFW_KEY_F1:            return GFX_KEY_F1;
		case GLFW_KEY_F2:            return GFX_KEY_F2;
		case GLFW_KEY_F3:            return GFX_KEY_F3;
		case GLFW_KEY_F4:            return GFX_KEY_F4;
		case GLFW_KEY_F5:            return GFX_KEY_F5;
		case GLFW_KEY_F6:            return GFX_KEY_F6;
		case GLFW_KEY_F7:            return GFX_KEY_F7;
		case GLFW_KEY_F8:            return GFX_KEY_F8;
		case GLFW_KEY_F9:            return GFX_KEY_F9;
		case GLFW_KEY_F10:           return GFX_KEY_F10;
		case GLFW_KEY_F11:           return GFX_KEY_F11;
		case GLFW_KEY_F12:           return GFX_KEY_F12;
		case GLFW_KEY_F13:           return GFX_KEY_F13;
		case GLFW_KEY_F14:           return GFX_KEY_F14;
		case GLFW_KEY_F15:           return GFX_KEY_F15;
		case GLFW_KEY_F16:           return GFX_KEY_F16;
		case GLFW_KEY_F17:           return GFX_KEY_F17;
		case GLFW_KEY_F18:           return GFX_KEY_F18;
		case GLFW_KEY_F19:           return GFX_KEY_F19;
		case GLFW_KEY_F20:           return GFX_KEY_F20;
		case GLFW_KEY_F21:           return GFX_KEY_F21;
		case GLFW_KEY_F22:           return GFX_KEY_F22;
		case GLFW_KEY_F23:           return GFX_KEY_F23;
		case GLFW_KEY_F24:           return GFX_KEY_F24;
		case GLFW_KEY_LEFT_SHIFT:    return GFX_KEY_LSHIFT;
		case GLFW_KEY_RIGHT_SHIFT:   return GFX_KEY_RSHIFT;
		case GLFW_KEY_LEFT_CONTROL:  return GFX_KEY_LCONTROL;
		case GLFW_KEY_RIGHT_CONTROL: return GFX_KEY_RCONTROL;
		case GLFW_KEY_LEFT_ALT:      return GFX_KEY_LALT;
		case GLFW_KEY_RIGHT_ALT:     return GFX_KEY_RALT;
		case GLFW_KEY_LEFT_SUPER:    return GFX_KEY_LSUPER;
		case GLFW_KEY_RIGHT_SUPER:   return GFX_KEY_RSUPER;
		case GLFW_KEY_LEFT:          return GFX_KEY_LEFT;
		case GLFW_KEY_RIGHT:         return GFX_KEY_RIGHT;
		case GLFW_KEY_UP:            return GFX_KEY_UP;
		case GLFW_KEY_DOWN:          return GFX_KEY_DOWN;
		case GLFW_KEY_SPACE:         return GFX_KEY_SPACE;
		case GLFW_KEY_BACKSPACE:     return GFX_KEY_BACKSPACE;
		case GLFW_KEY_ENTER:         return GFX_KEY_ENTER;
		case GLFW_KEY_TAB:           return GFX_KEY_TAB;
		case GLFW_KEY_ESCAPE:        return GFX_KEY_ESCAPE;
		case GLFW_KEY_PAUSE:         return GFX_KEY_PAUSE;
		case GLFW_KEY_DELETE:        return GFX_KEY_DELETE;
		case GLFW_KEY_INSERT:        return GFX_KEY_INSERT;
		case GLFW_KEY_HOME:          return GFX_KEY_HOME;
		case GLFW_KEY_PAGE_UP:       return GFX_KEY_PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:     return GFX_KEY_PAGE_DOWN;
		case GLFW_KEY_END:           return GFX_KEY_END;
		case GLFW_KEY_COMMA:         return GFX_KEY_COMMA;
		case GLFW_KEY_PERIOD:        return GFX_KEY_PERIOD;
		case GLFW_KEY_SLASH:         return GFX_KEY_SLASH;
		case GLFW_KEY_APOSTROPHE:    return GFX_KEY_APOSTROPHE;
		case GLFW_KEY_SEMICOLON:     return GFX_KEY_SEMICOLON;
		case GLFW_KEY_GRAVE_ACCENT:  return GFX_KEY_GRAVE;
		case GLFW_KEY_LEFT_BRACKET:  return GFX_KEY_LBRACKET;
		case GLFW_KEY_RIGHT_BRACKET: return GFX_KEY_RBRACKET;
		case GLFW_KEY_BACKSLASH:     return GFX_KEY_BACKSLASH;
		case GLFW_KEY_EQUAL:         return GFX_KEY_EQUAL;
		case GLFW_KEY_MINUS:         return GFX_KEY_SUBTRACT;
		case GLFW_KEY_SCROLL_LOCK:   return GFX_KEY_SCROLL_LOCK;
		case GLFW_KEY_NUM_LOCK:      return GFX_KEY_NUM_LOCK;
		case GLFW_KEY_CAPS_LOCK:     return GFX_KEY_CAPS_LOCK;
		case GLFW_KEY_PRINT_SCREEN:  return GFX_KEY_PRINT;
	}
	return GFX_KEY_UNKNOWN;
}

static uint32_t get_mods(int state)
{
	uint32_t mods = 0;
	if (state & GLFW_MOD_CONTROL)
		mods |= GFX_KEY_MOD_CONTROL;
	if (state & GLFW_MOD_SHIFT)
		mods |= GFX_KEY_MOD_SHIFT;
#ifdef GLFW_MOD_CAPS_LOCK
	if (state & GLFW_MOD_CAPS_LOCK)
		mods |= GFX_KEY_MOD_CAPS_LOCK;
#endif
	if (state & GLFW_MOD_ALT)
		mods |= GFX_KEY_MOD_ALT;
#ifdef GLFW_MOD_NUM_LOCK
	if (state & GLFW_MOD_NUM_LOCK)
		mods |= GFX_KEY_MOD_NUM_LOCK;
#endif
	if (state & GLFW_MOD_SUPER)
		mods |= GFX_KEY_MOD_SUPER;
	return mods;
}

static enum gfx_mouse_button get_button(int button)
{
	switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
			return GFX_MOUSE_BUTTON_LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return GFX_MOUSE_BUTTON_RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return GFX_MOUSE_BUTTON_MIDDLE;
	}
	return (enum gfx_mouse_button)button;
}
