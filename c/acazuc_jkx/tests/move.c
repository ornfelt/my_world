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
	XSetWindowAttributes attributes;
	XVisualInfo vi;
	Display *display;
	Window window;
	Window root;
	int screen;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	root = DefaultRootWindow(display);
	screen = DefaultScreen(display);
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	attributes.border_pixel = 0xFF0000;
	window = XCreateWindow(display,
	                       root,
	                       0,
	                       0,
	                       50,
	                       50,
	                       10,
	                       vi.depth,
	                       InputOutput,
	                       vi.visual,
	                       CWBorderPixel,
	                       &attributes);
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
	                (uint8_t*)"move",
	                4);
	XMapWindow(display, window);
	while (1)
	{
		XEvent event;

		while (XPending(display))
			XNextEvent(display, &event);
		float t = nanotime() / 1000000000.f * M_PI;
		int32_t x = 400 + 100 * cosf(t);
		int32_t y = 400 + 100 * sinf(t);
		int32_t width = 50 + cosf(t) * 25;
		int32_t height = 50 + sinf(t) * 25;
		uint32_t border = 10 + sinf(t) * 5;
		XMoveResizeWindow(display, window, x, y, width, height);
		XSetWindowBorderWidth(display, window, border);
		usleep(10000);
	}
	return EXIT_SUCCESS;
}
