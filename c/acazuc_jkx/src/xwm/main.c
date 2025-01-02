#include "xwm.h"

#include <X11/extensions/XShm.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define HEADER_HEIGHT         24
#define HEADER_BACKGROUND     0xD3D3D3
#define HEADER_FOREGROUND     0x8B8B8B
#define HEADER_HOVER          0xFFFFFF
#define BORDER_WIDTH          5
#define BUTTON_WIDTH          23
#define TASKBAR_HEIGHT        30
#define TASKBAR_BUTTON_HEIGHT 25
#define TASKBAR_BUTTON_WIDTH  200

#define BUTTON_CLOSE_X(win) ((win)->width + (win)->border_width + 2 - BUTTON_WIDTH - ((win)->border_width ? 0 : 2))
#define BUTTON_CLOSE_Y(win) ((win)->border_width ? 3 : 0)

#define BUTTON_MAXIMIZE_X(win) (BUTTON_CLOSE_X(win) - (BUTTON_WIDTH + 1))
#define BUTTON_MAXIMIZE_Y(win) ((win)->border_width ? 3 : 0)

#define BUTTON_ICONIFY_X(win) (BUTTON_MAXIMIZE_X(win) - (BUTTON_WIDTH + 1))
#define BUTTON_ICONIFY_Y(win) ((win)->border_width ? 3 : 0)

#if 0
#define RESIZE_DEBUG
#endif

uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_nsec + ts.tv_sec * 1000000000;
}

static struct window *get_window(struct xwm *xwm, Window win)
{
	struct window *window;
	TAILQ_FOREACH(window, &xwm->windows, chain)
	{
		if (window->child == win)
			return window;
	}
	return NULL;
}

static void taskbar_reposition_buttons(struct xwm *xwm)
{
	size_t i = 0;
	struct window *window;
	TAILQ_FOREACH(window, &xwm->windows, chain)
	{
		XMoveWindow(xwm->display, window->taskbar_button.window,
		            2 + i * 196, 5);
		++i;
	}
}

static void load_wm_name(struct xwm *xwm, struct window *window)
{
	XTextProperty *prop;
	if (!XGetWMName(xwm->display, window->child, &prop))
	{
		window->name = NULL;
		window->name_len = 0;
		return;
	}
	if (prop->value && prop->encoding == XA_STRING && prop->format == 8)
	{
		char *newname = strdup((char*)prop->value);
		if (newname)
		{
			free(window->name);
			window->name = newname;
			window->name_len = prop->nitems;
		}
	}
	free(prop->value);
	free(prop);
}

static void load_wm_icon_name(struct xwm *xwm, struct window *window)
{
	XTextProperty *prop;
	if (!XGetWMIconName(xwm->display, window->child, &prop))
	{
		window->name = NULL;
		window->name_len = 0;
		return;
	}
	if (prop->encoding == XA_STRING && prop->format == 8)
	{
		char *newname = strdup(window->name);
		if (newname)
		{
			free(window->name);
			window->name = newname;
			window->name_len = prop->nitems;
		}
	}
	free(prop->value);
	free(prop);
}

static void draw_header(struct xwm *xwm, struct window *window)
{
	struct asset *asset = &xwm->assets.header;
	for (size_t x = 0; x < window->width; x += asset->width)
	{
		XCopyArea(xwm->display, asset->pixmap, window->header,
		          xwm->bitmap_gc, 0, 0, asset->width, asset->height,
		          x, 0);
	}
	if (window->name)
		XDrawString(xwm->display, window->header,
		            xwm->header_text_gc,
		            2, 18, window->name, window->name_len);
}

