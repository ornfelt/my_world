#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

static int
error_handler(Display *display, XErrorEvent *event)
{
	(void)display;
	(void)event;
	exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
	Display *display;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	XSetErrorHandler(display, error_handler);
	for (Atom atom = 1;; ++atom)
	{
		char *name = XGetAtomName(display, atom);
		if (!name)
			break;
		printf("%u\t%s\n", atom, name);
	}
	return EXIT_SUCCESS;
}
