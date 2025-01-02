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
	int nfonts;
	char **fonts = XListFonts(display, "*", 4096, &nfonts);
	if (!fonts)
	{
		fprintf(stderr, "%s: failed to list fonts\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = 0; i < nfonts; ++i)
		printf("%s\n", fonts[i]);
	return EXIT_SUCCESS;
}
