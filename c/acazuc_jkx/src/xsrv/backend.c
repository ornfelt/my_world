#include "xsrv.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#if defined (__eklat__)

#include <eklat/evdev.h>
#include <eklat/fb.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>

static uint32_t tmp_pointer_x;
static uint32_t tmp_pointer_y;

static int fb;
static uint8_t *fb0;

int
backend_init(struct xsrv *xsrv)
{
	struct fb_info fb_info;
	int flags;

	fb = open("/dev/fb0", O_RDWR);
	if (fb == -1)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        xsrv->progname,
		        "/dev/fb0",
		        strerror(errno));
		return 1;
	}
	if (ioctl(fb, FBIOGET_INFO, &fb_info) == -1)
	{
		fprintf(stderr, "%s: ioctl(FBIOGET_INFO): %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	if (fb_info.bpp != 32)
	{
		fprintf(stderr, "%s: bpp != 32 not supported\n",
		        xsrv->progname);
		return 1;
	}
	xsrv->backend.width = fb_info.width;
	xsrv->backend.height = fb_info.height;
	xsrv->backend.pitch = fb_info.pitch;
	xsrv->backend.bpp = fb_info.bpp;
	fb0 = mmap(NULL,
	           fb_info.height * fb_info.pitch,
	           PROT_READ | PROT_WRITE,
	           MAP_PRIVATE,
	           fb,
	           0);
	if (fb0 == MAP_FAILED)
	{
		fprintf(stderr, "%s: mmap: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	xsrv->backend.data = mmap(NULL,
	                          fb_info.height * fb_info.pitch,
	                          PROT_READ | PROT_WRITE,
	                          MAP_PRIVATE | MAP_ANONYMOUS,
	                          -1,
	                          0);
	if (xsrv->backend.data == MAP_FAILED)
	{
		fprintf(stderr, "%s: mmap: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	xsrv->backend.fd = open("/dev/event0", O_RDONLY);
	if (xsrv->backend.fd == -1)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        xsrv->progname,
		        "/dev/event0",
		        strerror(errno));
		return 1;
	}
	flags = fcntl(xsrv->backend.fd, F_GETFL, 0);
	if (flags < 0)
	{
		fprintf(stderr, "%s: fcntl(F_GETFL): %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	flags |= O_NONBLOCK;
	if (fcntl(xsrv->backend.fd, F_SETFL, flags) == -1)
	{
		fprintf(stderr, "%s: fcntl(F_SETFL): %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}

	return 0;
}

int
backend_display(struct xsrv *xsrv, struct rect *rect)
{
	struct fb_rect fb_rect;
	uint8_t *src = xsrv->backend.data;
	uint8_t *dst = fb0;
	uint32_t bytes;

	src += rect->top * xsrv->backend.pitch;
	src += rect->left * 4;
	dst += rect->top * xsrv->backend.pitch;
	dst += rect->left * 4;
	bytes = (rect->right - rect->left + 1) * 4;
	for (uint32_t y = rect->top; y <= rect->bottom; ++y)
	{
		memcpy(dst, src, bytes);
		src += xsrv->backend.pitch;
		dst += xsrv->backend.pitch;
	}
	fb_rect.x = rect->left;
	fb_rect.y = rect->top;
	fb_rect.width = rect->right - rect->left + 1;
	fb_rect.height = rect->bottom - rect->top + 1;
	if (ioctl(fb, FBIO_FLUSH, &fb_rect) == -1)
	{
		fprintf(stderr, "%s: ioctl(FBIO_FLUSH): %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static void
commit_pointer(struct xsrv *xsrv)
{
	if (tmp_pointer_x == xsrv->pointer.x
	 && tmp_pointer_y == xsrv->pointer.y)
		return;
	xsrv_cursor_motion(xsrv, tmp_pointer_x, tmp_pointer_y);
}

static void
process_pointer_event(struct xsrv *xsrv, struct pointer_event *event)
{
	if (event->x < 0)
	{
		if ((size_t)-event->x > tmp_pointer_x)
			tmp_pointer_x = 0;
		else
			tmp_pointer_x += event->x;
	}
	else
	{
		if ((size_t)event->x >= xsrv->backend.width - tmp_pointer_x)
			tmp_pointer_x = xsrv->backend.width - 1;
		else
			tmp_pointer_x += event->x;
	}
	if (event->y < 0)
	{
		if ((size_t)-event->y > tmp_pointer_y)
			tmp_pointer_y = 0;
		else
			tmp_pointer_y += event->y;
	}
	else
	{
		if ((size_t)event->y >= xsrv->backend.height - tmp_pointer_y)
			tmp_pointer_y = xsrv->backend.height - 1;
		else
			tmp_pointer_y += event->y;
	}
}

static void
process_mouse_event(struct xsrv *xsrv, struct mouse_event *event)
{
	if (event->pressed)
		xsrv_button_press(xsrv, event->button);
	else
		xsrv_button_release(xsrv, event->button);
}

static void
process_scroll_event(struct xsrv *xsrv, struct scroll_event *event)
{
	if (event->y < 0)
	{
		xsrv_button_press(xsrv, 5);
		xsrv_button_release(xsrv, 5);
	}
	else if (event->y > 0)
	{
		xsrv_button_press(xsrv, 4);
		xsrv_button_release(xsrv, 4);
	}
}

static void
process_key_event(struct xsrv *xsrv, struct key_event *event)
{
	if (event->pressed)
		xsrv_key_press(xsrv, event->key + 8);
	else
		xsrv_key_release(xsrv, event->key + 8);
}

static void
process_event(struct xsrv *xsrv, struct event *event)
{
	switch (event->type)
	{
		case EVENT_POINTER:
			process_pointer_event(xsrv, &event->pointer);
			break;
		case EVENT_MOUSE:
			commit_pointer(xsrv);
			process_mouse_event(xsrv, &event->mouse);
			break;
		case EVENT_SCROLL:
			commit_pointer(xsrv);
			process_scroll_event(xsrv, &event->scroll);
			break;
		case EVENT_KEY:
			commit_pointer(xsrv);
			process_key_event(xsrv, &event->key);
			break;
	}
}

int
backend_poll(struct xsrv *xsrv)
{
	struct event events[128];
	ssize_t rd;
	size_t nevents;

	tmp_pointer_x = xsrv->pointer.x;
	tmp_pointer_y = xsrv->pointer.y;
	rd = read(xsrv->backend.fd, &events, sizeof(events));
	if (rd == -1 && errno == EAGAIN)
		return 0;
	if (rd <= 0)
		return 1;
	nevents = (size_t)rd / sizeof(*events);
	for (size_t i = 0; i < nevents; ++i)
		process_event(xsrv, &events[i]);
	commit_pointer(xsrv);
	return 0;
}

#else

#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <sys/shm.h>

#define WIDTH 1280
#define HEIGHT 720

static Display *display;
static Window window;
static XVisualInfo *vi;
static XShmSegmentInfo shminfo;
static XImage *image;
static Cursor cursor;
static GC gc;
static uint32_t tmp_pointer_x;
static uint32_t tmp_pointer_y;

static void
setup_cursor(void)
{
	static char zero[2 * 32] = {0};
	XColor background;
	XColor foreground;
	Pixmap source;
	Pixmap mask;

	source = XCreateBitmapFromData(display, window, zero, 32, 16);
	mask = XCreateBitmapFromData(display, window, zero, 32, 16);
	background.pixel = 0xFFFFFFFF;
	background.red = 0xFFFF;
	background.green = 0xFFFF;
	background.blue = 0xFFFF;
	background.flags = 0;
	background.pad = 0;
	foreground.pixel = 0;
	foreground.red = 0;
	foreground.green = 0;
	foreground.blue = 0;
	foreground.flags = 0;
	foreground.pad = 0;
	cursor = XCreatePixmapCursor(display,
	                             source,
	                             mask,
	                             &foreground,
	                             &background,
	                             8,
	                             8);
	XDefineCursor(display, window, cursor);
}

int
backend_init(struct xsrv *xsrv)
{
	XSetWindowAttributes swa;
	XSizeHints hints;
	XGCValues gc_values;
	int nitems;

	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n",
		        xsrv->progname);
		return 1;
	}
	vi = XGetVisualInfo(display, 0, NULL, &nitems);
	if (!vi)
	{
		fprintf(stderr, "%s: failed to get visual info\n",
		        xsrv->progname);
		return 1;
	}
	swa.event_mask = KeyPressMask
	               | KeyReleaseMask
	               | PointerMotionMask
	               | ButtonPressMask
	               | ButtonReleaseMask
	               | ExposureMask;
	window = XCreateWindow(display,
	                       DefaultRootWindow(display),
	                       0,
	                       0,
	                       WIDTH,
	                       HEIGHT,
	                       0,
	                       vi->depth,
	                       InputOutput,
	                       vi->visual,
	                       CWEventMask,
	                       &swa);
	memset(&hints, 0, sizeof(hints));
	hints.flags = PMinSize | PMaxSize;
	hints.min_width = WIDTH;
	hints.min_height = HEIGHT;
	hints.max_width = WIDTH;
	hints.max_height = HEIGHT;
	XSetWMNormalHints(display, window, &hints);
	setup_cursor();
	gc_values.foreground = 0xFFFFFFFF;
	gc_values.graphics_exposures = 0;
	gc = XCreateGC(display,
	               window,
	               GCForeground | GCGraphicsExposures,
	               &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", xsrv->progname);
		return 1;
	}
	image = XShmCreateImage(display,
	                        vi->visual,
	                        24,
	                        ZPixmap,
	                        NULL,
	                        &shminfo,
	                        WIDTH,
	                        HEIGHT);
	if (!image)
	{
		fprintf(stderr, "%s: failed to create image\n",
		        xsrv->progname);
		return 1;
	}
	shminfo.shmid = shmget(387920,
	                       image->bytes_per_line * image->height,
	                       IPC_CREAT | 0777);
	if (shminfo.shmid == -1)
	{
		fprintf(stderr, "%s: shmget: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	image->data = shmat(shminfo.shmid, 0, 0);
	if (image->data == (void*)-1)
	{
		fprintf(stderr, "%s: shmat: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	shminfo.shmaddr = image->data;
	shminfo.readOnly = False;
	XShmAttach(display, &shminfo);

	xsrv->backend.width = WIDTH;
	xsrv->backend.height = HEIGHT;
	xsrv->backend.pitch = WIDTH * 4;
	xsrv->backend.bpp = 32;
	xsrv->backend.data = image->data;
	xsrv->backend.fd = XConnectionNumber(display);

	XMapWindow(display, window);

	while (1)
	{
		XEvent event;
		XNextEvent(display, &event);
		if (event.type == Expose)
			break;
	}

	return 0;
}

int
backend_display(struct xsrv *xsrv, struct rect *rect)
{
	(void)xsrv;
	XShmPutImage(display,
	             window,
	             gc,
	             image,
	             rect->left,
	             rect->top,
	             rect->left,
	             rect->top,
	             rect->right - rect->left + 1,
	             rect->bottom - rect->top + 1,
	             False);
	XFlush(display);
	return 0;
}

static void
commit_pointer(struct xsrv *xsrv)
{
	if (tmp_pointer_x == xsrv->pointer.x
	 && tmp_pointer_y == xsrv->pointer.y)
		return;
	xsrv_cursor_motion(xsrv, tmp_pointer_x, tmp_pointer_y);
}

int
backend_poll(struct xsrv *xsrv)
{
	XEvent event;

	tmp_pointer_x = xsrv->pointer.x;
	tmp_pointer_y = xsrv->pointer.y;
	while (XPending(display))
	{
		XNextEvent(display, &event);
		switch (event.type)
		{
			case KeyPress:
				commit_pointer(xsrv);
				xsrv_key_press(xsrv, event.xkey.keycode);
				break;
			case KeyRelease:
				commit_pointer(xsrv);
				xsrv_key_release(xsrv, event.xkey.keycode);
				break;
			case MotionNotify:
			{
				int x = event.xmotion.x;
				int y = event.xmotion.y;
				if (x < 0)
					x = 0;
				if (y < 0)
					y = 0;
				tmp_pointer_x = x;
				tmp_pointer_y = y;
				break;
			}
			case ButtonPress:
				commit_pointer(xsrv);
				xsrv_button_press(xsrv, event.xbutton.button);
				break;
			case ButtonRelease:
				commit_pointer(xsrv);
				xsrv_button_release(xsrv, event.xbutton.button);
				break;
		}
	}
	commit_pointer(xsrv);
	return 0;
}

#endif
