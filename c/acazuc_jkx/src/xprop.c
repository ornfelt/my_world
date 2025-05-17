#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static const char *
window_gravities[] =
{
	[UnmapGravity]     = "Unmap",
	[NorthWestGravity] = "NorthWest",
	[NorthGravity]     = "North",
	[NorthEastGravity] = "NorthEast",
	[WestGravity]      = "West",
	[CenterGravity]    = "Center",
	[EastGravity]      = "East",
	[SouthWestGravity] = "SouthWest",
	[SouthGravity]     = "South",
	[SouthEastGravity] = "SouthEast",
	[StaticGravity]    = "Static",
};

static const char *
initial_states[] =
{
	[DontCareState] = "Don't Care State",
	[NormalState]   = "Normal State",
	[ZoomState]     = "Zoom State",
	[IconicState]   = "Iconic State",
	[InactiveState] = "Inactive State",
};

static const char *
window_states[] =
{
	[WithdrawnState] = "Withdrawn",
	[NormalState]    = "Normal",
	[IconicState]    = "Iconic",
};

static char *
atom_name(Display *display, Atom atom)
{
	char *name;
	char tmp[64];

	name = XGetAtomName(display, atom);
	if (name)
		return name;
	snprintf(tmp, sizeof(tmp), "UNKNOWN (%u)", atom);
	return strdup(tmp);
}

static void
print_size_hints(XSizeHints *size_hints)
{
	if (size_hints->flags & USPosition)
		printf("\n\t\tuser specified location: %d, %d",
		       size_hints->x, size_hints->y);
	if (size_hints->flags & USSize)
		printf("\n\t\tuser specified size: %d by %d",
		       size_hints->width, size_hints->height);
	if (size_hints->flags & PPosition)
		printf("\n\t\tprogram specified location: %d, %d",
		       size_hints->x, size_hints->y);
	if (size_hints->flags & PSize)
		printf("\n\t\tprogram specified size: %d by %d",
		       size_hints->width, size_hints->height);
	if (size_hints->flags & PMinSize)
		printf("\n\t\tprogram specified minimum size: %d by %d",
		       size_hints->min_width, size_hints->min_height);
	if (size_hints->flags & PMaxSize)
		printf("\n\t\tprogram specified maximum size: %d by %d",
		       size_hints->max_width, size_hints->max_height);
	if (size_hints->flags & PResizeInc)
		printf("\n\t\tprogram specified resize increments: %d by %d",
		       size_hints->width_inc, size_hints->height_inc);
	if (size_hints->flags & PAspect)
	{
		printf("\n\t\tprogram specified minimum aspect ratio: %d/%d",
		       size_hints->min_aspect.x, size_hints->min_aspect.y);
		printf("\n\t\tprogram specified maximum aspect ratio: %d/%d",
		       size_hints->max_aspect.x, size_hints->max_aspect.y);
	}
	if (size_hints->flags & PBaseSize)
		printf("\n\t\tprogram specified base size: %d by %d",
		       size_hints->base_width, size_hints->base_height);
	if (size_hints->flags & PWinGravity)
		printf("\n\t\twindow gravity: %s",
		       window_gravities[size_hints->win_gravity]);
}

static void
print_wm_hints(XWMHints *hints)
{
	if (hints->flags & InputHint)
		printf("\n\t\tclient accepts input or output focus: %s",
		       hints->input ? "True" : "False");
	if (hints->flags & StateInput)
		printf("\n\t\tinitial state is %s",
		       initial_states[hints->initial_state]);
	if (hints->flags & IconPixmapHint)
		printf("\n\t\tbitmap id # to use for icon: 0x%x",
		       hints->icon_pixmap);
	if (hints->flags & IconWindowHint)
		printf("\n\t\twindow id # to use for icon: 0x%x",
		       hints->icon_window);
	if (hints->flags & IconPositionHint)
		printf("\n\t\tstarting position for icon: %d %d",
		       hints->icon_x, hints->icon_y);
	if (hints->flags & IconMaskHint)
		printf("\n\t\tbitmap id # of mask for icon: 0x%x",
		       hints->icon_mask);
	if (hints->flags & WindowGroupHint)
		printf("\n\t\twindow id # of group leader: 0x%x",
		       hints->window_group);
	if (hints->flags & UrgencyHint)
		printf("\n\t\tThe urgency hint bit is set");
}

static void
print_strings(char *strings, unsigned long nitems)
{
	printf(" = \"");
	for (unsigned j = 0; j < nitems; ++j)
	{
		char c = strings[j];
		if (!c && (j != nitems - 1))
		{
			printf("\", \"");
			continue;
		}
		if (c == '\n')
			printf("\\n");
		else if (c == '\t')
			printf("\\t");
		else
			printf("%c", c);
	}
	printf("\"");
}

static void
print_wm_icon(uint32_t *data, unsigned long nitems)
{
	static const char chars[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";

	while (nitems >= 2)
	{
		uint32_t width = data[0];
		uint32_t height = data[1];
		data += 2;
		nitems -= 2;
		printf("\tIcon (%" PRIu32 " x %" PRIu32 "):\n", width, height);
		if (width * height > nitems)
			break;
		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				uint32_t pixel = *(data++);
				uint8_t lumi = ((((pixel >> 16) & 0xFF) * 76 / 255)
				              + (((pixel >> 8) & 0xFF) * 150 / 255)
				              + (((pixel >> 0) & 0xFF)) * 29 / 255);
				if (((pixel >> 24) & 0xFF) >= 0x80)
					putchar(chars[(uint32_t)(255 - lumi) * 68 / 255]);
				else
					printf(" ");
			}
			printf("\n");
		}
		nitems -= width * height;
	}
}

