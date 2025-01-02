#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	Display *display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	Window src_win = None;
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
	Window root = DefaultRootWindow(display);
	int screen = DefaultScreen(display);
	XVisualInfo vi;
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask;
	unsigned mask = CWEventMask;
	Window window = XCreateWindow(display, root, 0, 0, 640, 480, 0,
	                              vi.depth, InputOutput, vi.visual, mask,
	                              &swa);
	if (!window)
	{
		fprintf(stderr, "%s: failed to create window\n", argv[0]);
		return EXIT_FAILURE;
	}
	XGCValues gc_values;
	gc_values.graphics_exposures = 0;
	GC gc = XCreateGC(display, window, GCGraphicsExposures, &gc_values);
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
			XImage *image = XGetImage(display, src_win, 0, 0,
			                          64, 64, -1, ZPixmap);
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
