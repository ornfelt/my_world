#include "window.h"
#include "window_vtable.h"
#include "config.h"
#include "device.h"
#include <stdlib.h>
#include <string.h>

#if defined(GFX_ENABLE_WINDOW_X11)
# if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
struct gfx_window *gfx_glx_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
# if defined(GFX_ENABLE_DEVICE_VK)
struct gfx_window *gfx_vk_x11_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
#endif

#if defined(GFX_ENABLE_WINDOW_WL)
# if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
struct gfx_window *gfx_egl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
# if defined(GFX_ENABLE_DEVICE_VK)
struct gfx_window *gfx_vk_wl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
#endif

#if defined(GFX_ENABLE_WINDOW_WIN32)
# if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
struct gfx_window *gfx_wgl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
# if defined(GFX_ENABLE_DEVICE_D3D9) || defined(GFX_ENABLE_DEVICE_D3D11)
struct gfx_window *gfx_d3d_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
# if defined(GFX_ENABLE_DEVICE_VK)
struct gfx_window *gfx_vk_win32_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
#endif

#if defined(GFX_ENABLE_WINDOW_GLFW)
# if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
struct gfx_window *gfx_gl_glfw_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
# if defined(GFX_ENABLE_DEVICE_VK)
struct gfx_window *gfx_vk_glfw_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
#endif

#if defined(GFX_ENABLE_WINDOW_SDL)
# if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
struct gfx_window *gfx_gl_sdl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
# if defined(GFX_ENABLE_DEVICE_VK)
struct gfx_window *gfx_vk_sdl_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties);
# endif
#endif

#if 0
# include <stdio.h>
# define WIN_DEBUG printf("%s@%s:%d\n", __func__, __FILE__, __LINE__)
#else
# define WIN_DEBUG
#endif

struct gfx_memory gfx_memory;
gfx_error_callback_t gfx_error_callback;

bool gfx_has_window_backend(enum gfx_window_backend backend)
{
#if defined(GFX_ENABLE_WINDOW_X11)
	if (backend == GFX_WINDOW_X11)
		return true;
#endif

#if defined(GFX_ENABLE_WINDOW_WL)
	if (backend == GFX_WINDOW_WAYLAND)
		return true;
#endif

#if defined(GFX_ENABLE_WINDOW_WIN32)
	if (backend == GFX_WINDOW_WIN32)
		return true;
#endif

#if defined(GFX_ENABLE_WINDOW_GLFW)
	if (backend == GFX_WINDOW_GLFW)
		return true;
#endif

#if defined(GFX_ENABLE_WINDOW_SDL)
	if (backend == GFX_WINDOW_SDL)
		return true;
#endif

	return false;
}

bool gfx_has_device_backend(enum gfx_device_backend backend)
{
#if defined(GFX_ENABLE_DEVICE_GL3)
	if (backend == GFX_DEVICE_GL3)
		return true;
#endif

#if defined(GFX_ENABLE_DEVICE_GL4)
	if (backend == GFX_DEVICE_GL4)
		return true;
#endif

#if defined(GFX_ENABLE_DEVICE_D3D9)
	if (backend == GFX_DEVICE_D3D9)
		return true;
#endif

#if defined(GFX_ENABLE_DEVICE_D3D11)
	if (backend == GFX_DEVICE_D3D11)
		return true;
#endif

#if defined(GFX_ENABLE_DEVICE_VK)
	if (backend == GFX_DEVICE_VK)
		return true;
#endif

#if defined(GFX_ENABLE_DEVICE_GLES3)
	if (backend == GFX_DEVICE_GLES3)
		return true;
#endif

	return false;
}

static bool ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	window->key_down_callback = NULL;
	window->key_press_callback = NULL;
	window->key_up_callback = NULL;
	window->char_callback = NULL;
	window->mouse_down_callback = NULL;
	window->mouse_up_callback = NULL;
	window->scroll_callback = NULL;
	window->mouse_move_callback = NULL;
	window->cursor_enter_callback = NULL;
	window->cursor_leave_callback = NULL;
	window->focus_in_callback = NULL;
	window->focus_out_callback = NULL;
	window->resize_callback = NULL;
	window->move_callback = NULL;
	window->expose_callback = NULL;
	window->close_callback = NULL;
	window->properties = *properties;
	window->virtual_x = 0;
	window->virtual_y = 0;
	window->mouse_x = 0;
	window->mouse_y = 0;
	window->height = 0;
	window->width = 0;
	window->x = 0;
	window->y = 0;
	window->close_requested = false;
	window->grabbed = false;
	memset(window->keys, 0, sizeof(window->keys));
	window->mouse_buttons = 0;
	window->device = NULL;
	return true;
}

