#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XVisualInfo *vi;
	XGCValues gc_values;
	Display *display;
	Window window;
	Window child1;
	Window child2;
	Window child3;
	Window root;
	GC gc;
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
	                (uint8_t*)"circulate",
	                9);
	XMapWindow(display, window);
	gc_values.foreground = 0xFFFFFFFF;
	gc = XCreateGC(display, window, GCForeground, &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	swa.border_pixel = 0xFF00000;
	mask |= CWBorderPixel;
	child1 = XCreateWindow(display,
	                       window,
	                       100,
	                       100,
	                       100,
	                       100,
	                       10,
	                       vi->depth,
	                       InputOutput,
	                       vi->visual,
	                       mask,
	                       &swa);
	if (!child1)
	{
		fprintf(stderr, "%s: failed to create child1\n", argv[0]);
		return EXIT_FAILURE;
	}
	child2 = XCreateWindow(display,
	                       window,
	                       150,
	                       150,
	                       100,
	                       100,
	                       10,
	                       vi->depth,
	                       InputOutput,
	                       vi->visual,
	                       mask,
	                       &swa);
	if (!child2)
	{
		fprintf(stderr, "%s: failed to create child2\n", argv[0]);
		return EXIT_FAILURE;
	}
	child3 = XCreateWindow(display,
	                       window,
	                       75,
	                       175,
	                       100,
	                       100,
	                       10,
	                       vi->depth,
	                       InputOutput,
	                       vi->visual,
	                       mask,
	                       &swa);
	if (!child3)
	{
		fprintf(stderr, "%s: failed to create child2\n", argv[0]);
		return EXIT_FAILURE;
	}
	XMapWindow(display, child1);
	XMapWindow(display, child2);
	XMapWindow(display, child3);
	while (1)
	{
		XEvent event;

		if (XNextEvent(display, &event))
			continue;
		if (event.type == Expose)
		{
			XFillRectangle(display, child1, gc, 0, 0, 100, 100);
			XFillRectangle(display, child2, gc, 0, 0, 100, 100);
			XFillRectangle(display, child3, gc, 0, 0, 100, 100);
		}
		if (event.type == ButtonPress)
		{
			if (event.xbutton.button == Button1)
			{
				if (event.xbutton.window == child1)
					XRaiseWindow(display, child1);
				else if (event.xbutton.window == child2)
					XRaiseWindow(display, child2);
				else if (event.xbutton.window == child3)
					XRaiseWindow(display, child3);
			}
			else if (event.xbutton.button == Button3)
			{
				if (event.xbutton.window == child1)
					XLowerWindow(display, child1);
				else if (event.xbutton.window == child2)
					XLowerWindow(display, child2);
				else if (event.xbutton.window == child3)
					XLowerWindow(display, child3);
			}
		}
	}
	return EXIT_SUCCESS;
}
