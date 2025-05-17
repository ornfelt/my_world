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
	Window child;
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
	swa.event_mask = ButtonPressMask
	               | ButtonReleaseMask
	               | PointerMotionMask
	               | FocusChangeMask
	               | OwnerGrabButtonMask
	               | EnterWindowMask
	               | LeaveWindowMask
	               | ExposureMask;
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
	XChangeProperty(display, window, XA_WM_NAME, XA_STRING, 8,
	                PropModeReplace, (uint8_t*)"grabbutton", 10);
	XMapWindow(display, window);
	swa.border_pixel = 0xFF00000;
	mask |= CWBorderPixel;
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
	gc_values.foreground = 0;
	gc = XCreateGC(display, window, GCForeground, &gc_values);
	if (!gc)
	{
		fprintf(stderr, "%s: failed to create GC\n", argv[0]);
		return EXIT_FAILURE;
	}
	/*XGrabButton(display, Button1, ShiftMask, child, False,
	            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	            GrabModeAsync, GrabModeAsync, None, None);*/
	float child_x = 0;
	float child_y = 0;
	int cursor_x;
	int cursor_y;
	if (!XQueryPointer(display,
	                   root,
	                   NULL,
	                   NULL,
	                   &cursor_x,
	                   &cursor_y,
	                   NULL,
	                   NULL,
	                   NULL))
	{
		fprintf(stderr, "%s: failed to get cursor position\n", argv[0]);
		return EXIT_FAILURE;
	}
	while (1)
	{
		XEvent event;

		if (XNextEvent(display, &event))
			continue;
		if (event.type == Expose)
		{
			XFillRectangle(display, child, gc, 0, 0, 100, 100);
			XFillRectangle(display, window, gc, 0, 0, 640, 480);
		}
		else if (event.type == ButtonPress && event.xbutton.window == child)
		{
			XGrabPointer(display, child, False,
			             ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
			             GrabModeAsync, GrabModeAsync, None, None, event.xbutton.time);
		}
		else if (event.type == ButtonRelease)
		{
			XUngrabPointer(display, event.xbutton.time);
		}
		printf("[%lu] event %u on %u\n", time(NULL), event.type,
		       event.xany.window);
		if (event.type == MotionNotify && (event.xmotion.state & Button1Mask) && event.xmotion.window == child)
		{
			if (event.xmotion.x_root < cursor_x)
				child_x -= .5;
			else if (event.xmotion.x_root > cursor_x)
				child_x += .5;
			if (event.xmotion.y_root < cursor_y)
				child_y -= .5;
			else if (event.xmotion.y_root > cursor_y)
				child_y += .5;
			if (child_x < 0)
				child_x = 0;
			if (child_y < 0)
				child_y = 0;
			if (child_x > 540)
				child_x = 540;
			if (child_y > 380)
				child_y = 380;
			cursor_x = event.xmotion.x_root;
			cursor_y = event.xmotion.y_root;
			XMoveWindow(display, child, child_x, child_y);
		}
	}
	return EXIT_SUCCESS;
}