static void draw_close(struct xwm *xwm, struct window *window)
{
	struct asset *asset;
	if (window->close.state & BUTTON_CLICKED)
		asset = &xwm->assets.close_clicked;
	else if (window->close.state & BUTTON_HOVERED)
		asset = &xwm->assets.close_hovered;
	else
		asset = &xwm->assets.close;
	XCopyArea(xwm->display, asset->pixmap, window->close.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
}

static void draw_maximize(struct xwm *xwm, struct window *window)
{
	struct asset *asset;
	if (window->maximize.state & BUTTON_CLICKED)
		asset = &xwm->assets.maximize_clicked;
	else if (window->maximize.state & BUTTON_HOVERED)
		asset = &xwm->assets.maximize_hovered;
	else
		asset = &xwm->assets.maximize;
	XCopyArea(xwm->display, asset->pixmap, window->maximize.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
}

static void draw_iconify(struct xwm *xwm, struct window *window)
{
	struct asset *asset;
	if (window->iconify.state & BUTTON_CLICKED)
		asset = &xwm->assets.iconify_clicked;
	else if (window->iconify.state & BUTTON_HOVERED)
		asset = &xwm->assets.iconify_hovered;
	else
		asset = &xwm->assets.iconify;
	XCopyArea(xwm->display, asset->pixmap, window->iconify.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
}

static void draw_taskbar_button(struct xwm *xwm, struct window *window)
{
	struct asset *asset;
	if (!(window->flags & WINDOW_MAPPED))
		asset = &xwm->assets.taskbar_button_disabled;
	else if (window->taskbar_button.state & BUTTON_HOVERED)
		asset = &xwm->assets.taskbar_button_hovered;
	else
		asset = &xwm->assets.taskbar_button;
	for (size_t x = 0; x < TASKBAR_BUTTON_WIDTH; x += asset->width)
	{
		XCopyArea(xwm->display, asset->pixmap,
		          window->taskbar_button.window, xwm->bitmap_gc,
		          0, 0, asset->width, asset->height, x, 0);
	}
	asset = &xwm->assets.taskbar_button_l;
	XCopyArea(xwm->display, asset->pixmap, window->taskbar_button.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
	asset = &xwm->assets.taskbar_button_r;
	XCopyArea(xwm->display, asset->pixmap, window->taskbar_button.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height,
	          TASKBAR_BUTTON_WIDTH - asset->width, 0);
	if (window->icon_name)
		XDrawString(xwm->display, window->taskbar_button.window,
		            xwm->taskbar_text_gc, 7, 21, window->icon_name,
		            window->icon_name_len);
	else if (window->name)
		XDrawString(xwm->display, window->taskbar_button.window,
		            xwm->taskbar_text_gc, 7, 21, window->name,
		            window->name_len);
}

static void draw_resize_tl(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFF0000FF);
	XFillRectangle(xwm->display, window->resize_tl.window, xwm->resize_gc,
	               0, 0, window->border_width, window->border_width);
#else
	struct asset *asset = &xwm->assets.resize_tl;
	XCopyArea(xwm->display, asset->pixmap, window->resize_tl.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
#endif
}

static void draw_resize_tr(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFFFFFF00);
	XFillRectangle(xwm->display, window->resize_tr.window, xwm->resize_gc,
	               0, 0, window->border_width, window->border_width);
#else
	struct asset *asset = &xwm->assets.resize_tr;
	XCopyArea(xwm->display, asset->pixmap, window->resize_tr.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
#endif
}

static void draw_resize_bl(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFFFF0000);
	XFillRectangle(xwm->display, window->resize_bl.window, xwm->resize_gc,
	               0, 0, window->border_width, window->border_width);
#else
	struct asset *asset = &xwm->assets.resize_bl;
	XCopyArea(xwm->display, asset->pixmap, window->resize_bl.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
#endif
}

static void draw_resize_br(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFF00FF00);
	XFillRectangle(xwm->display, window->resize_br.window, xwm->resize_gc,
	               0, 0, window->border_width, window->border_width);
#else
	struct asset *asset = &xwm->assets.resize_br;
	XCopyArea(xwm->display, asset->pixmap, window->resize_br.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
#endif
}

static void draw_resize_l(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFF00FFFF);
	XFillRectangle(xwm->display, window->resize_l.window, xwm->resize_gc,
	               0, 0, window->border_width, window->height + HEADER_HEIGHT);
#else
	struct asset *asset = &xwm->assets.header_resize_l;
	XCopyArea(xwm->display, asset->pixmap, window->resize_l.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
	asset = &xwm->assets.resize_l;
	for (size_t y = xwm->assets.header_resize_l.height; y < window->height + HEADER_HEIGHT; y += asset->height)
	{
		XCopyArea(xwm->display, asset->pixmap, window->resize_l.window,
		          xwm->bitmap_gc, 0, 0, asset->width, asset->height,
		          0, y);
	}
#endif
}

static void draw_resize_r(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFFFF00FF);
	XFillRectangle(xwm->display, window->resize_r.window, xwm->resize_gc,
	               0, 0, window->border_width, window->height + HEADER_HEIGHT);
#else
	struct asset *asset = &xwm->assets.header_resize_r;
	XCopyArea(xwm->display, asset->pixmap, window->resize_r.window,
	          xwm->bitmap_gc, 0, 0, asset->width, asset->height, 0, 0);
	asset = &xwm->assets.resize_r;
	for (size_t y = xwm->assets.header_resize_r.height; y < window->height + HEADER_HEIGHT; y += asset->height)
	{
		XCopyArea(xwm->display, asset->pixmap, window->resize_r.window,
		          xwm->bitmap_gc, 0, 0, asset->width, asset->height,
		          0, y);
	}
#endif
}

static void draw_resize_t(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFF80FF80);
	XFillRectangle(xwm->display, window->resize_t.window, xwm->resize_gc,
	               0, 0, window->width, window->border_width);
#else
	struct asset *asset = &xwm->assets.resize_t;
	for (size_t x = 0; x < window->width; x += asset->width)
	{
		XCopyArea(xwm->display, asset->pixmap, window->resize_t.window,
		          xwm->bitmap_gc, 0, 0, asset->width, asset->height,
		          x, 0);
	}
#endif
}

static void draw_resize_b(struct xwm *xwm, struct window *window)
{
#ifdef RESIZE_DEBUG
	XSetForeground(xwm->display, xwm->resize_gc, 0xFFFFFF80);
	XFillRectangle(xwm->display, window->resize_b.window, xwm->resize_gc,
	               0, 0, window->width, window->border_width);
#else
	struct asset *asset = &xwm->assets.resize_b;
	for (size_t x = 0; x < window->width; x += asset->width)
	{
		XCopyArea(xwm->display, asset->pixmap, window->resize_b.window,
		          xwm->bitmap_gc, 0, 0, asset->width, asset->height,
		          x, 0);
	}
#endif
}

static int create_header(struct xwm *xwm, struct window *window,
                         XWindowAttributes *attributes)
{
	XSetWindowAttributes swa;
	swa.event_mask = EnterWindowMask
	               | LeaveWindowMask
	               | ButtonPressMask
	               | ButtonReleaseMask
	               | PointerMotionMask;
	swa.bit_gravity = NorthWestGravity;
	unsigned swa_mask = CWEventMask | CWBitGravity;
	window->header = XCreateWindow(xwm->display, window->parent,
	                               BORDER_WIDTH, BORDER_WIDTH,
	                               window->width,
	                               HEADER_HEIGHT, 0,
	                               attributes->depth, InputOutput,
	                               attributes->visual, swa_mask, &swa);
	if (!window->header)
	{
		fprintf(stderr, "%s: failed to create header window\n",
		        xwm->progname);
		return 1;
	}
	draw_header(xwm, window);
	/*XGrabButton(xwm->display, Button1, AnyModifier, window->header, False,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);*/
	XMapWindow(xwm->display, window->header);
	return 0;
}

static int create_close(struct xwm *xwm, struct window *window,
                        XWindowAttributes *attributes)
{
	XSetWindowAttributes swa;
	swa.event_mask = EnterWindowMask
	               | LeaveWindowMask
	               | ButtonPressMask
	               | ButtonReleaseMask;
	swa.cursor = xwm->cursors.hand1;
	unsigned swa_mask = CWEventMask | CWCursor;
	window->close.window = XCreateWindow(xwm->display, window->parent,
	                                     BUTTON_CLOSE_X(window),
	                                     BUTTON_CLOSE_Y(window),
	                                     BUTTON_WIDTH, BUTTON_WIDTH,
	                                     0, attributes->depth,
	                                     InputOutput, attributes->visual,
	                                     swa_mask, &swa);
	if (!window->close.window)
	{
		fprintf(stderr, "%s: failed to create close window\n",
		        xwm->progname);
		return 1;
	}
	draw_close(xwm, window);
	XMapRaised(xwm->display, window->close.window);
	return 0;
}

static int create_maximize(struct xwm *xwm, struct window *window,
                           XWindowAttributes *attributes)
{
	XSetWindowAttributes swa;
	swa.event_mask = EnterWindowMask
	               | LeaveWindowMask
	               | ButtonPressMask
	               | ButtonReleaseMask;
	swa.cursor = xwm->cursors.hand1;
	unsigned swa_mask = CWEventMask | CWCursor;
	window->maximize.window = XCreateWindow(xwm->display, window->parent,
	                                        BUTTON_MAXIMIZE_X(window),
	                                        BUTTON_MAXIMIZE_Y(window),
	                                        BUTTON_WIDTH, BUTTON_WIDTH,
	                                        0, attributes->depth,
	                                        InputOutput, attributes->visual,
	                                        swa_mask, &swa);
	if (!window->maximize.window)
	{
		fprintf(stderr, "%s: failed to create maximize window\n",
		        xwm->progname);
		return 1;
	}
	draw_maximize(xwm, window);
	XMapRaised(xwm->display, window->maximize.window);
	return 0;
}

static int create_iconify(struct xwm *xwm, struct window *window,
                          XWindowAttributes *attributes)
{
	XSetWindowAttributes swa;
	swa.event_mask = EnterWindowMask
	               | LeaveWindowMask
	               | ButtonPressMask
	               | ButtonReleaseMask;
	swa.cursor = xwm->cursors.hand1;
	unsigned swa_mask = CWEventMask | CWCursor;
	window->iconify.window = XCreateWindow(xwm->display, window->parent,
	                                       BUTTON_ICONIFY_X(window),
	                                       BUTTON_ICONIFY_Y(window),
	                                       BUTTON_WIDTH, BUTTON_WIDTH,
	                                       0, attributes->depth,
	                                       InputOutput, attributes->visual,
	                                       swa_mask, &swa);
	if (!window->iconify.window)
	{
		fprintf(stderr, "%s: failed to create iconify window\n",
		        xwm->progname);
		return 1;
	}
	draw_iconify(xwm, window);
	XMapRaised(xwm->display, window->iconify.window);
	return 0;
}

static int create_taskbar_button(struct xwm *xwm, struct window *window)
{
	XSetWindowAttributes swa;
	swa.event_mask = EnterWindowMask
	               | LeaveWindowMask
	               | ButtonPressMask
	               | ButtonReleaseMask;
	swa.cursor = xwm->cursors.hand1;
	unsigned swa_mask = CWEventMask | CWCursor;
	size_t i = 0;
	struct window *it;
	TAILQ_FOREACH(it, &xwm->windows, chain)
		++i;
	window->taskbar_button.window = XCreateWindow(xwm->display, xwm->taskbar,
	                                              2 + i * TASKBAR_BUTTON_WIDTH, 3,
	                                              TASKBAR_BUTTON_WIDTH,
	                                              TASKBAR_BUTTON_HEIGHT,
	                                              0, xwm->vi.depth,
	                                              InputOutput,
	                                              xwm->vi.visual,
	                                              swa_mask, &swa);
	if (!window->taskbar_button.window)
	{
		fprintf(stderr, "%s: failed to create taskbar button window\n",
		        xwm->progname);
		return 1;
	}
	draw_taskbar_button(xwm, window);
	XMapWindow(xwm->display, window->taskbar_button.window);
	return 0;
}

static int create_resizes(struct xwm *xwm, struct window *window)
{
	XSetWindowAttributes swa;
	swa.event_mask = EnterWindowMask
	               | LeaveWindowMask
	               | ButtonPressMask
	               | ButtonReleaseMask
	               | PointerMotionMask;
	swa.bit_gravity = NorthWestGravity;
	unsigned swa_mask = CWEventMask | CWCursor | CWBitGravity;

#define CORNER_RESIZE(name, cur, x, y) \
	swa.cursor = xwm->cursors.cur; \
	window->resize_##name.window = XCreateWindow(xwm->display, \
	                                             window->parent, \
	                                             x, y, BORDER_WIDTH, \
	                                             BORDER_WIDTH, 0, \
	                                             xwm->vi.depth, \
	                                             InputOutput, \
	                                             xwm->vi.visual, \
	                                             swa_mask, &swa); \
	if (!window->resize_##name.window) \
	{ \
		fprintf(stderr, "%s: failed to create resize " #name " window\n", \
		        xwm->progname); \
		return 1; \
	} \
	draw_resize_##name(xwm, window); \
	XMapWindow(xwm->display, window->resize_##name.window);

	CORNER_RESIZE(tl, top_left_corner,
	              0,
	              0);
	CORNER_RESIZE(tr, top_right_corner,
	              window->width + window->border_width,
	              0);
	CORNER_RESIZE(bl, bottom_left_corner,
	              0,
	              window->height + HEADER_HEIGHT + window->border_width);
	CORNER_RESIZE(br, bottom_right_corner,
	              window->width + window->border_width,
	              window->height + HEADER_HEIGHT + window->border_width);

#undef CORNER_RESIZE

#define SIDE_RESIZE(name, cur, x, y, width, height) \
	swa.cursor = xwm->cursors.cur; \
	window->resize_##name.window = XCreateWindow(xwm->display, \
	                                             window->parent, \
	                                             x, y, width, height, 0, \
	                                             xwm->vi.depth, \
	                                             InputOutput, \
	                                             xwm->vi.visual, \
	                                             swa_mask, &swa); \
	if (!window->resize_##name.window) \
	{ \
		fprintf(stderr, "%s: failed to create resize " #name " window\n", \
		        xwm->progname); \
		return 1; \
	} \
	draw_resize_##name(xwm, window); \
	XMapWindow(xwm->display, window->resize_##name.window);

	SIDE_RESIZE(l, left_side,
	            0,
	            window->border_width,
	            window->border_width,
	            window->height + HEADER_HEIGHT);
	SIDE_RESIZE(r, right_side,
	            window->width + window->border_width,
	            window->border_width,
	            window->border_width,
	            window->height + HEADER_HEIGHT);
	SIDE_RESIZE(t, top_side,
	            window->border_width,
	            0,
	            window->width,
	            window->border_width);
	SIDE_RESIZE(b, bottom_side,
	            window->border_width,
	            window->height + HEADER_HEIGHT + window->border_width,
	            window->width,
	            window->border_width);

#undef SIDE_RESIZE

	return 0;
}

static void map_window(struct xwm *xwm, struct window *window)
{
	XMapWindow(xwm->display, window->parent);
	window->flags |= WINDOW_MAPPED;
	draw_taskbar_button(xwm, window);
}

static void unmap_window(struct xwm *xwm, struct window *window)
{
	XUnmapWindow(xwm->display, window->parent);
	window->flags &= ~WINDOW_MAPPED;
	draw_taskbar_button(xwm, window);
}

static void handle_create_notify(struct xwm *xwm, XCreateWindowEvent *event)
{
	struct window *window;
	if (event->parent != xwm->root || !xwm->enable_reframe)
		return;
	TAILQ_FOREACH(window, &xwm->windows, chain)
	{
		if (window->parent == event->window)
			return;
	}
	XWindowAttributes attributes;
	if (!XGetWindowAttributes(xwm->display, event->window, &attributes))
	{
		fprintf(stderr, "%s: failed to get window attributes\n",
		        xwm->progname);
		return;
	}
	if (attributes.class != InputOutput)
		return;
	if (attributes.override_redirect)
		return;
	window = calloc(sizeof(*window), 1);
	if (!window)
	{
		fprintf(stderr, "%s: malloc: %s\n", xwm->progname,
		        strerror(errno));
		return;
	}
	window->child = event->window;
	window->width = attributes.width;
	window->height = attributes.height;
	window->border_width = BORDER_WIDTH;
	XSetWindowAttributes swa;
	swa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask;
	swa.cursor = xwm->cursors.left_ptr;
	unsigned swa_mask = CWEventMask | CWCursor;
	window->parent = XCreateWindow(xwm->display, xwm->container,
	                               attributes.x, attributes.y,
	                               window->width + window->border_width * 2,
	                               window->height + HEADER_HEIGHT + window->border_width * 2,
	                               0, attributes.depth,
	                               InputOutput, attributes.visual,
	                               swa_mask, &swa);
	if (!window->parent)
	{
		fprintf(stderr, "%s: failed to create parent window\n",
		        xwm->progname);
		goto err;
	}
	load_wm_name(xwm, window);
	load_wm_icon_name(xwm, window);
	if (create_header(xwm, window, &attributes)
	 || create_close(xwm, window, &attributes)
	 || create_maximize(xwm, window, &attributes)
	 || create_iconify(xwm, window, &attributes)
	 || create_taskbar_button(xwm, window)
	 || create_resizes(xwm, window))
		goto err;
	if (attributes.border_width)
		XSetWindowBorderWidth(xwm->display, window->child, 0);
	XReparentWindow(xwm->display, window->child, window->parent,
	                window->border_width, window->border_width + HEADER_HEIGHT);
	XSelectInput(xwm->display, window->child, PropertyChangeMask);
	map_window(xwm, window);
	TAILQ_INSERT_TAIL(&xwm->windows, window, chain);
	return;

err:
	if (window)
	{
		if (window->resize_tl.window)
			XDestroyWindow(xwm->display, window->resize_tl.window);
		if (window->resize_tr.window)
			XDestroyWindow(xwm->display, window->resize_tr.window);
		if (window->resize_bl.window)
			XDestroyWindow(xwm->display, window->resize_bl.window);
		if (window->resize_br.window)
			XDestroyWindow(xwm->display, window->resize_br.window);
		if (window->resize_l.window)
			XDestroyWindow(xwm->display, window->resize_l.window);
		if (window->resize_r.window)
			XDestroyWindow(xwm->display, window->resize_r.window);
		if (window->resize_t.window)
			XDestroyWindow(xwm->display, window->resize_t.window);
		if (window->resize_b.window)
			XDestroyWindow(xwm->display, window->resize_b.window);
		if (window->taskbar_button.window)
			XDestroyWindow(xwm->display, window->taskbar_button.window);
		if (window->close.window)
			XDestroyWindow(xwm->display, window->close.window);
		if (window->maximize.window)
			XDestroyWindow(xwm->display, window->maximize.window);
		if (window->iconify.window)
			XDestroyWindow(xwm->display, window->iconify.window);
		if (window->header)
			XDestroyWindow(xwm->display, window->header);
		if (window->parent)
			XDestroyWindow(xwm->display, window->parent);
		free(window->icon_name);
		free(window->name);
		free(window);
	}
}

static void handle_destroy_notify(struct xwm *xwm, XDestroyWindowEvent *event)
{
	struct window *window = get_window(xwm, event->window);
	if (!window)
		return;
	TAILQ_REMOVE(&xwm->windows, window, chain);
	if (window->resize_tl.window)
		XDestroyWindow(xwm->display, window->resize_tl.window);
	if (window->resize_tr.window)
		XDestroyWindow(xwm->display, window->resize_tr.window);
	if (window->resize_bl.window)
		XDestroyWindow(xwm->display, window->resize_bl.window);
	if (window->resize_br.window)
		XDestroyWindow(xwm->display, window->resize_br.window);
	if (window->resize_l.window)
		XDestroyWindow(xwm->display, window->resize_l.window);
	if (window->resize_r.window)
		XDestroyWindow(xwm->display, window->resize_r.window);
	if (window->resize_t.window)
		XDestroyWindow(xwm->display, window->resize_t.window);
	if (window->resize_b.window)
		XDestroyWindow(xwm->display, window->resize_b.window);
	if (window->taskbar_button.window)
		XDestroyWindow(xwm->display, window->taskbar_button.window);
	if (window->close.window)
		XDestroyWindow(xwm->display, window->close.window);
	if (window->maximize.window)
		XDestroyWindow(xwm->display, window->maximize.window);
	if (window->iconify.window)
		XDestroyWindow(xwm->display, window->iconify.window);
	if (window->header)
		XDestroyWindow(xwm->display, window->header);
	if (window->parent)
		XDestroyWindow(xwm->display, window->parent);
	free(window->icon_name);
	free(window->name);
	free(window);
	taskbar_reposition_buttons(xwm);
}

static void handle_map_request(struct xwm *xwm, XMapRequestEvent *event)
{
	struct window *window = get_window(xwm, event->window);
	if (!window)
	{
		XMapWindow(xwm->display, event->window);
		return;
	}
	XMapWindow(xwm->display, event->window);
	map_window(xwm, window);
	XSetInputFocus(xwm->display, window->child, RevertToPointerRoot,
	               CurrentTime);
}

static void handle_map_notify(struct xwm *xwm, XMapEvent *event)
{
	struct window *window = get_window(xwm, event->window);
	if (window)
	{
		window->flags |= WINDOW_MAPPED;
		draw_taskbar_button(xwm, window);
	}
}

static void handle_unmap_notify(struct xwm *xwm, XUnmapEvent *event)
{
	struct window *window = get_window(xwm, event->window);
	if (window)
		unmap_window(xwm, window);
}

static void resize_window(struct xwm *xwm, struct window *window,
                          uint32_t width, uint32_t height)
{
	window->width = width;
	window->height = height;
	XResizeWindow(xwm->display, window->header, width, HEADER_HEIGHT);
	XMoveWindow(xwm->display, window->close.window,
	            BUTTON_CLOSE_X(window),
	            BUTTON_CLOSE_Y(window));
	XMoveWindow(xwm->display, window->maximize.window,
	            BUTTON_MAXIMIZE_X(window),
	            BUTTON_MAXIMIZE_Y(window));
	XMoveWindow(xwm->display, window->iconify.window,
	            BUTTON_ICONIFY_X(window),
	            BUTTON_ICONIFY_Y(window));
	XMoveWindow(xwm->display, window->resize_tr.window,
	            width + window->border_width,
	            0);
	XMoveWindow(xwm->display, window->resize_bl.window,
	            0,
	            height + HEADER_HEIGHT + window->border_width);
	XMoveWindow(xwm->display, window->resize_br.window,
	            width + window->border_width,
	            height + HEADER_HEIGHT + window->border_width);
	XMoveWindow(xwm->display, window->resize_r.window,
	            width + window->border_width,
	            window->border_width);
	XMoveWindow(xwm->display, window->resize_b.window,
	            window->border_width,
	            height + HEADER_HEIGHT + window->border_width);
	XResizeWindow(xwm->display, window->resize_l.window,
	              window->border_width,
	              height + HEADER_HEIGHT);
	XResizeWindow(xwm->display, window->resize_r.window,
	              window->border_width,
	              height + HEADER_HEIGHT);
	XResizeWindow(xwm->display, window->resize_t.window,
	              width,
	              window->border_width);
	XResizeWindow(xwm->display, window->resize_b.window,
	              width,
	              window->border_width);
	draw_header(xwm, window);
	draw_resize_tl(xwm, window);
	draw_resize_tr(xwm, window);
	draw_resize_bl(xwm, window);
	draw_resize_br(xwm, window);
	draw_resize_l(xwm, window);
	draw_resize_r(xwm, window);
	draw_resize_t(xwm, window);
	draw_resize_b(xwm, window);
	XResizeWindow(xwm->display, window->parent,
	              width + window->border_width * 2,
	              height + HEADER_HEIGHT + window->border_width * 2);
	XResizeWindow(xwm->display, window->child, width, height);
}

static void move_window(struct xwm *xwm, struct window *window,
                        int32_t x, int32_t y)
{
	window->x = x;
	window->y = y;
	XMoveWindow(xwm->display, window->parent, x, y);
}

static void maximize_window(struct xwm *xwm, struct window *window)
{
	XSetWindowBorderWidth(xwm->display, window->parent, 0);
	window->prev_x = window->x;
	window->prev_y = window->y;
	window->prev_width = window->width;
	window->prev_height = window->height;
	window->border_width = 0;
	XMoveWindow(xwm->display, window->child, 0, HEADER_HEIGHT);
	XMoveWindow(xwm->display, window->header, 0, 0);
	window->flags |= WINDOW_MAXIMIZED;
	move_window(xwm, window, 0, 0);
	resize_window(xwm, window, xwm->screen_width,
	              xwm->screen_height - HEADER_HEIGHT - TASKBAR_HEIGHT);
}

static void unmaximize_window(struct xwm *xwm, struct window *window)
{
	window->flags &= ~WINDOW_MAXIMIZED;
	window->border_width = BORDER_WIDTH;
	XMoveWindow(xwm->display, window->child,
	            window->border_width,
	            window->border_width + HEADER_HEIGHT);
	XMoveWindow(xwm->display, window->header,
	            window->border_width,
	            window->border_width);
	move_window(xwm, window, window->prev_x, window->prev_y);
	resize_window(xwm, window, window->prev_width, window->prev_height);
}

static void handle_configure_request(struct xwm *xwm,
                                     XConfigureRequestEvent *event)
{
	struct window *window = get_window(xwm, event->window);
	if (!window)
	{
		XWindowChanges values;
		values.x = event->x;
		values.y = event->y;
		values.width = event->width;
		values.height = event->height;
		values.border_width = event->border_width;
		values.sibling = event->above;
		values.stack_mode = event->detail;
		XConfigureWindow(xwm->display, event->window,
		                 event->value_mask, &values);
		return;
	}
	XWindowChanges parent_changes;
	XWindowChanges child_changes;
	uint32_t parent_value_mask = 0;
	uint32_t child_value_mask = 0;
	if (event->value_mask & CWX)
	{
		parent_value_mask |= CWX;
		parent_changes.x = event->x;
		window->x = event->x;
	}
	if (event->value_mask & CWY)
	{
		parent_value_mask |= CWY;
		parent_changes.y = event->y;
		window->y = event->y;
	}
	if (event->value_mask & (CWWidth | CWHeight))
	{
		uint32_t width;
		uint32_t height;
		if (event->value_mask & CWWidth)
			width = event->width;
		else
			width = window->width;
		if (event->value_mask & CWHeight)
			height = event->height;
		else
			height = window->height;
		resize_window(xwm, window, width, height);
	}
	if (event->value_mask & CWSibling)
	{
		parent_value_mask |= CWSibling;
		parent_changes.sibling = event->above;
	}
	if (event->value_mask & CWStackMode)
	{
		parent_value_mask |= CWStackMode;
		parent_changes.stack_mode = event->detail;
	}
	XConfigureWindow(xwm->display, window->parent, parent_value_mask,
	                 &parent_changes);
	XConfigureWindow(xwm->display, window->child, child_value_mask,
	                 &child_changes);
}

static void handle_enter_notify(struct xwm *xwm, XCrossingEvent *event)
{
	struct window *window;
	TAILQ_FOREACH(window, &xwm->windows, chain)
	{

#define TEST_BUTTON_HOVERED(name, redraw) \
		if (event->window == window->name.window) \
		{ \
			window->name.state |= BUTTON_HOVERED; \
			if (redraw) \
				draw_##name(xwm, window); \
			break; \
		}

		TEST_BUTTON_HOVERED(close, 1);
		TEST_BUTTON_HOVERED(maximize, 1);
		TEST_BUTTON_HOVERED(iconify, 1);
		TEST_BUTTON_HOVERED(taskbar_button, 1);
		TEST_BUTTON_HOVERED(resize_tl, 0);
		TEST_BUTTON_HOVERED(resize_tr, 0);
		TEST_BUTTON_HOVERED(resize_bl, 0);
		TEST_BUTTON_HOVERED(resize_br, 0);
		TEST_BUTTON_HOVERED(resize_l, 0);
		TEST_BUTTON_HOVERED(resize_r, 0);
		TEST_BUTTON_HOVERED(resize_t, 0);
		TEST_BUTTON_HOVERED(resize_b, 0);

#undef TEST_BUTTON_HOVERED

	}
}

static void handle_leave_notify(struct xwm *xwm, XCrossingEvent *event)
{
	struct window *window;
	TAILQ_FOREACH(window, &xwm->windows, chain)
	{

#define TEST_BUTTON_HOVERED(name, redraw) \
		if (event->window == window->name.window) \
		{ \
			window->name.state &= ~BUTTON_HOVERED; \
			if (redraw) \
				draw_##name(xwm, window); \
			break; \
		}

		TEST_BUTTON_HOVERED(close, 1);
		TEST_BUTTON_HOVERED(maximize, 1);
		TEST_BUTTON_HOVERED(iconify, 1);
		TEST_BUTTON_HOVERED(taskbar_button, 1);
		TEST_BUTTON_HOVERED(resize_tl, 0);
		TEST_BUTTON_HOVERED(resize_tr, 0);
		TEST_BUTTON_HOVERED(resize_bl, 0);
		TEST_BUTTON_HOVERED(resize_br, 0);
		TEST_BUTTON_HOVERED(resize_l, 0);
		TEST_BUTTON_HOVERED(resize_r, 0);
		TEST_BUTTON_HOVERED(resize_t, 0);
		TEST_BUTTON_HOVERED(resize_b, 0);

#undef TEST_BUTTON_HOVERED

	}
}

static void handle_button_press(struct xwm *xwm, XButtonEvent *event)
{
	struct window *window;
	TAILQ_FOREACH(window, &xwm->windows, chain)
	{

#define TEST_BUTTON_CLICKED(name, set_drag, redraw) \
		if (event->window == window->name.window) \
		{ \
			window->name.state |= BUTTON_CLICKED; \
			if (redraw) \
				draw_##name(xwm, window); \
			if (XGrabPointer(xwm->display, event->window, False, \
			                 ButtonPressMask \
			               | ButtonReleaseMask \
			               | PointerMotionMask, \
			                 GrabModeAsync, GrabModeAsync, None, \
			                 None, event->time)) \
			{ \
				fprintf(stderr, "failed to grab pointer\n"); \
				break; \
			} \
			if (set_drag) \
			{ \
				XRaiseWindow(xwm->display, window->parent); \
				xwm->drag = window; \
				xwm->drag_x = event->x; \
				xwm->drag_y = event->y; \
			} \
			break; \
		}

		TEST_BUTTON_CLICKED(close, 0, 1);
		TEST_BUTTON_CLICKED(maximize, 0, 1);
		TEST_BUTTON_CLICKED(iconify, 0, 1);
		TEST_BUTTON_CLICKED(taskbar_button, 0, 1);
		TEST_BUTTON_CLICKED(resize_tl, 1, 0);
		TEST_BUTTON_CLICKED(resize_tr, 1, 0);
		TEST_BUTTON_CLICKED(resize_bl, 1, 0);
		TEST_BUTTON_CLICKED(resize_br, 1, 0);
		TEST_BUTTON_CLICKED(resize_l, 1, 0);
		TEST_BUTTON_CLICKED(resize_r, 1, 0);
		TEST_BUTTON_CLICKED(resize_t, 1, 0);
		TEST_BUTTON_CLICKED(resize_b, 1, 0);

#undef TEST_BUTTON_CLICKED

		if (event->window == window->header)
		{
			if (XGrabPointer(xwm->display, event->window, False,
			                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			                 GrabModeAsync, GrabModeAsync, None, None,
			                 event->time))
			{
				fprintf(stderr, "failed to grab pointer\n");
				break;
			}
			XRaiseWindow(xwm->display, window->parent);
			xwm->drag = window;
			xwm->drag_x = event->x;
			xwm->drag_y = event->y;
			XDefineCursor(xwm->display, window->header,
			              xwm->cursors.fleur);
			break;
		}
	}
}

static void handle_button_release(struct xwm *xwm, XButtonEvent *event)
{
	XUngrabPointer(xwm->display, event->time); /* XXX more smart */
	if (xwm->drag)
	{
		if (event->window == xwm->drag->header)
			XDefineCursor(xwm->display, event->window,
			              xwm->cursors.left_ptr);
		xwm->drag = NULL;
	}
	struct window *window;
	TAILQ_FOREACH(window, &xwm->windows, chain)
	{
		if (event->window == window->close.window
		 && (window->close.state & (BUTTON_HOVERED | BUTTON_CLICKED)) == (BUTTON_HOVERED | BUTTON_CLICKED))
		{
			/* XXX */
		}
		if (event->window == window->maximize.window
		 && (window->maximize.state & (BUTTON_HOVERED | BUTTON_CLICKED)) == (BUTTON_HOVERED | BUTTON_CLICKED))
		{
			if (window->flags & WINDOW_MAXIMIZED)
				unmaximize_window(xwm, window);
			else
				maximize_window(xwm, window);
		}
		if (event->window == window->iconify.window
		 && (window->iconify.state & (BUTTON_HOVERED | BUTTON_CLICKED)) == (BUTTON_HOVERED | BUTTON_CLICKED))
		{
			unmap_window(xwm, window);
		}
		if (event->window == window->taskbar_button.window
		 && (window->taskbar_button.state & (BUTTON_HOVERED | BUTTON_CLICKED)) == (BUTTON_HOVERED | BUTTON_CLICKED))
		{
			if (window->flags & WINDOW_MAPPED)
				unmap_window(xwm, window);
			else
				map_window(xwm, window);
		}

#define TEST_BUTTON_CLICKED(name, redraw) \
		if (window->name.state & BUTTON_CLICKED) \
		{ \
			window->name.state &= ~BUTTON_CLICKED; \
			if (redraw) \
				draw_##name(xwm, window); \
		}

		TEST_BUTTON_CLICKED(close, 1);
		TEST_BUTTON_CLICKED(maximize, 1);
		TEST_BUTTON_CLICKED(iconify, 1);
		TEST_BUTTON_CLICKED(taskbar_button, 1);
		TEST_BUTTON_CLICKED(resize_tl, 0);
		TEST_BUTTON_CLICKED(resize_tr, 0);
		TEST_BUTTON_CLICKED(resize_bl, 0);
		TEST_BUTTON_CLICKED(resize_br, 0);
		TEST_BUTTON_CLICKED(resize_l, 0);
		TEST_BUTTON_CLICKED(resize_r, 0);
		TEST_BUTTON_CLICKED(resize_t, 0);
		TEST_BUTTON_CLICKED(resize_b, 0);

#undef TEST_BUTTON_CLICKED

	}
}

static void handle_header_drag(struct xwm *xwm, XMotionEvent *event)
{
	if (xwm->drag->flags & WINDOW_MAXIMIZED)
		unmaximize_window(xwm, xwm->drag);
	move_window(xwm, xwm->drag,
	            event->x_root - xwm->drag_x - xwm->drag->border_width,
	            event->y_root - xwm->drag_y - xwm->drag->border_width);
}

static void handle_resize_drag(struct xwm *xwm, XMotionEvent *event,
                               enum resize_x_type resize_x,
                               enum resize_y_type resize_y)
{
	int32_t new_x;
	int32_t new_y;
	int32_t new_width;
	int32_t new_height;
	switch (resize_x)
	{
		case RESIZE_X_NONE:
			new_x = xwm->drag->x;
			new_width = xwm->drag->width;
			break;
		case RESIZE_X_LEFT:
			new_x = event->x_root - xwm->drag_x;
			new_width = xwm->drag->width + xwm->drag->x - new_x;
			if (new_width < 0)
			{
				new_x += new_width;
				new_width = 0;
			}
			break;
		case RESIZE_X_RIGHT:
			new_x = xwm->drag->x;
			new_width = event->x_root - xwm->drag_x - xwm->drag->x - xwm->drag->border_width;
			if (new_width < 0)
				new_width = 0;
			break;
	}
	switch (resize_y)
	{
		case RESIZE_Y_NONE:
			new_y = xwm->drag->y;
			new_height = xwm->drag->height;
			break;
		case RESIZE_Y_TOP:
			new_y = event->y_root - xwm->drag_y;
			new_height = xwm->drag->height + xwm->drag->y - new_y;
			if (new_height < 0)
			{
				new_y += new_height;
				new_height = 0;
			}
			break;
		case RESIZE_Y_BOTTOM:
			new_y = xwm->drag->y;
			new_height = event->y_root - xwm->drag_y - xwm->drag->y - HEADER_HEIGHT - xwm->drag->border_width;
			if (new_height < 0)
				new_height = 0;
			break;
	}
	resize_window(xwm, xwm->drag, new_width, new_height);
	move_window(xwm, xwm->drag, new_x, new_y);
}

static void handle_motion_notify(struct xwm *xwm, XMotionEvent *event)
{
	if (xwm->drag)
	{
		if (event->window == xwm->drag->header)
		{
			handle_header_drag(xwm, event);
			return;
		}
		if (event->window == xwm->drag->resize_tl.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_LEFT, RESIZE_Y_TOP);
			return;
		}
		if (event->window == xwm->drag->resize_tr.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_RIGHT, RESIZE_Y_TOP);
			return;
		}
		if (event->window == xwm->drag->resize_bl.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_LEFT, RESIZE_Y_BOTTOM);
			return;
		}
		if (event->window == xwm->drag->resize_br.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_RIGHT, RESIZE_Y_BOTTOM);
			return;
		}
		if (event->window == xwm->drag->resize_l.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_LEFT, RESIZE_Y_NONE);
			return;
		}
		if (event->window == xwm->drag->resize_r.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_RIGHT, RESIZE_Y_NONE);
			return;
		}
		if (event->window == xwm->drag->resize_t.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_NONE, RESIZE_Y_TOP);
			return;
		}
		if (event->window == xwm->drag->resize_b.window)
		{
			handle_resize_drag(xwm, event,
			                   RESIZE_X_NONE, RESIZE_Y_BOTTOM);
			return;
		}
	}
}

