#ifndef GFX_WIN32_WINDOW_H
#define GFX_WIN32_WINDOW_H

#include "window.h"

#include <windows.h>

struct gfx_win32_window
{
	struct gfx_window *winref;
	HINSTANCE hinstance;
	HCURSOR cursor;
	HWND window;
	int32_t prev_mouse_x;
	int32_t prev_mouse_y;
	bool hidden_cursor;
	bool mouse_hover;
	char *classname;
	void (*on_resize)(struct gfx_window *winref);
};

bool gfx_win32_create_window(struct gfx_win32_window *window, const char *title, uint32_t width, uint32_t height);
void gfx_win32_ctr(struct gfx_win32_window *window, struct gfx_window *win_ref);
void gfx_win32_dtr(struct gfx_win32_window *window);
void gfx_win32_show(struct gfx_win32_window *window);
void gfx_win32_hide(struct gfx_win32_window *window);
void gfx_win32_set_title(struct gfx_win32_window *window, const char *title);
void gfx_win32_set_icon(struct gfx_win32_window *window, const void *data, uint32_t width, uint32_t height);
void gfx_win32_poll_events(struct gfx_win32_window *window);
void gfx_win32_wait_events(struct gfx_win32_window *window);
void gfx_win32_resize(struct gfx_win32_window *window, uint32_t width, uint32_t height);
void gfx_win32_grab_cursor(struct gfx_win32_window *window);
void gfx_win32_ungrab_cursor(struct gfx_win32_window *window);
char *gfx_win32_get_clipboard(struct gfx_win32_window *window);
void gfx_win32_set_clipboard(struct gfx_win32_window *window, const char *text);
gfx_cursor_t gfx_win32_create_native_cursor(struct gfx_win32_window *window, enum gfx_native_cursor cursor);
gfx_cursor_t gfx_win32_create_cursor(struct gfx_win32_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
void gfx_win32_delete_cursor(struct gfx_win32_window *window, gfx_cursor_t cursor);
void gfx_win32_set_cursor(struct gfx_win32_window *window, gfx_cursor_t cursor);
void gfx_win32_set_mouse_position(struct gfx_win32_window *window, int32_t x, int32_t y);

#endif
