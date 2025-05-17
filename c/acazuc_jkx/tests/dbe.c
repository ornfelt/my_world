#include <X11/extensions/Xdbe.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	Display *display;
	int xdbe_major_opcode;
	int xdbe_first_event;
	int xdbe_first_error;
	int xdbe_major;
	int xdbe_minor;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XQueryExtension(display,
	                     "DOUBLE-BUFFER",
	                     &xdbe_major_opcode,
	                     &xdbe_first_event,
	                     &xdbe_first_error))
	{
		fprintf(stderr, "%s: no xdbe\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!XdbeQueryExtension(display, &xdbe_major, &xdbe_minor))
	{
		fprintf(stderr, "%s: failed to get xdbe version\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("major opcode: %d, "
	       "first event: %d, "
	       "first error: %d, "
	       "major version: %d, "
	       "minor_version: %d\n",
	       xdbe_major_opcode,
	       xdbe_first_event,
	       xdbe_first_error,
	       xdbe_major,
	       xdbe_minor);
	return EXIT_SUCCESS;
}
