#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

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
	XVisualInfo vi;
	XGCValues gc_values;
	Display *display;
	Window window;
	Window root;
	int screen;
	GC gc;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	XSynchronize(display, False);
	root = DefaultRootWindow(display);
	screen = DefaultScreen(display);
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	window = XCreateWindow(display,
	                       root,
	                       0,
	                       0,
	                       640,
	                       480,
	                       0,
	                       vi.depth,
	                       InputOutput,
	                       vi.visual,
	                       0,
	                       NULL);
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
	                (uint8_t*)"anim",
	                4);
	gc_values.foreground = 0xFFFFFFFF;
	gc_values.graphics_exposures = 0;
	gc = XCreateGC(display,
	               window,
	               GCForeground | GCGraphicsExposures,
	               &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	XMapWindow(display, window);
	while (1)
	{
		XEvent event;

		while (XPending(display))
			XNextEvent(display, &event);
		float t = nanotime() / 1000000000.f * M_PI;
		int32_t x = 320 + 280 * cosf(t);
		int32_t y = 240 + 200 * sinf(t);
		XDrawArc(display, window, gc,
		         x - 16, y - 16, 32, 32,
		         0, 360 * 64);
		usleep(10000);
	}
	return EXIT_SUCCESS;
}
