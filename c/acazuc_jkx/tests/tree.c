#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void
print_indent(size_t indent)
{
	for (size_t i = 0; i < indent; ++i)
		printf("  ");
}

static int
query_tree(const char *progname, Display *display, Window window, size_t indent)
{
	XTextProperty *prop;
	Window *children;
	Window parent;
	Window root;
	unsigned nchildren;

	if (!XQueryTree(display, window, &root, &parent, &children, &nchildren))
	{
		fprintf(stderr, "%s: failed to query tree\n", progname);
		return 1;
	}
	if (!XGetWMName(display, window, &prop))
		prop = NULL;
	print_indent(indent);
	printf("%u: ", window);
	if (prop)
	{
		printf("%s", prop->value);
		free(prop->value);
		free(prop);
	}
	else
	{
		printf("[unknown]");
	}
	printf("\n");
	for (size_t i = 0; i < nchildren; ++i)
	{
		if (query_tree(progname, display, children[i], indent + 1))
		{
			free(children);
			return 1;
		}
	}
	free(children);
	return 0;
}

int
main(int argc, char **argv)
{
	Display *display;
	Window win;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc > 1)
	{
		win = strtol(argv[1], NULL, 10);
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
	if (!win)
		win = DefaultRootWindow(display);
	if (query_tree(argv[0], display, win, 0))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
