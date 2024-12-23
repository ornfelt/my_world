#include "windows/sdl.h"

#include "window_vtable.h"
#include "config.h"

#include <jks/utf8.h>
#include <stdlib.h>
#include <string.h>

static const int cursors[GFX_CURSOR_LAST] =
{
	[GFX_CURSOR_ARROW] = SDL_SYSTEM_CURSOR_ARROW,
	[GFX_CURSOR_CROSS] = SDL_SYSTEM_CURSOR_CROSSHAIR,
	[GFX_CURSOR_HAND] = SDL_SYSTEM_CURSOR_HAND,
	[GFX_CURSOR_IBEAM] = SDL_SYSTEM_CURSOR_IBEAM,
	[GFX_CURSOR_NO] = SDL_SYSTEM_CURSOR_NO,
	[GFX_CURSOR_SIZEALL] = SDL_SYSTEM_CURSOR_SIZEALL,
	[GFX_CURSOR_VRESIZE] = SDL_SYSTEM_CURSOR_SIZENS,
	[GFX_CURSOR_HRESIZE] = SDL_SYSTEM_CURSOR_SIZEWE,
	[GFX_CURSOR_WAIT] = SDL_SYSTEM_CURSOR_WAITARROW,
	[GFX_CURSOR_BLANK] = 0,
};

static enum gfx_key_code get_key_code(int key_code);
static enum gfx_mouse_button get_button(int button);
static uint32_t get_mods(int state);

void *gfx_sdl_get_proc_address(const char *name)
{
	return SDL_GL_GetProcAddress(name);
}

bool gfx_sdl_create_window(struct gfx_sdl_window *window, const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	uint32_t flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
	switch (properties->device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GL3
		case GFX_DEVICE_GL3:
			flags |= SDL_WINDOW_OPENGL;
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		case GFX_DEVICE_GL4:
			flags |= SDL_WINDOW_OPENGL;
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_GLES3
		case GFX_DEVICE_GLES3:
			flags |= SDL_WINDOW_OPENGL;
			break;
#endif
#ifdef GFX_ENABLE_DEVICE_VK
		case GFX_DEVICE_VK:
			flags |= SDL_WINDOW_VULKAN;
			break;
#endif
		default:
			GFX_ERROR_CALLBACK("unknown sdl backend");
			return false;
	}
	window->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (!window->window)
	{
		GFX_ERROR_CALLBACK("failed to create sdl window");
		return false;
	}
	return true;
}

bool gfx_sdl_ctr(struct gfx_sdl_window *window, struct gfx_window *winref)
{
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		GFX_ERROR_CALLBACK("failed to initialize sdl");
		return false;
	}
	window->winref = winref;
	window->window = NULL;
	return true;
}

void gfx_sdl_dtr(struct gfx_sdl_window *window)
{
	SDL_DestroyWindow(window->window);
}

void gfx_sdl_show(struct gfx_sdl_window *window)
{
	SDL_ShowWindow(window->window);
}

void gfx_sdl_hide(struct gfx_sdl_window *window)
{
	SDL_HideWindow(window->window);
}