static void dtr(struct gfx_window *window)
{
	if (window->device)
		gfx_device_delete(window->device);
}

const struct gfx_window_vtable gfx_window_vtable =
{
	.ctr = ctr,
	.dtr = dtr,
};

struct gfx_window *gfx_create_window(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	switch (properties->window_backend)
	{
#if defined(GFX_ENABLE_WINDOW_X11)
		case GFX_WINDOW_X11:
			switch (properties->device_backend)
			{
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
				case GFX_DEVICE_GL3:
				case GFX_DEVICE_GL4:
				case GFX_DEVICE_GLES3:
					return gfx_glx_window_new(title, width, height, properties);
#endif
#if defined(GFX_ENABLE_DEVICE_VK)
				case GFX_DEVICE_VK:
					return gfx_vk_x11_window_new(title, width, height, properties);
#endif
				default:
					break;
			}
			break;
#endif
#if defined(GFX_ENABLE_WINDOW_WIN32)
		case GFX_WINDOW_WIN32:
			switch (properties->device_backend)
			{
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
				case GFX_DEVICE_GL3:
				case GFX_DEVICE_GL4:
				case GFX_DEVICE_GLES3:
					return gfx_wgl_window_new(title, width, height, properties);
#endif
#if defined(GFX_ENABLE_DEVICE_D3D9) || defined(GFX_ENABLE_DEVICE_D3D11)
				case GFX_DEVICE_D3D9:
				case GFX_DEVICE_D3D11:
					return gfx_d3d_window_new(title, width, height, properties);
#endif
#if defined(GFX_ENABLE_DEVICE_VK)
				case GFX_DEVICE_VK:
					return gfx_vk_win32_window_new(title, width, height, properties);
#endif
				default:
					break;
			}
			break;
#endif
#if defined(GFX_ENABLE_WINDOW_WL)
		case GFX_WINDOW_WAYLAND:
			switch (properties->device_backend)
			{
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
				case GFX_DEVICE_GL3:
				case GFX_DEVICE_GL4:
				case GFX_DEVICE_GLES3:
					return gfx_egl_window_new(title, width, height, properties);
#endif
#if defined(GFX_ENABLE_DEVICE_VK)
				case GFX_DEVICE_VK:
					return gfx_vk_wl_window_new(title, width, height, properties);
#endif
				default:
					break;
			}
			break;
#endif
#if defined(GFX_ENABLE_WINDOW_GLFW)
		case GFX_WINDOW_GLFW:
			switch (properties->device_backend)
			{
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
				case GFX_DEVICE_GL3:
				case GFX_DEVICE_GL4:
				case GFX_DEVICE_GLES3:
					return gfx_gl_glfw_window_new(title, width, height, properties);
#endif
#if defined(GFX_ENABLE_DEVICE_VK)
				case GFX_DEVICE_VK:
					return gfx_vk_glfw_window_new(title, width, height, properties);
#endif
				default:
					break;
			}
			break;
#endif
#if defined(GFX_ENABLE_WINDOW_SDL)
		case GFX_WINDOW_SDL:
			switch (properties->device_backend)
			{
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GLES3)
				case GFX_DEVICE_GL3:
				case GFX_DEVICE_GL4:
				case GFX_DEVICE_GLES3:
					return gfx_gl_sdl_window_new(title, width, height, properties);
#endif
#if defined(GFX_ENABLE_DEVICE_VK)
				case GFX_DEVICE_VK:
					return gfx_vk_sdl_window_new(title, width, height, properties);
#endif
				default:
					break;
			}
			break;
#endif
		default:
			break;
	}
	return NULL;
}

