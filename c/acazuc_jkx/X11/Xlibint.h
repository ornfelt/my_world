#ifndef X11_XINT_H
#define X11_XINT_H

#include <X11/Xlib.h>

#include <xcb/xcb.h>

#define REQUEST(display, name, ...) \
	(display)->synchronize \
	? \
		xlib_handle_request_check((display), \
			xcb_##name##_checked((display)->conn, ##__VA_ARGS__)) \
	: \
		xlib_handle_request_async((display), \
			xcb_##name((display)->conn, ##__VA_ARGS__))

#define XID_REQ(display, xid, name, ...) \
	(display)->synchronize \
	? \
		xlib_handle_request_check_xid((display), (xid), \
			xcb_##name##_checked((display)->conn, ##__VA_ARGS__)) \
	: \
		xlib_handle_request_async_xid((display), (xid), \
			xcb_##name((display)->conn, ##__VA_ARGS__))

#define REPLY_REQ(display, name, ...) \
	xcb_##name##_cookie_t cookie = xcb_##name((display)->conn, ##__VA_ARGS__); \
	xcb_generic_error_t *error = NULL; \
	xcb_##name##_reply_t *reply = xcb_##name##_reply((display)->conn, cookie, \
	                                                 &error);

struct _Display
{
	xcb_connection_t *conn;
	Font cursor_font;
	XColor black;
	XColor white;
	int screen;
	int min_keycode;
	int max_keycode;
	KeySym *keysyms;
	int screens_count;
	Screen *screens;
	int keysyms_per_keycode;
	int synchronize;
	int (*error_handler)(Display *display, XErrorEvent *event);
};

struct _Screen
{
	xcb_screen_t *screen;
	Display *display;
	GC default_gc;
};

struct _Visual
{
	xcb_visualtype_t visual;
};

struct _GC
{
	Display *display;
	xcb_gcontext_t gc;
	uint32_t batch_mask;
	XGCValues batch_values;
};

void xlib_handle_error(Display *display, xcb_generic_error_t *error);
Status xlib_handle_request_check(Display *display, xcb_void_cookie_t cookie);
Status xlib_handle_request_async(Display *display, xcb_void_cookie_t cookie);
XID xlib_handle_request_check_xid(Display *display, XID xid,
                                  xcb_void_cookie_t cookie);
XID xlib_handle_request_async_xid(Display *display, XID xid,
                                  xcb_void_cookie_t cookie);
void xlib_print_error(Display *display, xcb_generic_error_t *error);
void xlib_copy_event(Display *display, XEvent *xe, xcb_generic_event_t *e);

#endif