int
main(int argc, char **argv)
{
	Display *display;
	Window window;
	Atom *properties;
	char **names;
	int nprops;

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
	properties = XListProperties(display, window, &nprops);
	if (!properties)
	{
		fprintf(stderr, "%s: failed to list properties\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!nprops)
		return EXIT_SUCCESS;
	names = malloc(sizeof(*names) * nprops);
	if (!names)
	{
		fprintf(stderr, "%s: malloc: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (!XGetAtomNames(display, properties, nprops, names))
	{
		fprintf(stderr, "%s: failed to get atom names\n", argv[0]);
		return EXIT_FAILURE;
	}
	/* XXX xcb might be a god-tier solution here */
	for (int i = 0; i < nprops; ++i)
	{
		char *name = names[i];
		Atom type;
		int format;
		unsigned long nitems;
		unsigned long bytes_after;
		uint8_t *prop;

		if (XGetWindowProperty(display,
		                       window,
		                       properties[i],
		                       0,
		                       0,
		                       False,
		                       AnyPropertyType,
		                       &type,
		                       &format,
		                       &nitems,
		                       &bytes_after,
		                       &prop)
		 || XGetWindowProperty(display,
		                       window,
		                       properties[i],
		                       0,
		                       (bytes_after + 3) / 4,
		                       False,
		                       type,
		                       &type,
		                       &format,
		                       &nitems,
		                       &bytes_after,
		                       &prop))
		{
			printf("%s\n", name);
			free(name);
			continue;
		}
		char *type_name = atom_name(display, type);
		printf("%s(%s)", name, type_name);
		switch (type)
		{
			case XA_WINDOW:
				if (format != 32)
					break;
				printf(": window id #");
				for (unsigned j = 0; j < nitems; ++j)
				{
					if (j)
						printf(",");
					printf(" 0x%" PRIx32, ((uint32_t*)prop)[j]);
				}
				break;
			case XA_CARDINAL:
				if (format == 8)
				{
					printf(" =");
					for (unsigned j = 0; j < nitems; ++j)
					{
						if (j)
							printf(",");
						printf(" %" PRIu8, ((uint8_t*)prop)[j]);
					}
				}
				else if (format == 16)
				{
					printf(" =");
					for (unsigned j = 0; j < nitems; ++j)
					{
						if (j)
							printf(",");
						printf(" %" PRIu16, ((uint16_t*)prop)[j]);
					}
				}
				else if (format == 32)
				{
					printf(" =");
					if (!strcmp(name, "_NET_WM_ICON"))
					{
						printf(" ");
						print_wm_icon((uint32_t*)prop, nitems);
						break;
					}
					for (unsigned j = 0; j < nitems; ++j)
					{
						if (j)
							printf(",");
						printf(" %" PRIu32, ((uint32_t*)prop)[j]);
					}
				}
				break;
			case XA_INTEGER:
				if (format == 8)
				{
					printf(" =");
					for (unsigned j = 0; j < nitems; ++j)
					{
						if (j)
							printf(",");
						printf(" %" PRId8, ((int8_t*)prop)[j]);
					}
				}
				else if (format == 16)
				{
					printf(" =");
					for (unsigned j = 0; j < nitems; ++j)
					{
						if (j)
							printf(",");
						printf(" %" PRId16, ((int16_t*)prop)[j]);
					}
				}
				else if (format == 32)
				{
					printf(" =");
					for (unsigned j = 0; j < nitems; ++j)
					{
						if (j)
							printf(",");
						printf(" %" PRId32, ((int32_t*)prop)[j]);
					}
				}
				break;
			case XA_ATOM:
			{
				if (format != 32)
					break;
				printf(" =");
				for (unsigned j = 0; j < nitems; ++j)
				{
					char *prop_name = atom_name(display, ((uint32_t*)prop)[j]);
					if (j)
						printf(",");
					printf(" %s", prop_name);
					free(prop_name);
				}
				break;
			}
			case XA_STRING:
				if (format != 8)
					break;
				print_strings((char*)prop, nitems);
				break;
			case XA_WM_SIZE_HINTS:
			{
				if (format != 32 || nitems < 18)
					break;
				print_size_hints((XSizeHints*)prop);
				break;
			}
			case XA_WM_HINTS:
			{
				if (format != 32 || nitems < 9)
					break;
				print_wm_hints((XWMHints*)prop);
				break;
			}
			default:
				if (!strcmp(type_name, "WM_STATE"))
				{
					if (format != 32 || nitems < 2)
						break;
					printf("\n\t\twindow state: %s",
					       window_states[((uint32_t*)prop)[0]]);
					printf("\n\t\ticon window: 0x%" PRIx32, ((uint32_t*)prop)[1]);
					break;
				}
				if (!strcmp(type_name, "UTF8_STRING"))
				{
					if (format != 8)
						break;
					print_strings((char*)prop, nitems);
					break;
				}
				if (!strcmp(type_name, "COMPOUND_TEXT"))
				{
					if (format != 8)
						break;
					print_strings((char*)prop, nitems);
					break;
				}
				break;
		}
		printf("\n");
		free(name);
		free(type_name);
	}
	return EXIT_SUCCESS;
}