static void draw_background(struct xwm *xwm)
{
	XCopyArea(xwm->display, xwm->assets.background.pixmap, xwm->container,
	          xwm->bitmap_gc, 0, 0, xwm->screen_width, xwm->screen_height,
	          0, 0);
}

static void draw_taskbar(struct xwm *xwm)
{
	struct asset *asset = &xwm->assets.taskbar;
	size_t xfrom = xwm->assets.taskbar_l.width;
	size_t xto = xwm->screen_width
	           - xwm->assets.taskbar_r.width
	           - xwm->assets.taskbar_l.width;
	for (size_t x = xfrom; x < xto; x += asset->width)
	{
		XCopyArea(xwm->display, asset->pixmap, xwm->taskbar,
		          xwm->bitmap_gc, 0, 0, asset->width, asset->height,
		          x, 0);
	}
	asset = &xwm->assets.taskbar_l;
	XCopyArea(xwm->display, asset->pixmap, xwm->taskbar, xwm->bitmap_gc,
	          0, 0, asset->width, asset->height, 0, 0);
	asset = &xwm->assets.taskbar_r;
	XCopyArea(xwm->display, asset->pixmap, xwm->taskbar, xwm->bitmap_gc,
	          0, 0, asset->width, asset->height,
	          xwm->screen_width - asset->width, 0);
}

