#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char *
map_states[] =
{
	[IsUnmapped]   = "IsUnmapped",
	[IsUnviewable] = "IsUnviewable",
	[IsViewable]   = "IsViewable",
};

static const char *
classes[] =
{
	[InputOnly]   = "InputOnly",
	[InputOutput] = "InputOutput",
};

static const char *
win_gravities[] =
{
	[UnmapGravity]     = "UnmapGravity",
	[NorthWestGravity] = "NorthWestGravity",
	[NorthGravity]     = "NorthGravity",
	[NorthEastGravity] = "NorthEastGravity",
	[WestGravity]      = "WestGravity",
	[CenterGravity]    = "CenterGravity",
	[EastGravity]      = "EastGravity",
	[SouthWestGravity] = "SouthWestGravity",
	[SouthGravity]     = "SouthGravity",
	[SouthEastGravity] = "SouthEastGravity",
	[StaticGravity]    = "StaticGravity",
};

static const char *
bit_gravities[] =
{
	[ForgetGravity]    = "ForgetGravity",
	[NorthWestGravity] = "NorthWestGravity",
	[NorthGravity]     = "NorthGravity",
	[NorthEastGravity] = "NorthEastGravity",
	[WestGravity]      = "WestGravity",
	[CenterGravity]    = "CenterGravity",
	[EastGravity]      = "EastGravity",
	[SouthWestGravity] = "SouthWestGravity",
	[SouthGravity]     = "SouthGravity",
	[SouthEastGravity] = "SouthEastGravity",
	[StaticGravity]    = "StaticGravity",
};

static const char *
backing_stores[] =
{
	[NotUseful]  = "NotUseful",
	[WhenMapped] = "WhenMapped",
	[Always]     = "Always",
};

static const char *
event_masks[] =
{
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"EnterWindow",
	"LeaveWindow"
	"PointerMotion",
	"PointerMotionHint",
	"Button1Motion",
	"Button2Motion",
	"Button3Motion",
	"Button4Motion",
	"Button5Motion",
	"ButtonMotion",
	"KeymapState",
	"Exposure",
	"VisibilityChange",
	"StructureNotify",
	"ResizeRedirect",
	"SubstructureNotify",
	"SubstructureRedirect",
	"FocusChange",
	"PropertyChange",
	"ColormapChange",
	"OwnerGrabButton",
};

int
main(int argc, char **argv)
{
	XWindowAttributes attributes;
	XTextProperty *prop;
	Display *display;
	Window window;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc > 1)
	{
		window = strtol(argv[1], NULL, 10);
	}
	else
	{
		int revert_to;
		if (!XGetInputFocus(display, &window, &revert_to))
		{
			fprintf(stderr, "%s: failed to get focus window\n",
			        argv[0]);
			return EXIT_FAILURE;
		}
	}
	if (!window)
		window = DefaultRootWindow(display);
	if (!XGetWindowAttributes(display, window, &attributes))
	{
		fprintf(stderr, "%s: failed to get window attributes\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	if (!XGetWMName(display, window, &prop))
		prop = NULL;
	printf("\n");
	printf("%s: Window id: 0x%x \"%s\"\n",
	       argv[0],
	       window,
	       prop ? (char*)prop->value : "");
	printf("\n");
	printf("  Relative upper-left X: %d\n", attributes.x);
	printf("  Relative upper-left Y: %d\n", attributes.y);
	printf("  Width: %u\n", attributes.width);
	printf("  Height: %u\n", attributes.height);
	printf("  Depth: %u\n", attributes.depth);
	printf("  Visual: 0x%x\n", XVisualIDFromVisual(attributes.visual));
	printf("  Visual Class: %s\n", ""); /* XXX */
	printf("  Border Width: %u\n", attributes.border_width);
	printf("  Class: %s\n", classes[attributes.class]);
	printf("  Colormap: 0x%x (%s)\n", attributes.colormap,
	       attributes.map_installed ? "installed" : "not installed");
	printf("  Bit Gravity State: %s\n",
	       bit_gravities[attributes.bit_gravity]);
	printf("  Window Gravity State: %s\n",
	       win_gravities[attributes.win_gravity]);
	printf("  Backing Store State: %s\n",
	       backing_stores[attributes.backing_store]);
	printf("  Save Under State: %s\n", attributes.save_under ? "yes" : "no");
	printf("  Map State: %s\n", map_states[attributes.map_state]);
	printf("  Override Redirect State: %s\n",
	       attributes.override_redirect ? "yes" : "no");
	printf("\n");
	printf("  Bit Gravity: %s\n",
	       bit_gravities[attributes.bit_gravity]);
	printf("  Window Gravity: %s\n",
	       win_gravities[attributes.win_gravity]);
	printf("  Backing-store hint: %s\n",
	       backing_stores[attributes.backing_store]);
	printf("  Backing-planes to be preserved: 0x%lx\n",
	       attributes.backing_planes);
	printf("  Backing pixel: %lu\n", attributes.backing_pixel);
	printf("  Save-unders: %s\n", attributes.save_under ? "Yes" : "No");
	printf("\n");
	printf("  Someone wants these events:\n");
	for (size_t i = 0; i < 25; ++i)
	{
		if (!(attributes.all_event_masks & (1 << i)))
			continue;
		printf("      %s\n", event_masks[i]);
	}
	printf("  Do not propagate these events:\n");
	for (size_t i = 0; i < 25; ++i)
	{
		if (!(attributes.do_not_propagate_mask & (1 << i)))
			continue;
		printf("      %s\n", event_masks[i]);
	}
	printf("  Override redirection?: : %s\n",
	       attributes.override_redirect ? "Yes" : "No");
	printf("\n");
	return EXIT_SUCCESS;
}
