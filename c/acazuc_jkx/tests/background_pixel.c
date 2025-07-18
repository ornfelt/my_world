#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XGCValues gc_values;
	Display *display;
	XVisualInfo vi;
	Window window;
	Window root;
	GC gc;
	unsigned mask;
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
	swa.background_pixel = 0xFFFF0000;
	swa.event_mask = ExposureMask;
	mask = CWBackingPixel | CWEventMask;
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
	                (uint8_t*)"background_pixel",
	                16);
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
	XClearWindow(display, window);
	while (1)
	{
		XEvent event;

		while (XPending(display))
			XNextEvent(display, &event);
	}
	return EXIT_SUCCESS;
}
