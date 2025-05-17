#include <GLX/glx.h>

#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	Display *display;
	const char *extensions;
	const char *client_vendor;
	const char *client_version;
	const char *client_extensions;
	const char *server_vendor;
	const char *server_version;
	const char *server_extensions;
	int glx_major_opcode;
	int glx_first_event;
	int glx_first_error;
	int glx_major;
	int glx_minor;
	int screen;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	screen = DefaultScreen(display);
	if (!XQueryExtension(display,
	                     "GLX",
	                     &glx_major_opcode,
	                     &glx_first_event,
	                     &glx_first_error))
	{
		fprintf(stderr, "%s: no glx\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!glXQueryVersion(display, &glx_major, &glx_minor))
	{
		fprintf(stderr, "%s: failed to get glx version\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("major opcode: %d, "
	       "first event: %d, "
	       "first error: %d, "
	       "major version: %d, "
	       "minor_version: %d\n",
	       glx_major_opcode,
	       glx_first_event,
	       glx_first_error,
	       glx_major,
	       glx_minor);
	extensions = glXQueryExtensionsString(display, screen);
	if (!extensions)
	{
		fprintf(stderr, "%s: failed to get glx extensions\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("extensions: %s\n", extensions);
	client_vendor = glXGetClientString(display, GLX_VENDOR);
	client_version = glXGetClientString(display, GLX_VERSION);
	client_extensions = glXGetClientString(display, GLX_EXTENSIONS);
	printf("client vendor: %s\n", client_vendor);
	printf("client version: %s\n", client_version);
	printf("client extensions: %s\n", client_extensions);
	server_vendor = glXQueryServerString(display, screen, GLX_VENDOR);
	server_version = glXQueryServerString(display, screen, GLX_VERSION);
	server_extensions = glXQueryServerString(display, screen, GLX_EXTENSIONS);
	printf("server vendor: %s\n", server_vendor);
	printf("server version: %s\n", server_version);
	printf("server extensions: %s\n", server_extensions);
	glXChooseFBConfig(display, 0, NULL, NULL);
	return EXIT_SUCCESS;
}
