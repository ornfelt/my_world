#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

static uint64_t
nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_nsec + ts.tv_sec * 1000000000;
}

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XGCValues gc_values;
	XVisualInfo *vi;
	Display *display;
	Pixmap pixmap;
	Window window;
	Window child;
	Window root;
	XImage *image;
	GC gc;
	uint8_t data[64 * 64 * 4];
	unsigned mask;
	int nitems;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	vi = XGetVisualInfo(display, 0, NULL, &nitems);
	if (!vi)
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	root = XRootWindow(display, 0);
	swa.event_mask = ButtonPressMask | ExposureMask;
	mask = CWEventMask;
	window = XCreateWindow(display,
	                       root,
	                       0,
	                       0,
	                       640,
	                       480,
	                       0,
	                       vi->depth,
	                       InputOutput,
	                       vi->visual,
	                       mask,
	                       &swa);
	if (!window)
	{
		fprintf(stderr, "%s: failed to create window\n", argv[0]);
		return EXIT_FAILURE;
	}
	XChangeProperty(display,
	                window,
	                XA_WM_NAME,
	                XA_STRING,
	                8,
	                PropModeReplace,
	                (uint8_t*)"border_pixmap",
	                13);
	XMapWindow(display, window);
	gc_values.foreground = 0;
	gc = XCreateGC(display, window, GCForeground, &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	pixmap = XCreatePixmap(display, window, 64, 64, 24);
	if (!pixmap)
	{
		fprintf(stderr, "%s: failed to create pixmap\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (size_t y = 0; y < 64; ++y)
	{
		for (size_t x = 0; x < 64; ++x)
		{
			uint8_t *dst = &data[4 * (x + y * 64)];
			dst[0] = 0xFF;
			dst[1] = x << 2;
			dst[2] = y << 2;
			dst[3] = 0xFF;
		}
	}
	image = XCreateImage(display,
	                     vi->visual,
	                     24,
	                     ZPixmap,
	                     0,
	                     (char*)data,
	                     64,
	                     64,
	                     32,
	                     64 * 4);
	if (!image)
	{
		fprintf(stderr, "%s: failed to create image\n", argv[0]);
		return EXIT_FAILURE;
	}
	XPutImage(display, pixmap, gc, image, 0, 0, 0, 0, 64, 64);
	swa.border_pixmap = pixmap;
	mask |= CWBorderPixmap;
	child = XCreateWindow(display,
	                      window,
	                      0,
	                      0,
	                      100,
	                      100,
	                      10,
	                      vi->depth,
	                      InputOutput,
	                      vi->visual,
	                      mask,
	                      &swa);
	if (!child)
	{
		fprintf(stderr, "%s: failed to create child\n", argv[0]);
		return EXIT_FAILURE;
	}
	XMapWindow(display, child);
	while (1)
	{
		XEvent event;

		while (XPending(display))
		{
			XNextEvent(display, &event);
			if (event.type == Expose)
			{
				XFillRectangle(display, child, gc, 0, 0, 200, 200);
				XFillRectangle(display, window, gc, 0, 0, 640, 480);
			}
		}
		float t = nanotime() / 1000000000.f * M_PI;
		uint32_t size = 100 + 50 * cos(t);
		uint32_t border_width = 50 + 50 * sin(t);
		uint8_t color = abs((int)(cos(t * M_PI) * 0xFF));
		XResizeWindow(display, child, size, size);
		XSetWindowBorderWidth(display, child, border_width);
		for (size_t y = 0; y < 64; ++y)
		{
			for (size_t x = 0; x < 64; ++x)
			{
				uint8_t *dst = &data[4 * (x + y * 64)];
				dst[0] = color;
				dst[1] = x << 2;
				dst[2] = y << 2;
				dst[3] = 0xFF;
			}
		}
		XPutImage(display, pixmap, gc, image, 0, 0, 0, 0, 64, 64);
		usleep(10000);
	}
	return EXIT_SUCCESS;
}