void gfx_delete_window(struct gfx_window *window)
{
	if (window == NULL)
		return;
	WIN_DEBUG;
	window->vtable->dtr(window);
	GFX_FREE(window);
	WIN_DEBUG;
}

bool gfx_create_device(struct gfx_window *window)
{
	WIN_DEBUG;
	bool ret = window->vtable->create_device(window);
	WIN_DEBUG;
	return ret && window->device;
}

void gfx_window_show(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->show(window);
	WIN_DEBUG;
}

void gfx_window_hide(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->hide(window);
	WIN_DEBUG;
}

void gfx_window_poll_events(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->poll_events(window);
	WIN_DEBUG;
}

void gfx_window_wait_events(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->wait_events(window);
	WIN_DEBUG;
}

void gfx_window_grab_cursor(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->grab_cursor(window);
	WIN_DEBUG;
}

void gfx_window_ungrab_cursor(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->ungrab_cursor(window);
	WIN_DEBUG;
}

void gfx_window_swap_buffers(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->swap_buffers(window);
	WIN_DEBUG;
}

void gfx_window_make_current(struct gfx_window *window)
{
	WIN_DEBUG;
	window->vtable->make_current(window);
	WIN_DEBUG;
}

void gfx_window_set_swap_interval(struct gfx_window *window, int interval)
{
	WIN_DEBUG;
	window->vtable->set_swap_interval(window, interval);
	WIN_DEBUG;
}

void gfx_window_set_title(struct gfx_window *window, const char *title)
{
	WIN_DEBUG;
	window->vtable->set_title(window, title);
	WIN_DEBUG;
}

void gfx_window_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	WIN_DEBUG;
	window->vtable->set_icon(window, data, width, height);
	WIN_DEBUG;
}

void gfx_window_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	WIN_DEBUG;
	window->vtable->resize(window, width, height);
	WIN_DEBUG;
}

char *gfx_window_get_clipboard(struct gfx_window *window)
{
	WIN_DEBUG;
	char *ret = window->vtable->get_clipboard(window);
	WIN_DEBUG;
	return ret;
}

void gfx_window_set_clipboard(struct gfx_window *window, const char *clipboard)
{
	WIN_DEBUG;
	window->vtable->set_clipboard(window, clipboard);
	WIN_DEBUG;
}

gfx_cursor_t gfx_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor native_cursor)
{
	WIN_DEBUG;
	gfx_cursor_t cursor = window->vtable->create_native_cursor(window, native_cursor);
	WIN_DEBUG;
	return cursor;
}

gfx_cursor_t gfx_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	WIN_DEBUG;
	gfx_cursor_t cursor = window->vtable->create_cursor(window, data, width, height, xhot, yhot);
	WIN_DEBUG;
	return cursor;
}

void gfx_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	WIN_DEBUG;
	window->vtable->delete_cursor(window, cursor);
	WIN_DEBUG;
}

void gfx_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	WIN_DEBUG;
	window->vtable->set_cursor(window, cursor);
	WIN_DEBUG;
}

void gfx_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	WIN_DEBUG;
	window->vtable->set_mouse_position(window, x, y);
	WIN_DEBUG;
}

int32_t gfx_get_mouse_x(struct gfx_window *window)
{
	if (window->grabbed)
		return window->virtual_x;
	return window->mouse_x;
}

int32_t gfx_get_mouse_y(struct gfx_window *window)
{
	if (window->grabbed)
		return window->virtual_y;
	return window->mouse_y;
}

bool gfx_is_key_down(struct gfx_window *window, enum gfx_key_code key)
{
	if (key >= GFX_KEY_LAST)
		return false;
	return window->keys[key / 8] & (1 << (key % 8));
}

bool gfx_is_mouse_button_down(struct gfx_window *window, enum gfx_mouse_button mouse_button)
{
	if (mouse_button >= GFX_MOUSE_BUTTON_LAST)
		return false;
	return window->mouse_buttons & (1 << mouse_button);
}

void gfx_window_properties_init(struct gfx_window_properties *properties)
{
	properties->stencil_bits = 8;
	properties->depth_bits = 24;
	properties->red_bits = 8;
	properties->green_bits = 8;
	properties->blue_bits = 8;
	properties->alpha_bits = 8;
}
