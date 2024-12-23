#ifndef GFX_X11_WINDOW_H
#define GFX_X11_WINDOW_H

#include "window.h"

#include <X11/extensions/XInput2.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

enum gfx_x11_atom
{
	X11_ATOM_DELETE,
	X11_ATOM_ICON,
	X11_ATOM_NAME,
	X11_ATOM_ICON_NAME,
	X11_ATOM_UTF8,
	X11_ATOM_PID,
	X11_ATOM_PING,
	X11_ATOM_WM_PROTOCOLS,
	X11_ATOM_CLIPBOARD,
	X11_ATOM_SELECTION,
	X11_ATOM_INCR,
	X11_ATOM_XSEL_DATA,
	X11_ATOM_TARGETS,
	X11_ATOM_LAST
};

struct gfx_x11_window
{
	struct gfx_window *winref;
	Display *display;
	Cursor blank_cursor;
	Cursor cursor;
	Window window;
	Window root;
	Atom atoms[X11_ATOM_LAST];
	XIC xic;
	XIM xim;
	int32_t prev_mouse_x;
	int32_t prev_mouse_y;
	char *clipboard;
	bool hidden_cursor;
};

bool gfx_x11_create_window(struct gfx_x11_window *window, const char *title, uint32_t width, uint32_t height, XVisualInfo *vi);
bool gfx_x11_ctr(struct gfx_x11_window *window, struct gfx_window *winref);
void gfx_x11_dtr(struct gfx_x11_window *window);
void gfx_x11_show(struct gfx_x11_window *window);
void gfx_x11_hide(struct gfx_x11_window *window);
void gfx_x11_poll_events(struct gfx_x11_window *window);
void gfx_x11_wait_events(struct gfx_x11_window *window);
void gfx_x11_set_title(struct gfx_x11_window *window, const char *title);
void gfx_x11_set_icon(struct gfx_x11_window *window, const void *data, uint32_t width, uint32_t height);
void gfx_x11_resize(struct gfx_x11_window *window, uint32_t width, uint32_t height);
void gfx_x11_grab_cursor(struct gfx_x11_window *window);
void gfx_x11_ungrab_cursor(struct gfx_x11_window *window);
char *gfx_x11_get_clipboard(struct gfx_x11_window *window);
void gfx_x11_set_clipboard(struct gfx_x11_window *window, const char *text);
gfx_cursor_t gfx_x11_create_native_cursor(struct gfx_x11_window *window, enum gfx_native_cursor cursor);
gfx_cursor_t gfx_x11_create_cursor(struct gfx_x11_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
void gfx_x11_delete_cursor(struct gfx_x11_window *window, gfx_cursor_t cursor);
void gfx_x11_set_cursor(struct gfx_x11_window *window, gfx_cursor_t cursor);
void gfx_x11_set_mouse_position(struct gfx_x11_window *window, int32_t x, int32_t y);

#endif
