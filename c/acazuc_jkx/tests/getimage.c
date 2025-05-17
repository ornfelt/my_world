#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XVisualInfo vi;
	XGCValues gc_values;
	Display *display;
	Window src_win = None;
	Window window;
	Window root;
	GC gc;
	unsigned mask;
	int screen;

	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc >= 2)
		src_win = strtol(argv[1], NULL, 0);
	if (!src_win)
	{
		int revert_to;
		if (!XGetInputFocus(display, &src_win, &revert_to))
		{
			fprintf(stderr, "%s: failed to get focus window\n",
			        argv[0]);
			return EXIT_FAILURE;
		}
	}
	root = DefaultRootWindow(display);
	screen = DefaultScreen(display);
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	swa.event_mask = ExposureMask;
	mask = CWEventMask;
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
	gc_values.graphics_exposures = 0;
	gc = XCreateGC(display, window, GCGraphicsExposures, &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	XMapWindow(display, window);
	while (1)
	{
		XEvent event;

		if (XNextEvent(display, &event))
			continue;
		if (event.type == Expose)
		{
			XImage *image;

			image = XGetImage(display,
			                  src_win,
			                  0,
			                  0,
			                  64,
			                  64,
			                  -1,
			                  ZPixmap);
			if (!image)
			{
				fprintf(stderr, "%s: failed to get image\n",
				        argv[0]);
				return EXIT_FAILURE;
			}
			XPutImage(display, window, gc, image, 0, 0, 0, 0, 64, 64);
			XDestroyImage(image);
		}
	}
	return EXIT_SUCCESS;
}