static void process_event(struct gfx_sdl_window *window, SDL_Event *event)
{
	switch (event->type)
	{
		case SDL_KEYDOWN:
			/* FALLTHROUGH */
		case SDL_KEYUP:
		{
			enum gfx_key_code key_code = get_key_code(event->key.keysym.sym);
			if (key_code < sizeof(window->winref->keys) * 8)
			{
				if (event->type == SDL_KEYDOWN)
					window->winref->keys[key_code / 8] |= 1 << (key_code % 8);
				else
					window->winref->keys[key_code / 8] &= ~(1 << (key_code % 8));
			}
			struct gfx_key_event evt;
			evt.used = false;
			evt.key = key_code;
			evt.mods = get_mods(event->key.keysym.mod);
			if (event->type == SDL_KEYDOWN)
			{
				if (event->key.repeat)
				{
					if (window->winref->key_press_callback)
						window->winref->key_press_callback(window->winref, &evt);
				}
				else
				{
					if (window->winref->key_down_callback)
						window->winref->key_down_callback(window->winref, &evt);
				}
			}
			else
			{
				if (window->winref->key_up_callback)
					window->winref->key_up_callback(window->winref, &evt);
			}
			break;
		}
		case SDL_TEXTINPUT:
		{
			if (window->winref->char_callback)
			{
				struct gfx_char_event evt;
				evt.used = false;
				if (strlen(event->text.text) > 4)
				{
					GFX_ERROR_CALLBACK("invalid input utf8");
					break;
				}
				strcpy(evt.utf8, event->text.text);
				const char *pos = &evt.utf8[0];
				if (utf8_decode(&pos, &evt.codepoint))
					window->winref->char_callback(window->winref, &evt);
				else
					GFX_ERROR_CALLBACK("invalid input codepoint");
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			if (window->winref->grabbed)
			{
				window->winref->virtual_x = event->motion.x;
				window->winref->virtual_y = event->motion.y;
			}
			else
			{
				window->winref->mouse_x = event->motion.x;
				window->winref->mouse_y = event->motion.y;
			}
			if (window->winref->mouse_move_callback)
			{
				struct gfx_pointer_event evt;
				evt.used = false;
				evt.x = event->motion.x;
				evt.y = event->motion.y;
				window->winref->mouse_move_callback(window->winref, &evt);
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			enum gfx_mouse_button button = get_button(event->button.button);
			if (button >= 0 && button < GFX_MOUSE_BUTTON_LAST)
				window->winref->mouse_buttons &= ~(1 << button);
			if (window->winref->mouse_down_callback)
			{
				struct gfx_mouse_event evt;
				evt.used = false;
				evt.x = gfx_get_mouse_x(window->winref);
				evt.y = gfx_get_mouse_y(window->winref);
				evt.button = button;
				evt.mods = get_mods(SDL_GetModState());
				window->winref->mouse_down_callback(window->winref, &evt);
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			enum gfx_mouse_button button = get_button(event->button.button);
			if (button >= 0 && button < GFX_MOUSE_BUTTON_LAST)
				window->winref->mouse_buttons |= 1 << button;
			if (window->winref->mouse_up_callback)
			{
				struct gfx_mouse_event evt;
				evt.used = false;
				evt.x = gfx_get_mouse_x(window->winref);
				evt.y = gfx_get_mouse_y(window->winref);
				evt.button = button;
				evt.mods = get_mods(SDL_GetModState());
				window->winref->mouse_up_callback(window->winref, &evt);
			}
			break;
		}
		case SDL_WINDOWEVENT:
		{
			switch (event->window.event)
			{
				case SDL_WINDOWEVENT_ENTER:
					if (window->winref->cursor_enter_callback)
						window->winref->cursor_enter_callback(window->winref);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					if (window->winref->cursor_leave_callback)
						window->winref->cursor_leave_callback(window->winref);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					if (window->winref->focus_in_callback)
						window->winref->focus_in_callback(window->winref);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					if (window->winref->focus_out_callback)
						window->winref->focus_out_callback(window->winref);
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					window->winref->width = event->window.data1;
					window->winref->height = event->window.data2;
					if (window->winref->resize_callback)
					{
						struct gfx_resize_event evt;
						evt.used = false;
						evt.width = event->window.data1;
						evt.height = event->window.data2;
						window->winref->resize_callback(window->winref, &evt);
					}
					break;
				case SDL_WINDOWEVENT_MOVED:
					window->winref->x = event->window.data1;
					window->winref->y = event->window.data2;
					if (window->winref->move_callback)
					{
						struct gfx_move_event evt;
						evt.used = false;
						evt.x = event->window.data1;
						evt.y = event->window.data2;
						window->winref->move_callback(window->winref, &evt);
					}
					break;
				case SDL_WINDOWEVENT_CLOSE:
					window->winref->close_requested = true;
					if (window->winref->close_callback)
						window->winref->close_callback(window->winref);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					if (window->winref->expose_callback)
						window->winref->expose_callback(window->winref);
					break;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			if (window->winref->scroll_callback)
			{
				struct gfx_scroll_event evt;
				evt.used = false;
				evt.mouse_x = gfx_get_mouse_x(window->winref);
				evt.mouse_y = gfx_get_mouse_y(window->winref);
				evt.x = event->wheel.x;
				evt.y = event->wheel.y;
				window->winref->scroll_callback(window->winref, &evt);
			}
			break;
		}
	}
}

void gfx_sdl_poll_events(struct gfx_sdl_window *window)
{
	(void)window;
	SDL_Event event;
	while (SDL_PollEvent(&event))
		process_event(window, &event);
}

void gfx_sdl_wait_events(struct gfx_sdl_window *window)
{
	(void)window;
	SDL_Event event;
	if (!SDL_WaitEvent(&event))
		return;
	process_event(window, &event);
	gfx_sdl_poll_events(window);
}

void gfx_sdl_set_title(struct gfx_sdl_window *window, const char *title)
{
	SDL_SetWindowTitle(window->window, title);
}

void gfx_sdl_set_icon(struct gfx_sdl_window *window, const void *data, uint32_t width, uint32_t height)
{
	SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormatFrom((void*)data, width, height, 8, 4 * width, SDL_PIXELFORMAT_ABGR8888);
	SDL_SetWindowIcon(window->window, icon);
	SDL_FreeSurface(icon);
}

void gfx_sdl_resize(struct gfx_sdl_window *window, uint32_t width, uint32_t height)
{
	SDL_SetWindowSize(window->window, width, height);
}

void gfx_sdl_grab_cursor(struct gfx_sdl_window *window)
{
	SDL_SetRelativeMouseMode(SDL_TRUE);
	window->winref->grabbed = true;
}

void gfx_sdl_ungrab_cursor(struct gfx_sdl_window *window)
{
	SDL_SetRelativeMouseMode(SDL_FALSE);
	window->winref->grabbed = false;
}

char *gfx_sdl_get_clipboard(struct gfx_sdl_window *window)
{
	(void)window;
	return SDL_GetClipboardText();
}

void gfx_sdl_set_clipboard(struct gfx_sdl_window *window, const char *text)
{
	(void)window;
	SDL_SetClipboardText(text);
}

gfx_cursor_t gfx_sdl_create_native_cursor(struct gfx_sdl_window *window, enum gfx_native_cursor cursor)
{
	(void)window;
	return SDL_CreateSystemCursor(cursors[cursor]);
}

gfx_cursor_t gfx_sdl_create_cursor(struct gfx_sdl_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	(void)window;
	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom((void*)data, width, height, 8, 4 * width, SDL_PIXELFORMAT_ABGR8888);
	SDL_Cursor *cursor = SDL_CreateColorCursor(surface, xhot, yhot);
	SDL_FreeSurface(surface);
	return cursor;
}

void gfx_sdl_delete_cursor(struct gfx_sdl_window *window, gfx_cursor_t cursor)
{
	(void)window;
	if (!cursor)
		return;
	SDL_FreeCursor((SDL_Cursor*)cursor);
}

void gfx_sdl_set_cursor(struct gfx_sdl_window *window, gfx_cursor_t cursor)
{
	(void)window;
	SDL_SetCursor(cursor);
}

void gfx_sdl_set_mouse_position(struct gfx_sdl_window *window, int32_t x, int32_t y)
{
	SDL_WarpMouseInWindow(window->window, x, y);
}

static enum gfx_key_code get_key_code(int key_code)
{
	switch (key_code)
	{
		case SDLK_a:             return GFX_KEY_A;
		case SDLK_b:             return GFX_KEY_B;
		case SDLK_c:             return GFX_KEY_C;
		case SDLK_d:             return GFX_KEY_D;
		case SDLK_e:             return GFX_KEY_E;
		case SDLK_f:             return GFX_KEY_F;
		case SDLK_g:             return GFX_KEY_G;
		case SDLK_h:             return GFX_KEY_H;
		case SDLK_i:             return GFX_KEY_I;
		case SDLK_j:             return GFX_KEY_J;
		case SDLK_k:             return GFX_KEY_K;
		case SDLK_l:             return GFX_KEY_L;
		case SDLK_m:             return GFX_KEY_M;
		case SDLK_n:             return GFX_KEY_N;
		case SDLK_o:             return GFX_KEY_O;
		case SDLK_p:             return GFX_KEY_P;
		case SDLK_q:             return GFX_KEY_Q;
		case SDLK_r:             return GFX_KEY_R;
		case SDLK_s:             return GFX_KEY_S;
		case SDLK_t:             return GFX_KEY_T;
		case SDLK_u:             return GFX_KEY_U;
		case SDLK_v:             return GFX_KEY_V;
		case SDLK_w:             return GFX_KEY_W;
		case SDLK_x:             return GFX_KEY_X;
		case SDLK_y:             return GFX_KEY_Y;
		case SDLK_z:             return GFX_KEY_Z;
		case SDLK_0:             return GFX_KEY_0;
		case SDLK_1:             return GFX_KEY_1;
		case SDLK_2:             return GFX_KEY_2;
		case SDLK_3:             return GFX_KEY_3;
		case SDLK_4:             return GFX_KEY_4;
		case SDLK_5:             return GFX_KEY_5;
		case SDLK_6:             return GFX_KEY_6;
		case SDLK_7:             return GFX_KEY_7;
		case SDLK_8:             return GFX_KEY_8;
		case SDLK_9:             return GFX_KEY_9;
		case SDLK_KP_0:          return GFX_KEY_KP_0;
		case SDLK_KP_1:          return GFX_KEY_KP_1;
		case SDLK_KP_2:          return GFX_KEY_KP_2;
		case SDLK_KP_3:          return GFX_KEY_KP_3;
		case SDLK_KP_4:          return GFX_KEY_KP_4;
		case SDLK_KP_5:          return GFX_KEY_KP_5;
		case SDLK_KP_6:          return GFX_KEY_KP_6;
		case SDLK_KP_7:          return GFX_KEY_KP_7;
		case SDLK_KP_8:          return GFX_KEY_KP_8;
		case SDLK_KP_9:          return GFX_KEY_KP_9;
		case SDLK_KP_DIVIDE:     return GFX_KEY_KP_DIVIDE;
		case SDLK_KP_MULTIPLY:   return GFX_KEY_KP_MULTIPLY;
		case SDLK_KP_MINUS:      return GFX_KEY_KP_SUBTRACT;
		case SDLK_KP_PLUS:       return GFX_KEY_KP_ADD;
		case SDLK_KP_EQUALS:     return GFX_KEY_KP_EQUAL;
		case SDLK_KP_PERIOD:     return GFX_KEY_KP_DECIMAL;
		case SDLK_KP_ENTER:      return GFX_KEY_KP_ENTER;
		case SDLK_F1:            return GFX_KEY_F1;
		case SDLK_F2:            return GFX_KEY_F2;
		case SDLK_F3:            return GFX_KEY_F3;
		case SDLK_F4:            return GFX_KEY_F4;
		case SDLK_F5:            return GFX_KEY_F5;
		case SDLK_F6:            return GFX_KEY_F6;
		case SDLK_F7:            return GFX_KEY_F7;
		case SDLK_F8:            return GFX_KEY_F8;
		case SDLK_F9:            return GFX_KEY_F9;
		case SDLK_F10:           return GFX_KEY_F10;
		case SDLK_F11:           return GFX_KEY_F11;
		case SDLK_F12:           return GFX_KEY_F12;
		case SDLK_F13:           return GFX_KEY_F13;
		case SDLK_F14:           return GFX_KEY_F14;
		case SDLK_F15:           return GFX_KEY_F15;
		case SDLK_F16:           return GFX_KEY_F16;
		case SDLK_F17:           return GFX_KEY_F17;
		case SDLK_F18:           return GFX_KEY_F18;
		case SDLK_F19:           return GFX_KEY_F19;
		case SDLK_F20:           return GFX_KEY_F20;
		case SDLK_F21:           return GFX_KEY_F21;
		case SDLK_F22:           return GFX_KEY_F22;
		case SDLK_F23:           return GFX_KEY_F23;
		case SDLK_F24:           return GFX_KEY_F24;
		case SDLK_LSHIFT:        return GFX_KEY_LSHIFT;
		case SDLK_RSHIFT:        return GFX_KEY_RSHIFT;
		case SDLK_LCTRL:         return GFX_KEY_LCONTROL;
		case SDLK_RCTRL:         return GFX_KEY_RCONTROL;
		case SDLK_LALT:          return GFX_KEY_LALT;
		case SDLK_RALT:          return GFX_KEY_RALT;
		case SDLK_LGUI:          return GFX_KEY_LSUPER;
		case SDLK_RGUI:          return GFX_KEY_RSUPER;
		case SDLK_LEFT:          return GFX_KEY_LEFT;
		case SDLK_RIGHT:         return GFX_KEY_RIGHT;
		case SDLK_UP:            return GFX_KEY_UP;
		case SDLK_DOWN:          return GFX_KEY_DOWN;
		case SDLK_SPACE:         return GFX_KEY_SPACE;
		case SDLK_BACKSPACE:     return GFX_KEY_BACKSPACE;
		case SDLK_RETURN:        return GFX_KEY_ENTER;
		case SDLK_TAB:           return GFX_KEY_TAB;
		case SDLK_ESCAPE:        return GFX_KEY_ESCAPE;
		case SDLK_PAUSE:         return GFX_KEY_PAUSE;
		case SDLK_DELETE:        return GFX_KEY_DELETE;
		case SDLK_INSERT:        return GFX_KEY_INSERT;
		case SDLK_HOME:          return GFX_KEY_HOME;
		case SDLK_PAGEUP:        return GFX_KEY_PAGE_UP;
		case SDLK_PAGEDOWN:      return GFX_KEY_PAGE_DOWN;
		case SDLK_END:           return GFX_KEY_END;
		case SDLK_COMMA:         return GFX_KEY_COMMA;
		case SDLK_PERIOD:        return GFX_KEY_PERIOD;
		case SDLK_SLASH:         return GFX_KEY_SLASH;
		case SDLK_QUOTE:         return GFX_KEY_APOSTROPHE;
		case SDLK_SEMICOLON:     return GFX_KEY_SEMICOLON;
		case SDLK_BACKQUOTE:     return GFX_KEY_GRAVE;
		case SDLK_LEFTBRACKET:   return GFX_KEY_LBRACKET;
		case SDLK_RIGHTBRACKET:  return GFX_KEY_RBRACKET;
		case SDLK_BACKSLASH:     return GFX_KEY_BACKSLASH;
		case SDLK_EQUALS:        return GFX_KEY_EQUAL;
		case SDLK_MINUS:         return GFX_KEY_SUBTRACT;
		case SDLK_SCROLLLOCK:    return GFX_KEY_SCROLL_LOCK;
		case SDLK_NUMLOCKCLEAR:  return GFX_KEY_NUM_LOCK;
		case SDLK_CAPSLOCK:      return GFX_KEY_CAPS_LOCK;
		case SDLK_PRINTSCREEN:   return GFX_KEY_PRINT;
	}
	return GFX_KEY_UNKNOWN;
}

static uint32_t get_mods(int state)
{
	uint32_t mods = 0;
	if (state & KMOD_CTRL)
		mods |= GFX_KEY_MOD_CONTROL;
	if (state & KMOD_SHIFT)
		mods |= GFX_KEY_MOD_SHIFT;
	if (state & KMOD_CAPS)
		mods |= GFX_KEY_MOD_CAPS_LOCK;
	if (state & KMOD_ALT)
		mods |= GFX_KEY_MOD_ALT;
	if (state & KMOD_NUM)
		mods |= GFX_KEY_MOD_NUM_LOCK;
	if (state & KMOD_GUI)
		mods |= GFX_KEY_MOD_SUPER;
	return mods;
}

static enum gfx_mouse_button get_button(int button)
{
	switch (button)
	{
		case SDL_BUTTON_LEFT:
			return GFX_MOUSE_BUTTON_LEFT;
		case SDL_BUTTON_RIGHT:
			return GFX_MOUSE_BUTTON_RIGHT;
		case SDL_BUTTON_MIDDLE:
			return GFX_MOUSE_BUTTON_MIDDLE;
	}
	return (enum gfx_mouse_button)button;
}
