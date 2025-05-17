#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>

static const char *
byte_orders[] =
{
	[LSBFirst] = "LSBFirst",
	[MSBFirst] = "MSBFirst",
};

static const char *
backing_stores[] =
{
	[NotUseful]  = "NOT USEFUL",
	[WhenMapped] = "WHEN MAPPED",
	[Always]     = "ALWAYS",
};

static const char *
revert_tos[] =
{
	[RevertToNone]        = "revert to None",
	[RevertToPointerRoot] = "revert to Pointer root",
	[RevertToParent]      = "revert to Parent",
};

int
main(int argc, char **argv)
{
	XPixmapFormatValues *pixmaps;
	Display *display;
	Window focus;
	char **extensions;
	int min_keycode;
	int max_keycode;
	int nextensions;
	int npixmaps;
	int revert_to;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	XDisplayKeycodes(display, &min_keycode, &max_keycode);
	extensions = XListExtensions(display, &nextensions);
	if (!extensions)
	{
		fprintf(stderr, "%s: failed to list extensions\n", argv[0]);
		return EXIT_FAILURE;
	}
	pixmaps = XListPixmapFormats(display, &npixmaps);
	if (!pixmaps)
	{
		fprintf(stderr, "%s: failed to list pixmap formats\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("name of display: %s\n", DisplayString(display));
	printf("version number: %d.%d\n", ProtocolVersion(display),
	       ProtocolRevision(display));
	printf("vendor string: %s\n", ServerVendor(display));
	printf("vendor release number: %d\n", VendorRelease(display));
	printf("maximum request size: %ld\n", XMaxRequestSize(display));
	printf("motion buffer size: %ld\n", XDisplayMotionBufferSize(display));
	printf("bitmap unit, bit order, padding: %d, %s, %d\n", BitmapUnit(display),
	       byte_orders[BitmapBitOrder(display)], BitmapPad(display));
	printf("image byte order: %s\n", byte_orders[ImageByteOrder(display)]);
	printf("number of supported pixmap formats: %d\n", npixmaps);
	printf("supported pixmap formats:\n");
	for (int i = 0; i < npixmaps; ++i)
	{
		printf("    depth %d, bits_per_pixel %d, scanline_pad %d\n",
		       pixmaps[i].depth, pixmaps[i].bits_per_pixel,
		       pixmaps[i].scanline_pad);
	}
	printf("keycode range: minimum %d maximum %d\n", min_keycode, max_keycode);
	if (!XGetInputFocus(display, &focus, &revert_to))
	{
		fprintf(stderr, "%s: failed to get input focus\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("focus: window 0x%x, %s\n", focus, revert_tos[revert_to]);
	printf("number of extensions: %d\n", nextensions);
	for (size_t i = 0; extensions[i]; ++i)
		printf("    %s\n", extensions[i]);
	printf("default screen number: %d\n", DefaultScreen(display));
	printf("number of screens: %d\n",
	       XScreenNumberOfScreen(DefaultScreenOfDisplay(display)));
	for (int i = 0; i < ScreenCount(display); ++i)
	{
		XVisualInfo *visuals;
		Screen *screen;
		unsigned cursor_width;
		unsigned cursor_height;
		int ndepths;
		int *depths;
		int nvisuals;

		screen = ScreenOfDisplay(display, i);
		depths = XListDepths(display, i, &ndepths);
		visuals = XGetVisualInfo(display,
		                         VisualNoMask,
		                         NULL,
		                         &nvisuals);
		printf("\n");
		printf("screen #%d:\n", i);
		printf(" dimensions: %dx%d pixels (%dx%d millimeters)\n",
		       WidthOfScreen(screen),
		       HeightOfScreen(screen),
		       WidthMMOfScreen(screen),
		       HeightMMOfScreen(screen));
		printf(" depths (%d):", ndepths);
		for (int j = 0; j < ndepths; ++j)
		{
			printf(" %d", depths[j]);
			if (j != ndepths - 1)
				printf(",");
		}
		printf("\n");
		printf(" root window id: %d\n",
		       RootWindowOfScreen(screen));
		printf(" depth of root window: %d planes\n",
		       PlanesOfScreen(screen));
		printf(" number of colormaps: minimum %d maximum %d\n",
		       MinCmapsOfScreen(screen),
		       MaxCmapsOfScreen(screen));
		printf(" default colormap: %d\n",
		       DefaultColormapOfScreen(screen));
		printf(" default number of colormap cells: %d\n",
		       CellsOfScreen(screen));
		printf(" preallocated pixels: black %lu, white %lu\n",
		       BlackPixelOfScreen(screen),
		       WhitePixelOfScreen(screen));
		printf(" options: backing-store %s, save-unders %s\n",
		       backing_stores[DoesBackingStore(screen)],
		       DoesSaveUnders(screen) ? "YES" : "NO");
		XQueryBestCursor(display,
		                 RootWindowOfScreen(screen),
		                 0xFFFF,
		                 0xFFFF,
		                 &cursor_width,
		                 &cursor_height);
		printf(" largest cursor: %ux%u\n",
		       cursor_width,
		       cursor_height);
		printf(" current input event mask: %ld\n",
		       EventMaskOfScreen(screen));
		printf(" number of visuals: %d\n", nvisuals);
		printf(" default visual id: 0x%x\n",
		       XVisualIDFromVisual(DefaultVisualOfScreen(screen)));
		for (int j = 0; j < nvisuals; ++j)
		{
			static const char *
			classes[] =
			{
				[StaticGray]  = "StaticGray",
				[GrayScale]   = "GrayScale",
				[StaticColor] = "StaticColor",
				[PseudoColor] = "PseudoColor",
				[TrueColor]   = "TrueColor",
				[DirectColor] = "DirectColor",
			};
			XVisualInfo *vi = &visuals[j];
			printf("  visual:\n");
			printf("    visual id: 0x%x\n", vi->visualid);
			printf("    class: %s\n", classes[vi->class]);
			printf("    depth: %u planes\n", vi->depth);
			printf("    available colormap entries: %d per subfield\n",
			       vi->colormap_size);
			printf("    red, green, blue masks: 0x%lx, 0x%lx, 0x%lx\n",
			       vi->red_mask, vi->green_mask, vi->blue_mask);
			printf("    significant bits in color specification: %d bits\n",
			       vi->bits_per_rgb);
		}
	}
	return EXIT_SUCCESS;
}