static void handle_expose(struct xwm *xwm, XExposeEvent *event)
{
	if (event->window == xwm->root)
	{
		draw_background(xwm);
		return;
	}
	if (event->window == xwm->taskbar)
	{
		draw_taskbar(xwm);
		return;
	}
}

static void handle_error(struct xwm *xwm, XErrorEvent *event)
{
	fprintf(stderr, "X Error of failed request: %u\n",
	        event->error_code);
	fprintf(stderr, "  Major opcode of failed request: %u\n",
	        event->request_code);
	fprintf(stderr, "  Serial number of failed request: %lu\n",
	        event->serial);
	fprintf(stderr, "  Current serial number in output stream: %lu\n",
	        XNextRequest(xwm->display));
}

static void handle_property_notify(struct xwm *xwm, XPropertyEvent *event)
{
	struct window *window = get_window(xwm, event->window);
	if (!window)
		return;
	switch (event->atom)
	{
		case XA_WM_NAME:
			load_wm_name(xwm, window);
			draw_header(xwm, window);
			if (!window->icon_name)
				draw_taskbar_button(xwm, window);
			break;
		case XA_WM_ICON_NAME:
			load_wm_icon_name(xwm, window);
			draw_taskbar_button(xwm, window);
			break;
	}
}

static void handle_event(struct xwm *xwm, XEvent *event)
{
	switch (event->type)
	{
		case 0:
			handle_error(xwm, &event->xerror);
			break;
		case CreateNotify:
			handle_create_notify(xwm, &event->xcreatewindow);
			break;
		case DestroyNotify:
			handle_destroy_notify(xwm, &event->xdestroywindow);
			break;
		case MapRequest:
			handle_map_request(xwm, &event->xmaprequest);
			break;
		case MapNotify:
			handle_map_notify(xwm, &event->xmap);
			break;
		case UnmapNotify:
			handle_unmap_notify(xwm, &event->xunmap);
			break;
		case ConfigureRequest:
			handle_configure_request(xwm, &event->xconfigurerequest);
			break;
		case EnterNotify:
			handle_enter_notify(xwm, &event->xcrossing);
			break;
		case LeaveNotify:
			handle_leave_notify(xwm, &event->xcrossing);
			break;
		case ButtonPress:
			handle_button_press(xwm, &event->xbutton);
			break;
		case ButtonRelease:
			handle_button_release(xwm, &event->xbutton);
			break;
		case MotionNotify:
			handle_motion_notify(xwm, &event->xmotion);
			break;
		case Expose:
			handle_expose(xwm, &event->xexpose);
			break;
		case PropertyNotify:
			handle_property_notify(xwm, &event->xproperty);
			break;
	}
}

