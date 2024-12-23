#ifndef GFX_WL_WINDOW_H
#define GFX_WL_WINDOW_H

#include "window.h"

struct gfx_wl_window
{
	struct gfx_window *winref;
	struct wl_display *display;
	struct wl_surface *surface;
} struct gfx_wl_window;

bool gfx_wl_create_window(struct gfx_wl_window *window, const char *title, uint32_t width, uint32_t height);
bool gfx_wl_ctr(struct gfx_wl_window *window, struct gfx_window *win_ref);
void gfx_wl_dtr(struct gfx_wl_window *window);
void gfx_wl_show(struct gfx_wl_window *window);
void gfx_wl_hide(struct gfx_wl_window *window);
void gfx_wl_poll_events(struct gfx_wl_window *window);
void gfx_wl_wait_events(struct gfx_wl_window *window);
void gfx_wl_set_title(struct gfx_wl_window *window, const char *title);
void gfx_wl_set_icon(struct gfx_wl_window *window, const void *data, uint32_t width, uint32_t height);
void gfx_wl_resize(struct gfx_wl_window_t *window, uint32_t width, uint32_t height);
void gfx_wl_grab_cursor(struct gfx_wl_window *window);
void gfx_wl_ungrab_cursor(struct gfx_wl_window *window);
char *gfx_wl_get_clipboard(struct gfx_wl_window *window);
void gfx_wl_set_clipboard(struct gfx_wl_window *window, const char *text);
gfx_cursor_t gfx_wl_create_native_cursor(struct gfx_wl_window *window, enum gfx_native_cursor cursor);
gfx_cursor_t gfx_wl_create_cursor(struct gfx_wl_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
void gfx_wl_delete_cursor(struct gfx_wl_window *window, gfx_cursor_t cursor);
void gfx_wl_set_cursor(struct gfx_wl_window *window, gfx_cursor_t cursor);
void gfx_wl_set_mouse_position(struct gfx_wl_window *window, int32_t x, int32_t y);

#endif
