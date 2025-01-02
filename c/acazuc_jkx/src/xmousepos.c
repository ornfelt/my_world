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
	Window win;
	if (argc > 1)
	{
		win = strtol(argv[1], NULL, 0);
	}
	else
	{
		int revert_to;
		if (!XGetInputFocus(display, &win, &revert_to))
		{
			fprintf(stderr, "%s: failed to get focus window\n",
			        argv[0]);
			return EXIT_FAILURE;
		}
	}
	Window root;
	Window child;
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	unsigned mask;
	if (!XQueryPointer(display, win, &root, &child, &root_x, &root_y, &win_x,
	                   &win_y, &mask))
	{
		fprintf(stderr, "%s: failed to get mouse position\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("%d %d %d %d\n", root_x, root_y, win_x, win_y);
	return EXIT_SUCCESS;
}
