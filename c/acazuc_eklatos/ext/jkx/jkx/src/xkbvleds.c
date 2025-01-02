#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	(void)argc;
	Display *display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	Window root = XRootWindow(display, 0);
	int screen = DefaultScreen(display);
	XVisualInfo vi;
	if (!XMatchVisualInfo(display, screen, 24, TrueColor, &vi))
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask
	               | KeyPressMask
	               | KeyReleaseMask
	               | FocusChangeMask;
	unsigned mask = CWEventMask;
	Window window = XCreateWindow(display, root, 0, 0, 144, 20, 0, vi.depth,
	                              InputOutput, vi.visual, mask, &swa);
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
	XSynchronize(display, False);
	while (1)
	{
		XEvent event;
		if (XNextEvent(display, &event))
			continue;
		if (event.type == Expose
		 || event.type == KeyPress
		 || event.type == KeyRelease
		 || event.type == FocusIn)
		{
			XKeyboardState state;
			if (!XGetKeyboardControl(display, &state))
			{
				fprintf(stderr, "%s: failed to get keyboard state\n", argv[0]);
				return EXIT_FAILURE;
			}
			XSetForeground(display, gc, 0xFF666666);
			XFillRectangle(display, window, gc, 0, 0, 144, 20);
			for (uint8_t i = 0; i < 14; ++i)
			{
				int x = 144 - 10 * (i + 1);
				int y = 4;
				XSetForeground(display, gc, 0xFFFFFFFF);
				XSegment white_segs[] =
				{
					{x + 5, y + 1, x + 5, y + 11},
					{x + 1, y + 11, x + 5, y + 11},
				};
				XDrawSegments(display, window, gc, white_segs, 2);
				XSetForeground(display, gc, 0);
				XSegment black_segs[] =
				{
					{x, y, x + 5, y},
					{x, y, x, y + 11},
				};
				XDrawSegments(display, window, gc, black_segs, 2);
				if (state.led_mask & (1 << i))
					XSetForeground(display, gc, 0xFF00FF00);
				else
					XSetForeground(display, gc, 0xFF005000);
				XFillRectangle(display, window, gc, x + 1, y + 1, 4, 10);
			}
			XFlush(display);
		}
	}
	return EXIT_SUCCESS;
}