static int setup_taskbar(struct xwm *xwm)
{
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask;
	unsigned swa_mask = CWEventMask;
	xwm->taskbar = XCreateWindow(xwm->display, xwm->root, 0,
	                             xwm->screen_height - TASKBAR_HEIGHT,
	                             xwm->screen_width, TASKBAR_HEIGHT,
	                             0, xwm->vi.depth,
	                             InputOutput, xwm->vi.visual, swa_mask,
	                             &swa);
	if (!xwm->taskbar)
	{
		fprintf(stderr, "%s: failed to create taskbar\n",
		        xwm->progname);
		return 1;
	}
	XMapWindow(xwm->display, xwm->taskbar);
	return 0;
}

static int setup_container(struct xwm *xwm)
{
	xwm->container = XCreateWindow(xwm->display, xwm->root, 0, 0,
	                               xwm->screen_width,
	                               xwm->screen_height - TASKBAR_HEIGHT, 0,
	                               xwm->vi.depth, InputOutput,
	                               xwm->vi.visual, 0, NULL);
	if (!xwm->container)
	{
		fprintf(stderr, "%s: failed to create container window\n",
		        xwm->progname);
		return 1;
	}
	XMapWindow(xwm->display, xwm->container);
	return 0;
}

static int setup_cursors(struct xwm *xwm)
{
#define LOAD_CURSOR(name) \
	xwm->cursors.name = XCreateFontCursor(xwm->display, XC_##name); \
	if (!xwm->cursors.name) \
	{ \
		fprintf(stderr, "%s: failed to create " #name " cursor\n", \
		        xwm->progname); \
		return 1; \
	}

	LOAD_CURSOR(left_ptr);
	LOAD_CURSOR(hand1);
	LOAD_CURSOR(bottom_left_corner);
	LOAD_CURSOR(bottom_right_corner);
	LOAD_CURSOR(top_left_corner);
	LOAD_CURSOR(top_right_corner);
	LOAD_CURSOR(left_side);
	LOAD_CURSOR(right_side);
	LOAD_CURSOR(top_side);
	LOAD_CURSOR(bottom_side);
	LOAD_CURSOR(fleur);

#undef LOAD_CURSOR

	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-d] [-h]\n", progname);
	printf("-d: daemonize\n");
	printf("-h: display this help\n");
}

