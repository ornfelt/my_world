#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

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
	if (argc != 3)
	{
		fprintf(stderr, "%s x y\n", argv[0]);
		return EXIT_FAILURE;
	}
	XWarpPointer(display,
	             None,
	             DefaultRootWindow(display),
	             0,
	             0,
	             0,
	             0,
	             strtol(argv[1], NULL, 0),
	             strtol(argv[2], NULL, 0));
	return EXIT_SUCCESS;
}