int main(int argc, char **argv)
{
	int daemonize = 0;
	int c;

	while ((c = getopt(argc, argv, "dh")) != -1)
	{
		switch (c)
		{
			case 'd':
				daemonize = 1;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	/* XXX drop privileges */
	struct xwm xwm;
	memset(&xwm, 0, sizeof(xwm));
	if (!getuid())
		xwm.assets_path = "/usr/share/xwm";
	else
		xwm.assets_path = "./src/xwm/assets";
	TAILQ_INIT(&xwm.windows);
	xwm.progname = argv[0];
	xwm.display = XOpenDisplay(NULL);
	if (!xwm.display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XShmQueryVersion(xwm.display, NULL, NULL, NULL))
	{
		fprintf(stderr, "%s: failed to get xshm version\n", argv[0]);
		return EXIT_FAILURE;
	}
	xwm.screen = DefaultScreen(xwm.display);
	xwm.root = RootWindow(xwm.display, xwm.screen);
	xwm.screen_width = DisplayWidth(xwm.display, xwm.screen);
	xwm.screen_height = DisplayHeight(xwm.display, xwm.screen);
	XGrabServer(xwm.display);
	if (XSelectInput(xwm.display, xwm.root, SubstructureRedirectMask
	                                      | SubstructureNotifyMask
	                                      | ExposureMask))
	{
		fprintf(stderr, "%s: failed to select input\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XMatchVisualInfo(xwm.display, xwm.screen, 24, TrueColor, &xwm.vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	xwm.header_gc = XCreateGC(xwm.display, xwm.root, 0, NULL);
	if (!xwm.header_gc)
	{
		fprintf(stderr, "%s: failed to create header gc\n", argv[0]);
		return EXIT_FAILURE;
	}
	xwm.bitmap_gc = XCreateGC(xwm.display, xwm.root, 0, NULL);
	if (!xwm.bitmap_gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	xwm.header_font = XLoadFont(xwm.display, "8x16");
	if (!xwm.header_font)
	{
		fprintf(stderr, "%s: failed to open header font\n", argv[0]);
		return EXIT_FAILURE;
	}
	XGCValues gc_values;
	gc_values.background = 0x00141B1E;
	gc_values.foreground = 0x00D0D0D0;
	gc_values.font = xwm.header_font;
	unsigned gc_mask = GCForeground | GCBackground | GCFont;
	xwm.header_text_gc = XCreateGC(xwm.display, xwm.root, gc_mask,
	                               &gc_values);
	if (!xwm.header_text_gc)
	{
		fprintf(stderr, "%s: failed to create header text gc\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	xwm.taskbar_font = xwm.header_font;
	gc_values.background = 0x000A0A0A;
	gc_values.foreground = 0x00D0D0D0;
	gc_values.font = xwm.taskbar_font;
	xwm.taskbar_text_gc = XCreateGC(xwm.display, xwm.root, gc_mask,
	                                &gc_values);
	if (!xwm.taskbar_text_gc)
	{
		fprintf(stderr, "%s: failed to create taskbar text gc\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	xwm.resize_gc = XCreateGC(xwm.display, xwm.root, gc_mask, &gc_values);
	if (!xwm.resize_gc)
	{
		fprintf(stderr, "%s: failed to create resize gc\n",
		        argv[0]);
		return EXIT_FAILURE;
	}

	if (setup_cursors(&xwm))
		return EXIT_FAILURE;
	if (setup_assets(&xwm))
		return EXIT_FAILURE;
	if (setup_taskbar(&xwm))
		return EXIT_FAILURE;
	if (setup_container(&xwm))
		return EXIT_FAILURE;
	if (setup_background(&xwm))
		return EXIT_FAILURE;

	draw_background(&xwm);
	XDefineCursor(xwm.display, xwm.root, xwm.cursors.left_ptr);

	XSync(xwm.display, False);
	XSynchronize(xwm.display, False);

	while (XPending(xwm.display))
	{
		XEvent event;
		if (XNextEvent(xwm.display, &event))
			continue;
		handle_event(&xwm, &event);
		XFlush(xwm.display);
	}
	XSync(xwm.display, False);
	xwm.enable_reframe = 1;
	XUngrabServer(xwm.display);

	if (daemonize)
	{
		if (daemon(1, 1))
		{
			fprintf(stderr, "%s: daemon: %s\n",
			        argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
	}
	while (1)
	{
		XEvent event;
		if (XNextEvent(xwm.display, &event))
			continue;
		handle_event(&xwm, &event);
	}
	return EXIT_SUCCESS;
}
