#include "render/render.h"
#include "shm/shm.h"
#include "xsrv.h"

#ifdef __eklat__
#include <eklat/evdev.h>
#endif

#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/X.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#if 0
#define DEBUG_TIMING
#endif

extern uint8_t font8x8[256][8];

struct xcursor
{
	uint8_t width;
	uint8_t height;
	uint8_t xhot;
	uint8_t yhot;
	uint8_t data[32];
};

extern const struct xcursor *xcursors[154];

static int setup_sock(struct xsrv *xsrv)
{
	xsrv->sock = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (xsrv->sock == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	struct sockaddr_un sockaddr;
	sockaddr.sun_family = AF_LOCAL;
	snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path),
	         "/tmp/.X11-unix/X%d", 5);
	unlink(sockaddr.sun_path);
	if (bind(xsrv->sock, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1)
	{
		fprintf(stderr, "%s: bind: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	if (listen(xsrv->sock, 128) == -1)
	{
		fprintf(stderr, "%s: listen: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int handle_new_client(struct xsrv *xsrv)
{
	struct sockaddr_un caddr;
	socklen_t caddrlen = sizeof(caddr);
	int fd = accept(xsrv->sock, (struct sockaddr*)&caddr, &caddrlen);
	if (fd == -1)
	{
		if (errno == EINTR || errno == EAGAIN)
			return 0;
		fprintf(stderr, "%s: accept: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		fprintf(stderr, "%s: fcntl(F_GETFL): %s\n", xsrv->progname,
		        strerror(errno));
		close(fd);
		return 1;
	}
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1)
	{
		fprintf(stderr, "%s: fcntl(F_SETFL): %s\n", xsrv->progname,
		        strerror(errno));
		close(fd);
		return 1;
	}
	struct client *client = client_new(xsrv, fd);
	if (!client)
	{
		close(fd);
		return 1;
	}
	TAILQ_INSERT_TAIL(&xsrv->clients, client, chain);
	return 0;
}

static struct format *create_format(struct xsrv *xsrv, uint8_t depth,
                                    uint8_t bpp, uint8_t scanline_pad)
{
	struct format *format = malloc(sizeof(*format));
	if (!format)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	format->depth = depth;
	format->bpp = bpp;
	format->scanline_pad = scanline_pad;
	return format;
}

static int setup_formats(struct xsrv *xsrv)
{
	size_t formats_count = 7;
	struct format **formats = calloc(sizeof(*formats), formats_count);
	if (!formats)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	formats[0] = create_format(xsrv, 1, 1, 32);
	formats[1] = create_format(xsrv, 4, 8, 32);
	formats[2] = create_format(xsrv, 8, 8, 32);
	formats[3] = create_format(xsrv, 15, 16, 32);
	formats[4] = create_format(xsrv, 16, 16, 32);
	formats[5] = create_format(xsrv, 24, 32, 32);
	formats[6] = create_format(xsrv, 32, 32, 32);
	if (!formats[0]
	 || !formats[1]
	 || !formats[2]
	 || !formats[3]
	 || !formats[4]
	 || !formats[5]
	 || !formats[6])
		goto err;
	xsrv->formats_count = formats_count;
	xsrv->formats = formats;
	return 0;

err:
	for (size_t i = 0; i < formats_count; ++i)
		free(formats[i]);
	free(formats);
	return 1;
}

const struct format *xsrv_get_format(struct xsrv *xsrv, uint8_t depth)
{
	for (size_t i = 0; i < xsrv->formats_count; ++i)
	{
		if (xsrv->formats[i]->depth == depth)
			return xsrv->formats[i];
	}
	return NULL;
}

static struct visual *create_visual(struct xsrv *xsrv, uint32_t id,
                                    uint8_t class, uint8_t bpp,
                                    uint16_t colormap_entries,
                                    uint32_t red_mask,
                                    uint32_t green_mask,
                                    uint32_t blue_mask)
{
	struct visual *visual = malloc(sizeof(*visual));
	if (!visual)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	visual->id = id;
	visual->class = class;
	visual->bpp = bpp;
	visual->colormap_entries = colormap_entries;
	visual->red_mask = red_mask;
	visual->green_mask = green_mask;
	visual->blue_mask = blue_mask;
	return visual;
}

static struct depth *create_depth(struct xsrv *xsrv, uint8_t value,
                                  uint16_t visuals_count,
                                  struct visual **visuals)
{
	struct depth *depth = malloc(sizeof(*depth));
	if (!depth)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	depth->depth = value;
	depth->visuals_count = visuals_count;
	depth->visuals = visuals;
	return depth;
}

static int setup_screens(struct xsrv *xsrv)
{
	struct visual **visuals_24 = NULL;
	struct visual **visuals_32 = NULL;
	struct screen **screens = NULL;
	struct depth **depths = NULL;
	size_t visuals_24_count;
	size_t visuals_32_count;
	size_t screens_count;
	size_t depths_count;
	visuals_24_count = 1;
	visuals_24 = calloc(sizeof(*visuals_24), visuals_24_count);
	if (!visuals_24)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		goto err;
	}
	visuals_24[0] = create_visual(xsrv, 1, TrueColor, 8, 256,
	                           0xFF0000, 0xFF00, 0xFF);
	if (!visuals_24[0])
		goto err;
	visuals_32_count = 1;
	visuals_32 = calloc(sizeof(*visuals_32), visuals_32_count);
	if (!visuals_32)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		goto err;
	}
	visuals_32[0] = create_visual(xsrv, 2, TrueColor, 8, 256,
	                           0xFF0000, 0xFF00, 0xFF);
	if (!visuals_32[0])
		goto err;
	depths_count = 7;
	depths = calloc(sizeof(*depths), depths_count);
	if (!depths)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		goto err;
	}
	depths[0] = create_depth(xsrv, 1, 0, NULL);
	depths[1] = create_depth(xsrv, 4, 0, NULL);
	depths[2] = create_depth(xsrv, 8, 0, NULL);
	depths[3] = create_depth(xsrv, 15, 0, NULL);
	depths[4] = create_depth(xsrv, 16, 0, NULL);
	depths[5] = create_depth(xsrv, 24, visuals_24_count, visuals_24);
	depths[6] = create_depth(xsrv, 32, visuals_32_count, visuals_32);
	if (!depths[0]
	 || !depths[1]
	 || !depths[2]
	 || !depths[3]
	 || !depths[4]
	 || !depths[5]
	 || !depths[6])
		goto err;
	screens_count = 1;
	screens = calloc(sizeof(*screens), screens_count);
	if (!screens)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		goto err;
	}
	screens[0] = malloc(sizeof(**screens));
	if (!screens[0])
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		goto err;
	}
	struct screen *screen = screens[0];
	struct window_attributes attributes;
	attributes.background_pixmap = NULL;
	attributes.background_pixel = 0;
	attributes.border_pixmap = NULL;
	attributes.border_pixel = 0;
	attributes.bit_gravity = ForgetGravity;
	attributes.win_gravity = ForgetGravity;
	attributes.backing_store = NotUseful;
	attributes.backing_planes = -1;
	attributes.backing_pixel = 0;
	attributes.override_redirect = 0;
	attributes.save_under = 0;
	attributes.event_mask = 0;
	attributes.do_not_propagate_mask = 0;
	attributes.colormap = NULL;
	attributes.cursor = NULL;
	screen->root = window_new(xsrv, NULL, xsrv_allocate_id(xsrv), NULL, 0,
	                          0, xsrv->backend.width, xsrv->backend.height,
	                          xsrv_get_format(xsrv, 24), 0, visuals_24[0],
	                          InputOutput, &attributes);
	if (!screen->root)
	{
		fprintf(stderr, "%s: failed to create root window\n",
		        xsrv->progname);
		goto err;
	}
	screen->root->screen = screen;
	screen->root->flags |= WINDOW_MAPPED;
	screen->root->drawable.root = screen->root;
	object_add(xsrv, &screen->root->object);
	screen->colormap = colormap_new(xsrv, NULL, xsrv_allocate_id(xsrv));
	if (!screen->colormap)
	{
		object_free(xsrv, OBJECT(screen->root));
		fprintf(stderr, "%s: failed to create root colormap\n",
		        xsrv->progname);
		goto err;
	}
	screen->white_pixel = 0xFFFFFF;
	screen->black_pixel = 0x000000;
	screen->input_mask = KeyPressMask | KeyReleaseMask | EnterWindowMask
	                   | LeaveWindowMask | StructureNotifyMask
	                   | SubstructureNotifyMask | SubstructureRedirectMask
	                   | PropertyChangeMask | ColormapChangeMask;
	screen->width = xsrv->backend.width;
	screen->height = xsrv->backend.height;
	screen->mm_width = 530; /* XXX */
	screen->mm_height = 301; /* XXX */
	screen->min_maps = 1; /* XXX */
	screen->max_maps = 1; /* XXX */
	screen->visual = depths[5]->visuals[0];
	screen->backing_stores = WhenMapped;
	screen->save_unders = 0;
	screen->root_depth = depths[5]->depth;
	screen->depths_count = depths_count;
	screen->depths = depths;
	xsrv->screens = screens;
	xsrv->screens_count = screens_count;
	return 0;

err:
	if (depths)
	{
		for (size_t i = 0; i < depths_count; ++i)
			free(depths[i]);
		free(depths);
	}
	if (visuals_24)
	{
		for (size_t i = 0; i < visuals_24_count; ++i)
			free(visuals_24[i]);
		free(visuals_24);
	}
	if (visuals_32)
	{
		for (size_t i = 0; i < visuals_32_count; ++i)
			free(visuals_32[i]);
		free(visuals_32);
	}
	if (screens)
	{
		for (size_t i = 0; i < screens_count; ++i)
			free(screens[i]);
		free(screens);
	}
	return 1;
}

static int setup_cursor(struct xsrv *xsrv)
{
	struct pixmap *color = pixmap_new(xsrv, NULL, xsrv_allocate_id(xsrv),
	                                  32, 16, xsrv_get_format(xsrv, 1),
	                                  xsrv->screens[0]->root);
	if (!color)
	{
		fprintf(stderr, "%s: failed to create default cursor color\n",
		        xsrv->progname);
		return 1;
	}
	uint8_t colors[] =
	{
		0x03, 0x00, 0x00, 0x00,
		0x05, 0x00, 0x00, 0x00,
		0x09, 0x00, 0x00, 0x00,
		0x11, 0x00, 0x00, 0x00,
		0x21, 0x00, 0x00, 0x00,
		0x41, 0x00, 0x00, 0x00,
		0x81, 0x00, 0x00, 0x00,
		0x01, 0x01, 0x00, 0x00,
		0x01, 0x02, 0x00, 0x00,
		0x01, 0x04, 0x00, 0x00,
		0x01, 0x03, 0x00, 0x00,
		0x81, 0x00, 0x00, 0x00,
		0x99, 0x00, 0x00, 0x00,
		0x27, 0x01, 0x00, 0x00,
		0x20, 0x01, 0x00, 0x00,
		0xC0, 0x00, 0x00, 0x00,
	};
	memcpy(color->drawable.data, colors, sizeof(colors));
	struct pixmap *mask = pixmap_new(xsrv, NULL, xsrv_allocate_id(xsrv),
	                                 32, 16, xsrv_get_format(xsrv, 1),
	                                 xsrv->screens[0]->root);
	if (!mask)
	{
		fprintf(stderr, "%s: failed to create default cursor mask\n",
		        xsrv->progname);
		return 1;
	}
	uint8_t masks[] =
	{
		0x03, 0x00, 0x00, 0x00,
		0x07, 0x00, 0x00, 0x00,
		0x0F, 0x00, 0x00, 0x00,
		0x1F, 0x00, 0x00, 0x00,
		0x3F, 0x00, 0x00, 0x00,
		0x7F, 0x00, 0x00, 0x00,
		0xFF, 0x00, 0x00, 0x00,
		0xFF, 0x01, 0x00, 0x00,
		0xFF, 0x03, 0x00, 0x00,
		0xFF, 0x07, 0x00, 0x00,
		0xFF, 0x03, 0x00, 0x00,
		0xFF, 0x00, 0x00, 0x00,
		0xFF, 0x00, 0x00, 0x00,
		0xE7, 0x01, 0x00, 0x00,
		0xE0, 0x01, 0x00, 0x00,
		0xC0, 0x00, 0x00, 0x00,
	};
	memcpy(mask->drawable.data, masks, sizeof(masks));
	struct cursor *cursor = cursor_new(xsrv, NULL, xsrv_allocate_id(xsrv),
	                                   color, mask, 0xFFFF, 0xFFFF, 0xFFFF,
	                                   0, 0, 0, 0, 0, 0, 0);
	if (!cursor)
	{
		fprintf(stderr, "%s: failed to create default cursor\n",
		        xsrv->progname);
		return 1;
	}
	xsrv->screens[0]->root->attributes.cursor = cursor;
	return 0;
}

static struct extension *create_extension(struct xsrv *xsrv, const char *name,
                                          uint8_t major_opcode,
                                          uint8_t first_event,
                                          uint8_t first_error)
{
	struct extension *extension = malloc(sizeof(*extension));
	if (!extension)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	extension->name = strdup(name);
	if (!extension->name)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		free(extension);
		return NULL;
	}
	extension->name_len = strlen(extension->name);
	extension->major_opcode = major_opcode;
	extension->first_event = first_event;
	extension->first_error = first_error;
	return extension;
}

struct extension *register_extension(struct xsrv *xsrv, const char *name)
{
	struct extension *extension = create_extension(xsrv, name,
	                                               128 + xsrv->extensions_count,
	                                               64 + xsrv->extensions_count,
	                                               128 + xsrv->extensions_count);
	if (!extension)
		return NULL;
	struct extension **extensions = realloc(xsrv->extensions,
	                                        sizeof(*extensions)
	                                      * (xsrv->extensions_count + 1));
	if (!extensions)
	{
		free(extension->name);
		free(extension);
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	extensions[xsrv->extensions_count++] = extension;
	xsrv->extensions = extensions;
	return extension;
}

uint8_t register_object_type(struct xsrv *xsrv, const struct object_def *def)
{
	for (size_t i = 0; i < sizeof(xsrv->objects_defs) / sizeof(*xsrv->objects_defs); ++i)
	{
		if (xsrv->objects_defs[i])
			continue;
		xsrv->objects_defs[i] = def;
		return i;
	}
	fprintf(stderr, "%s: no more object type id available\n", xsrv->progname);
	abort();
}

struct object *object_get_typed(struct xsrv *xsrv, uint32_t id, uint8_t type)
{
	struct object *object = object_get(xsrv, id);
	if (!object)
		return NULL;
	if (object->type != type)
	{
		object->refs--;
		return NULL;
	}
	return object;
}

static struct font_def *create_font_def(struct xsrv *xsrv, const char *name)
{
	struct font_def *font = calloc(sizeof(*font), 1);
	if (!font)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	font->name = strdup(name);
	if (!font->name)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		free(font);
		return NULL;
	}
	font->name_len = strlen(name);
	return font;
}

static int setup_8x16(struct xsrv *xsrv, struct font_def *font)
{
	for (size_t i = 0; i < 256; ++i)
	{
		font->glyphs[i] = malloc(sizeof(**font->glyphs));
		if (!font->glyphs[i])
		{
			fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
			        strerror(errno));
			return 1;
		}
		font->glyphs[i]->left_bearing = 0;
		font->glyphs[i]->right_bearing = 8;
		font->glyphs[i]->width = 8;
		font->glyphs[i]->ascent = 16;
		font->glyphs[i]->descent = 0;
		font->glyphs[i]->pixmap = pixmap_new(xsrv, NULL,
		                                     xsrv_allocate_id(xsrv),
		                                     8, 16,
		                                     xsrv_get_format(xsrv, 1),
		                                     xsrv->screens[0]->root);
		if (!font->glyphs[i]->pixmap)
		{
			free(font->glyphs[i]);
			font->glyphs[i] = NULL;
			for (size_t j = 0; j < i; ++j)
			{
				object_destroy(xsrv, OBJECT(font->glyphs[j]->pixmap));
				object_free(xsrv, OBJECT(font->glyphs[j]->pixmap));
				free(font->glyphs[j]);
				font->glyphs[j] = NULL;
			}
			return 1;
		}
		uint8_t *ptr = font->glyphs[i]->pixmap->drawable.data;
		for (size_t y = 0; y < 16; ++y)
		{
			*ptr = font8x8[i][y / 2];
			ptr += font->glyphs[i]->pixmap->drawable.pitch;
		}
	}
	return 0;
}

static int setup_cursor_font(struct xsrv *xsrv, struct font_def *font)
{
	for (size_t i = 0; i < 154; ++i)
	{
		const struct xcursor *xcursor = xcursors[i];
		if (!xcursor)
			continue;
		font->glyphs[i] = malloc(sizeof(**font->glyphs));
		if (!font->glyphs[i])
		{
			fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
			        strerror(errno));
			return 1;
		}
		font->glyphs[i]->left_bearing = -(int16_t)xcursor->xhot;
		font->glyphs[i]->right_bearing = xcursor->width - xcursor->xhot;
		font->glyphs[i]->width = xcursor->width;
		font->glyphs[i]->ascent = xcursor->yhot;
		font->glyphs[i]->descent = xcursor->height - xcursor->yhot;
		font->glyphs[i]->pixmap = pixmap_new(xsrv, NULL,
		                                     xsrv_allocate_id(xsrv),
		                                     xcursor->width,
		                                     xcursor->height,
		                                     xsrv_get_format(xsrv, 1),
		                                     xsrv->screens[0]->root);
		if (!font->glyphs[i]->pixmap)
		{
			free(font->glyphs[i]);
			font->glyphs[i] = NULL;
			for (size_t j = 0; j < i; ++j)
			{
				object_destroy(xsrv, OBJECT(font->glyphs[j]->pixmap));
				object_free(xsrv, OBJECT(font->glyphs[j]->pixmap));
				free(font->glyphs[j]);
				font->glyphs[j] = NULL;
			}
			return 1;
		}
		uint8_t *ptr = font->glyphs[i]->pixmap->drawable.data;
		for (size_t y = 0; y < xcursor->height; ++y)
		{
			ptr[0] = xcursor->data[y * 2 + 0];
			ptr[1] = xcursor->data[y * 2 + 1];
			ptr += font->glyphs[i]->pixmap->drawable.pitch;
		}
	}
	return 0;
}

static int setup_fonts(struct xsrv *xsrv)
{
	size_t fonts_count = 2;
	struct font_def **fonts = calloc(sizeof(*fonts), fonts_count);
	if (!fonts)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	fonts[0] = create_font_def(xsrv, "cursor");
	fonts[1] = create_font_def(xsrv, "8x16");
	if (!fonts[0]
	 || !fonts[1])
		goto err;
	if (setup_8x16(xsrv, fonts[1]))
		goto err;
	if (setup_cursor_font(xsrv, fonts[0]))
		goto err;
	xsrv->fonts = fonts;
	xsrv->fonts_count = fonts_count;
	return 0;

err:
	for (size_t i = 0; i < fonts_count; ++i)
	{
		if (!fonts[i])
			continue;
		free(fonts[i]->name);
		free(fonts[i]);
	}
	free(fonts);
	return 1;
}

static int setup_object_types(struct xsrv *xsrv)
{
	window_register(xsrv);
	colormap_register(xsrv);
	pixmap_register(xsrv);
	cursor_register(xsrv);
	font_register(xsrv);
	gcontext_register(xsrv);
	return 0;
}

static int setup_objects_map(struct xsrv *xsrv)
{
	xsrv->objects_map_size = 1024;
	xsrv->objects_map = malloc(sizeof(*xsrv->objects_map)
	                         * xsrv->objects_map_size);
	if (!xsrv->objects_map)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	for (size_t i = 0; i < xsrv->objects_map_size; ++i)
		TAILQ_INIT(&xsrv->objects_map[i]);
	return 0;
}

static int setup_focus(struct xsrv *xsrv)
{
	xsrv->focus.window = NULL;
	xsrv->focus.timestamp = millitime();
	xsrv->focus.revert_to = None;
	return 0;
}

static int setup_pointer(struct xsrv *xsrv)
{
	uint8_t map_len = sizeof(xsrv->pointer.map) / sizeof(*xsrv->pointer.map);
	for (size_t i = 0; i < map_len; ++i)
	{
		xsrv->pointer.map[i] = i + 1;
		xsrv->pointer.state[i] = 0;
	}
	xsrv->pointer.accel_numerator = 0;
	xsrv->pointer.accel_denominator = 1;
	xsrv->pointer.threshold = 0;
	xsrv->pointer.do_accel = 0;
	xsrv->pointer.do_threshold = 0;
	xsrv->pointer.x = 0;
	xsrv->pointer.y = 0;
	xsrv->pointer.cursor = xsrv->screens[0]->root->attributes.cursor;
	xsrv->pointer.cursor->object.refs++;
	xsrv->pointer.window = xsrv->screens[0]->root;
	return 0;
}

static int setup_keyboard(struct xsrv *xsrv)
{
	xsrv->keyboard.min_keycode = 8;
	xsrv->keyboard.max_keycode = 255;
	xsrv->keyboard.syms_per_code = 8;
	xsrv->keyboard.codes_per_modifier = 8;
#ifdef __eklat__
	xsrv->keyboard.modifiers[ControlMapIndex][0] = KBD_KEY_LCONTROL + 8;
	xsrv->keyboard.modifiers[ControlMapIndex][1] = KBD_KEY_RCONTROL + 8;
	xsrv->keyboard.modifiers[LockMapIndex][0] = KBD_KEY_CAPS_LOCK + 8;
	xsrv->keyboard.modifiers[ShiftMapIndex][0] = KBD_KEY_LSHIFT + 8;
	xsrv->keyboard.modifiers[ShiftMapIndex][1] = KBD_KEY_RSHIFT + 8;
	xsrv->keyboard.modifiers[Mod1MapIndex][0] = KBD_KEY_LALT + 8;
	xsrv->keyboard.modifiers[Mod1MapIndex][1] = KBD_KEY_RALT + 8;
	xsrv->keyboard.syms[8 + KBD_KEY_ESCAPE][0] = XK_Escape;
	xsrv->keyboard.syms[8 + KBD_KEY_0][0] = XK_0;
	xsrv->keyboard.syms[8 + KBD_KEY_0][1] = XK_parenright;
	xsrv->keyboard.syms[8 + KBD_KEY_1][0] = XK_1;
	xsrv->keyboard.syms[8 + KBD_KEY_1][1] = XK_exclam;
	xsrv->keyboard.syms[8 + KBD_KEY_2][0] = XK_2;
	xsrv->keyboard.syms[8 + KBD_KEY_2][1] = XK_at;
	xsrv->keyboard.syms[8 + KBD_KEY_3][0] = XK_3;
	xsrv->keyboard.syms[8 + KBD_KEY_3][1] = XK_numbersign;
	xsrv->keyboard.syms[8 + KBD_KEY_4][0] = XK_4;
	xsrv->keyboard.syms[8 + KBD_KEY_4][1] = XK_dollar;
	xsrv->keyboard.syms[8 + KBD_KEY_5][0] = XK_5;
	xsrv->keyboard.syms[8 + KBD_KEY_5][1] = XK_percent;
	xsrv->keyboard.syms[8 + KBD_KEY_6][0] = XK_6;
	xsrv->keyboard.syms[8 + KBD_KEY_6][1] = XK_asciicircum;
	xsrv->keyboard.syms[8 + KBD_KEY_7][0] = XK_7;
	xsrv->keyboard.syms[8 + KBD_KEY_7][1] = XK_ampersand;
	xsrv->keyboard.syms[8 + KBD_KEY_8][0] = XK_8;
	xsrv->keyboard.syms[8 + KBD_KEY_8][1] = XK_asterisk;
	xsrv->keyboard.syms[8 + KBD_KEY_9][0] = XK_9;
	xsrv->keyboard.syms[8 + KBD_KEY_9][1] = XK_parenleft;
	xsrv->keyboard.syms[8 + KBD_KEY_MINUS][0] = XK_minus;
	xsrv->keyboard.syms[8 + KBD_KEY_MINUS][1] = XK_underscore;
	xsrv->keyboard.syms[8 + KBD_KEY_EQUAL][0] = XK_equal;
	xsrv->keyboard.syms[8 + KBD_KEY_EQUAL][1] = XK_plus;
	xsrv->keyboard.syms[8 + KBD_KEY_BACKSPACE][0] = XK_Backspace;
	xsrv->keyboard.syms[8 + KBD_KEY_TAB][0] = XK_Tab;
	xsrv->keyboard.syms[8 + KBD_KEY_A][0] = XK_a;
	xsrv->keyboard.syms[8 + KBD_KEY_A][1] = XK_A;
	xsrv->keyboard.syms[8 + KBD_KEY_B][0] = XK_b;
	xsrv->keyboard.syms[8 + KBD_KEY_B][1] = XK_B;
	xsrv->keyboard.syms[8 + KBD_KEY_C][0] = XK_c;
	xsrv->keyboard.syms[8 + KBD_KEY_C][1] = XK_C;
	xsrv->keyboard.syms[8 + KBD_KEY_D][0] = XK_d;
	xsrv->keyboard.syms[8 + KBD_KEY_D][1] = XK_D;
	xsrv->keyboard.syms[8 + KBD_KEY_E][0] = XK_e;
	xsrv->keyboard.syms[8 + KBD_KEY_E][1] = XK_E;
	xsrv->keyboard.syms[8 + KBD_KEY_F][0] = XK_f;
	xsrv->keyboard.syms[8 + KBD_KEY_F][1] = XK_F;
	xsrv->keyboard.syms[8 + KBD_KEY_G][0] = XK_g;
	xsrv->keyboard.syms[8 + KBD_KEY_G][1] = XK_G;
	xsrv->keyboard.syms[8 + KBD_KEY_H][0] = XK_h;
	xsrv->keyboard.syms[8 + KBD_KEY_H][1] = XK_H;
	xsrv->keyboard.syms[8 + KBD_KEY_I][0] = XK_i;
	xsrv->keyboard.syms[8 + KBD_KEY_I][1] = XK_I;
	xsrv->keyboard.syms[8 + KBD_KEY_J][0] = XK_j;
	xsrv->keyboard.syms[8 + KBD_KEY_J][1] = XK_J;
	xsrv->keyboard.syms[8 + KBD_KEY_K][0] = XK_k;
	xsrv->keyboard.syms[8 + KBD_KEY_K][1] = XK_K;
	xsrv->keyboard.syms[8 + KBD_KEY_L][0] = XK_l;
	xsrv->keyboard.syms[8 + KBD_KEY_L][1] = XK_L;
	xsrv->keyboard.syms[8 + KBD_KEY_M][0] = XK_m;
	xsrv->keyboard.syms[8 + KBD_KEY_M][1] = XK_M;
	xsrv->keyboard.syms[8 + KBD_KEY_N][0] = XK_n;
	xsrv->keyboard.syms[8 + KBD_KEY_N][1] = XK_N;
	xsrv->keyboard.syms[8 + KBD_KEY_O][0] = XK_o;
	xsrv->keyboard.syms[8 + KBD_KEY_O][1] = XK_O;
	xsrv->keyboard.syms[8 + KBD_KEY_P][0] = XK_p;
	xsrv->keyboard.syms[8 + KBD_KEY_P][1] = XK_P;
	xsrv->keyboard.syms[8 + KBD_KEY_Q][0] = XK_q;
	xsrv->keyboard.syms[8 + KBD_KEY_Q][1] = XK_Q;
	xsrv->keyboard.syms[8 + KBD_KEY_R][0] = XK_r;
	xsrv->keyboard.syms[8 + KBD_KEY_R][1] = XK_R;
	xsrv->keyboard.syms[8 + KBD_KEY_S][0] = XK_s;
	xsrv->keyboard.syms[8 + KBD_KEY_S][1] = XK_S;
	xsrv->keyboard.syms[8 + KBD_KEY_T][0] = XK_t;
	xsrv->keyboard.syms[8 + KBD_KEY_T][1] = XK_T;
	xsrv->keyboard.syms[8 + KBD_KEY_U][0] = XK_u;
	xsrv->keyboard.syms[8 + KBD_KEY_U][1] = XK_U;
	xsrv->keyboard.syms[8 + KBD_KEY_V][0] = XK_v;
	xsrv->keyboard.syms[8 + KBD_KEY_V][1] = XK_V;
	xsrv->keyboard.syms[8 + KBD_KEY_W][0] = XK_w;
	xsrv->keyboard.syms[8 + KBD_KEY_W][1] = XK_W;
	xsrv->keyboard.syms[8 + KBD_KEY_X][0] = XK_x;
	xsrv->keyboard.syms[8 + KBD_KEY_X][1] = XK_X;
	xsrv->keyboard.syms[8 + KBD_KEY_Y][0] = XK_y;
	xsrv->keyboard.syms[8 + KBD_KEY_Y][1] = XK_Y;
	xsrv->keyboard.syms[8 + KBD_KEY_Z][0] = XK_z;
	xsrv->keyboard.syms[8 + KBD_KEY_Z][1] = XK_Z;
	xsrv->keyboard.syms[8 + KBD_KEY_LBRACKET][0] = XK_bracketleft;
	xsrv->keyboard.syms[8 + KBD_KEY_LBRACKET][1] = XK_braceleft;
	xsrv->keyboard.syms[8 + KBD_KEY_RBRACKET][0] = XK_bracketright;
	xsrv->keyboard.syms[8 + KBD_KEY_RBRACKET][1] = XK_braceright;
	xsrv->keyboard.syms[8 + KBD_KEY_ENTER][0] = XK_Return;
	xsrv->keyboard.syms[8 + KBD_KEY_SEMICOLON][0] = XK_semicolon;
	xsrv->keyboard.syms[8 + KBD_KEY_SEMICOLON][1] = XK_colon;
	xsrv->keyboard.syms[8 + KBD_KEY_SQUOTE][0] = XK_apostrophe;
	xsrv->keyboard.syms[8 + KBD_KEY_SQUOTE][1] = XK_quotedbl;
	xsrv->keyboard.syms[8 + KBD_KEY_TILDE][0] = XK_grave;
	xsrv->keyboard.syms[8 + KBD_KEY_TILDE][1] = XK_asciitilde;
	xsrv->keyboard.syms[8 + KBD_KEY_LCONTROL][0] = XK_Control_L;
	xsrv->keyboard.syms[8 + KBD_KEY_RCONTROL][0] = XK_Control_R;
	xsrv->keyboard.syms[8 + KBD_KEY_LSHIFT][0] = XK_Shift_L;
	xsrv->keyboard.syms[8 + KBD_KEY_RSHIFT][0] = XK_Shift_R;
	xsrv->keyboard.syms[8 + KBD_KEY_LALT][0] = XK_Alt_L;
	xsrv->keyboard.syms[8 + KBD_KEY_RALT][0] = XK_Alt_R;
	xsrv->keyboard.syms[8 + KBD_KEY_LMETA][0] = XK_Meta_L;
	xsrv->keyboard.syms[8 + KBD_KEY_RMETA][0] = XK_Meta_R;
	xsrv->keyboard.syms[8 + KBD_KEY_ANTISLASH][0] = XK_backslash;
	xsrv->keyboard.syms[8 + KBD_KEY_ANTISLASH][1] = XK_bar;
	xsrv->keyboard.syms[8 + KBD_KEY_COMMA][0] = XK_comma;
	xsrv->keyboard.syms[8 + KBD_KEY_COMMA][1] = XK_less;
	xsrv->keyboard.syms[8 + KBD_KEY_DOT][0] = XK_period;
	xsrv->keyboard.syms[8 + KBD_KEY_DOT][1] = XK_greater;
	xsrv->keyboard.syms[8 + KBD_KEY_SLASH][0] = XK_slash;
	xsrv->keyboard.syms[8 + KBD_KEY_SLASH][1] = XK_question;
	xsrv->keyboard.syms[8 + KBD_KEY_SPACE][0] = XK_space;
	xsrv->keyboard.syms[8 + KBD_KEY_CAPS_LOCK][0] = XK_Caps_Lock;
	xsrv->keyboard.syms[8 + KBD_KEY_F1][0] = XK_F1;
	xsrv->keyboard.syms[8 + KBD_KEY_F2][0] = XK_F2;
	xsrv->keyboard.syms[8 + KBD_KEY_F3][0] = XK_F3;
	xsrv->keyboard.syms[8 + KBD_KEY_F4][0] = XK_F4;
	xsrv->keyboard.syms[8 + KBD_KEY_F5][0] = XK_F5;
	xsrv->keyboard.syms[8 + KBD_KEY_F6][0] = XK_F6;
	xsrv->keyboard.syms[8 + KBD_KEY_F7][0] = XK_F7;
	xsrv->keyboard.syms[8 + KBD_KEY_F8][0] = XK_F8;
	xsrv->keyboard.syms[8 + KBD_KEY_F9][0] = XK_F9;
	xsrv->keyboard.syms[8 + KBD_KEY_F10][0] = XK_F10;
	xsrv->keyboard.syms[8 + KBD_KEY_F11][0] = XK_F11;
	xsrv->keyboard.syms[8 + KBD_KEY_F12][0] = XK_F12;
	xsrv->keyboard.syms[8 + KBD_KEY_NUM_LOCK][0] = XK_Num_Lock;
	xsrv->keyboard.syms[8 + KBD_KEY_SCROLL_LOCK][0] = XK_Scroll_Lock;
	xsrv->keyboard.syms[8 + KBD_KEY_HOME][0] = XK_Home;
	xsrv->keyboard.syms[8 + KBD_KEY_PGUP][0] = XK_Page_Up;
	xsrv->keyboard.syms[8 + KBD_KEY_PGDOWN][0] = XK_Page_Down;
	xsrv->keyboard.syms[8 + KBD_KEY_END][0] = XK_End;
	xsrv->keyboard.syms[8 + KBD_KEY_INSERT][0] = XK_Insert;
	xsrv->keyboard.syms[8 + KBD_KEY_DELETE][0] = XK_Delete;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_0][0] = XK_KP_0;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_1][0] = XK_KP_1;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_2][0] = XK_KP_2;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_3][0] = XK_KP_3;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_4][0] = XK_KP_4;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_5][0] = XK_KP_5;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_6][0] = XK_KP_6;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_7][0] = XK_KP_7;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_8][0] = XK_KP_8;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_9][0] = XK_KP_9;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_MULT][0] = XK_KP_Multiply;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_MINUS][0] = XK_KP_Subtract;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_PLUS][0] = XK_KP_Add;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_DOT][0] = XK_KP_Decimal;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_ENTER][0] = XK_KP_Enter;
	xsrv->keyboard.syms[8 + KBD_KEY_KP_SLASH][0] = XK_KP_Divide;
	xsrv->keyboard.syms[8 + KBD_KEY_CURSOR_UP][0] = XK_Up;
	xsrv->keyboard.syms[8 + KBD_KEY_CURSOR_LEFT][0] = XK_Left;
	xsrv->keyboard.syms[8 + KBD_KEY_CURSOR_RIGHT][0] = XK_Right;
	xsrv->keyboard.syms[8 + KBD_KEY_CURSOR_DOWN][0] = XK_Down;
#else
	xsrv->keyboard.modifiers[ControlMapIndex][0] = 37;
	xsrv->keyboard.modifiers[ControlMapIndex][1] = 105;
	xsrv->keyboard.modifiers[LockMapIndex][0] = 66;
	xsrv->keyboard.modifiers[ShiftMapIndex][0] = 50;
	xsrv->keyboard.modifiers[ShiftMapIndex][1] = 62;
	xsrv->keyboard.modifiers[Mod1MapIndex][0] = 64;
	xsrv->keyboard.modifiers[Mod1MapIndex][1] = 108;
	xsrv->keyboard.syms[9][0] = XK_Escape;
	xsrv->keyboard.syms[10][0] = XK_1;
	xsrv->keyboard.syms[10][1] = XK_exclam;
	xsrv->keyboard.syms[11][0] = XK_2;
	xsrv->keyboard.syms[11][1] = XK_at;
	xsrv->keyboard.syms[12][0] = XK_3;
	xsrv->keyboard.syms[12][1] = XK_numbersign;
	xsrv->keyboard.syms[13][0] = XK_4;
	xsrv->keyboard.syms[13][1] = XK_dollar;
	xsrv->keyboard.syms[14][0] = XK_5;
	xsrv->keyboard.syms[14][1] = XK_percent;
	xsrv->keyboard.syms[15][0] = XK_6;
	xsrv->keyboard.syms[15][1] = XK_asciicircum;
	xsrv->keyboard.syms[16][0] = XK_7;
	xsrv->keyboard.syms[16][1] = XK_ampersand;
	xsrv->keyboard.syms[17][0] = XK_8;
	xsrv->keyboard.syms[17][1] = XK_asterisk;
	xsrv->keyboard.syms[18][0] = XK_9;
	xsrv->keyboard.syms[18][1] = XK_parenleft;
	xsrv->keyboard.syms[19][0] = XK_0;
	xsrv->keyboard.syms[19][1] = XK_parenright;
	xsrv->keyboard.syms[20][0] = XK_minus;
	xsrv->keyboard.syms[20][1] = XK_underscore;
	xsrv->keyboard.syms[21][0] = XK_equal;
	xsrv->keyboard.syms[21][1] = XK_plus;
	xsrv->keyboard.syms[22][0] = XK_Backspace;
	xsrv->keyboard.syms[23][0] = XK_Tab;
	xsrv->keyboard.syms[24][0] = XK_q;
	xsrv->keyboard.syms[24][1] = XK_Q;
	xsrv->keyboard.syms[25][0] = XK_w;
	xsrv->keyboard.syms[25][1] = XK_W;
	xsrv->keyboard.syms[26][0] = XK_e;
	xsrv->keyboard.syms[26][1] = XK_R;
	xsrv->keyboard.syms[27][0] = XK_r;
	xsrv->keyboard.syms[27][1] = XK_R;
	xsrv->keyboard.syms[28][0] = XK_t;
	xsrv->keyboard.syms[28][1] = XK_T;
	xsrv->keyboard.syms[29][0] = XK_y;
	xsrv->keyboard.syms[29][1] = XK_Y;
	xsrv->keyboard.syms[30][0] = XK_u;
	xsrv->keyboard.syms[30][1] = XK_U;
	xsrv->keyboard.syms[31][0] = XK_i;
	xsrv->keyboard.syms[31][1] = XK_I;
	xsrv->keyboard.syms[32][0] = XK_o;
	xsrv->keyboard.syms[32][1] = XK_O;
	xsrv->keyboard.syms[33][0] = XK_p;
	xsrv->keyboard.syms[33][1] = XK_P;
	xsrv->keyboard.syms[34][0] = XK_bracketleft;
	xsrv->keyboard.syms[34][1] = XK_braceleft;
	xsrv->keyboard.syms[35][0] = XK_bracketright;
	xsrv->keyboard.syms[35][1] = XK_braceright;
	xsrv->keyboard.syms[36][0] = XK_Return;
	xsrv->keyboard.syms[37][0] = XK_Control_L;
	xsrv->keyboard.syms[38][0] = XK_a;
	xsrv->keyboard.syms[38][1] = XK_A;
	xsrv->keyboard.syms[39][0] = XK_s;
	xsrv->keyboard.syms[39][1] = XK_S;
	xsrv->keyboard.syms[40][0] = XK_d;
	xsrv->keyboard.syms[40][1] = XK_D;
	xsrv->keyboard.syms[41][0] = XK_f;
	xsrv->keyboard.syms[41][1] = XK_F;
	xsrv->keyboard.syms[42][0] = XK_g;
	xsrv->keyboard.syms[42][1] = XK_G;
	xsrv->keyboard.syms[43][0] = XK_h;
	xsrv->keyboard.syms[43][1] = XK_H;
	xsrv->keyboard.syms[44][0] = XK_j;
	xsrv->keyboard.syms[44][1] = XK_J;
	xsrv->keyboard.syms[45][0] = XK_k;
	xsrv->keyboard.syms[45][1] = XK_K;
	xsrv->keyboard.syms[46][0] = XK_l;
	xsrv->keyboard.syms[46][1] = XK_L;
	xsrv->keyboard.syms[47][0] = XK_semicolon;
	xsrv->keyboard.syms[47][1] = XK_colon;
	xsrv->keyboard.syms[48][0] = XK_apostrophe;
	xsrv->keyboard.syms[48][1] = XK_quotedbl;
	xsrv->keyboard.syms[49][0] = XK_grave;
	xsrv->keyboard.syms[49][1] = XK_asciitilde;
	xsrv->keyboard.syms[50][0] = XK_Shift_L;
	xsrv->keyboard.syms[51][0] = XK_backslash;
	xsrv->keyboard.syms[51][1] = XK_bar;
	xsrv->keyboard.syms[52][0] = XK_z;
	xsrv->keyboard.syms[52][1] = XK_Z;
	xsrv->keyboard.syms[53][0] = XK_x;
	xsrv->keyboard.syms[53][1] = XK_X;
	xsrv->keyboard.syms[54][0] = XK_c;
	xsrv->keyboard.syms[54][1] = XK_C;
	xsrv->keyboard.syms[55][0] = XK_v;
	xsrv->keyboard.syms[55][1] = XK_V;
	xsrv->keyboard.syms[56][0] = XK_b;
	xsrv->keyboard.syms[56][1] = XK_B;
	xsrv->keyboard.syms[57][0] = XK_n;
	xsrv->keyboard.syms[57][1] = XK_N;
	xsrv->keyboard.syms[58][0] = XK_m;
	xsrv->keyboard.syms[58][1] = XK_M;
	xsrv->keyboard.syms[59][0] = XK_comma;
	xsrv->keyboard.syms[59][1] = XK_less;
	xsrv->keyboard.syms[60][0] = XK_period;
	xsrv->keyboard.syms[60][1] = XK_greater;
	xsrv->keyboard.syms[61][0] = XK_slash;
	xsrv->keyboard.syms[61][1] = XK_question;
	xsrv->keyboard.syms[62][0] = XK_Shift_R;
	xsrv->keyboard.syms[63][0] = XK_KP_Multiply;
	xsrv->keyboard.syms[64][0] = XK_Alt_L;
	xsrv->keyboard.syms[65][0] = XK_space;
	xsrv->keyboard.syms[66][0] = XK_Caps_Lock;
	xsrv->keyboard.syms[67][0] = XK_F1;
	xsrv->keyboard.syms[68][0] = XK_F2;
	xsrv->keyboard.syms[69][0] = XK_F3;
	xsrv->keyboard.syms[70][0] = XK_F4;
	xsrv->keyboard.syms[71][0] = XK_F5;
	xsrv->keyboard.syms[72][0] = XK_F6;
	xsrv->keyboard.syms[73][0] = XK_F7;
	xsrv->keyboard.syms[74][0] = XK_F8;
	xsrv->keyboard.syms[75][0] = XK_F9;
	xsrv->keyboard.syms[76][0] = XK_F10;
	xsrv->keyboard.syms[77][0] = XK_Num_Lock;
	xsrv->keyboard.syms[78][0] = XK_Scroll_Lock;
	xsrv->keyboard.syms[79][0] = XK_KP_7;
	xsrv->keyboard.syms[80][0] = XK_KP_8;
	xsrv->keyboard.syms[81][0] = XK_KP_9;
	xsrv->keyboard.syms[82][0] = XK_KP_Subtract;
	xsrv->keyboard.syms[83][0] = XK_KP_4;
	xsrv->keyboard.syms[84][0] = XK_KP_5;
	xsrv->keyboard.syms[85][0] = XK_KP_6;
	xsrv->keyboard.syms[86][0] = XK_KP_Add;
	xsrv->keyboard.syms[87][0] = XK_KP_1;
	xsrv->keyboard.syms[88][0] = XK_KP_2;
	xsrv->keyboard.syms[89][0] = XK_KP_3;
	xsrv->keyboard.syms[90][0] = XK_KP_0;
	xsrv->keyboard.syms[91][0] = XK_KP_Decimal;
	xsrv->keyboard.syms[95][0] = XK_F11;
	xsrv->keyboard.syms[96][0] = XK_F12;
	xsrv->keyboard.syms[105][0] = XK_Control_R;
	xsrv->keyboard.syms[108][0] = XK_Alt_R;
	xsrv->keyboard.syms[111][0] = XK_Up;
	xsrv->keyboard.syms[116][0] = XK_Down;
	xsrv->keyboard.syms[113][0] = XK_Left;
	xsrv->keyboard.syms[114][0] = XK_Right;
#endif
	return 0;
}

static int setup_backend(struct xsrv *xsrv)
{
	if (backend_init(xsrv))
		return 1;
	memset(xsrv->backend.data, 0, xsrv->backend.pitch * xsrv->backend.height);
	return 0;
}

static int add_atom(struct xsrv *xsrv, const char *name, uint32_t id)
{
	struct atom *atom = malloc(sizeof(*atom));
	if (!atom)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	atom->name = strdup(name);
	if (!atom->name)
	{
		free(atom);
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	atom->name_len = strlen(atom->name);
	atom->id = id;
	TAILQ_INSERT_TAIL(&xsrv->atoms, atom, chain);
	return 0;
}

static int setup_atoms(struct xsrv *xsrv)
{
#define ADD_ATOM(atom) add_atom(xsrv, #atom, XA_##atom)
	if (ADD_ATOM(PRIMARY)
	 || ADD_ATOM(SECONDARY)
	 || ADD_ATOM(ARC)
	 || ADD_ATOM(ATOM)
	 || ADD_ATOM(BITMAP)
	 || ADD_ATOM(CARDINAL)
	 || ADD_ATOM(COLORMAP)
	 || ADD_ATOM(CURSOR)
	 || ADD_ATOM(CUT_BUFFER0)
	 || ADD_ATOM(CUT_BUFFER1)
	 || ADD_ATOM(CUT_BUFFER2)
	 || ADD_ATOM(CUT_BUFFER3)
	 || ADD_ATOM(CUT_BUFFER4)
	 || ADD_ATOM(CUT_BUFFER5)
	 || ADD_ATOM(CUT_BUFFER6)
	 || ADD_ATOM(CUT_BUFFER7)
	 || ADD_ATOM(DRAWABLE)
	 || ADD_ATOM(FONT)
	 || ADD_ATOM(INTEGER)
	 || ADD_ATOM(PIXMAP)
	 || ADD_ATOM(POINT)
	 || ADD_ATOM(RECTANGLE)
	 || ADD_ATOM(RESOURCE_MANAGER)
	 || ADD_ATOM(RGB_COLOR_MAP)
	 || ADD_ATOM(RGB_BEST_MAP)
	 || ADD_ATOM(RGB_BLUE_MAP)
	 || ADD_ATOM(RGB_DEFAULT_MAP)
	 || ADD_ATOM(RGB_GRAY_MAP)
	 || ADD_ATOM(RGB_GREEN_MAP)
	 || ADD_ATOM(RGB_RED_MAP)
	 || ADD_ATOM(STRING)
	 || ADD_ATOM(VISUALID)
	 || ADD_ATOM(WINDOW)
	 || ADD_ATOM(WM_COMMAND)
	 || ADD_ATOM(WM_HINTS)
	 || ADD_ATOM(WM_CLIENT_MACHINE)
	 || ADD_ATOM(WM_ICON_NAME)
	 || ADD_ATOM(WM_ICON_SIZE)
	 || ADD_ATOM(WM_NAME)
	 || ADD_ATOM(WM_NORMAL_HINTS)
	 || ADD_ATOM(WM_SIZE_HINTS)
	 || ADD_ATOM(WM_ZOOM_HINTS)
	 || ADD_ATOM(MIN_SPACE)
	 || ADD_ATOM(NORM_SPACE)
	 || ADD_ATOM(MAX_SPACE)
	 || ADD_ATOM(END_SPACE)
	 || ADD_ATOM(SUPERSCRIPT_X)
	 || ADD_ATOM(SUPERSCRIPT_Y)
	 || ADD_ATOM(SUBSCRIPT_X)
	 || ADD_ATOM(SUBSCRIPT_Y)
	 || ADD_ATOM(UNDERLINE_POSITION)
	 || ADD_ATOM(UNDERLINE_THICKNESS)
	 || ADD_ATOM(STRIKEOUT_POSITION)
	 || ADD_ATOM(STRIKEOUT_DESCENT)
	 || ADD_ATOM(ITALIC_ANGLE)
	 || ADD_ATOM(X_HEIGHT)
	 || ADD_ATOM(QUAD_WIDTH)
	 || ADD_ATOM(WEIGHT)
	 || ADD_ATOM(POINT_SIZE)
	 || ADD_ATOM(RESOLUTION)
	 || ADD_ATOM(COPYRIGHT)
	 || ADD_ATOM(NOTICE)
	 || ADD_ATOM(FONT_NAME)
	 || ADD_ATOM(FAMILY_NAME)
	 || ADD_ATOM(FULL_NAME)
	 || ADD_ATOM(CAP_HEIGHT)
	 || ADD_ATOM(WM_CLASS)
	 || ADD_ATOM(TRANSIENT_FOR))
		return 1;
	return 0;
#undef ADD_ATOM
}

uint32_t xsrv_allocate_id(struct xsrv *xsrv)
{
	if (xsrv->xid >= 0x7FFFFF)
	{
		fprintf(stderr, "xid out of bounds\n");
		abort(); /* XXX */
	}
	return ++xsrv->xid;
}

uint32_t xsrv_allocate_id_base(struct xsrv *xsrv)
{
	for (size_t i = 0; i < 0x200; i += sizeof(size_t) * 8)
	{
		size_t *v = &xsrv->id_base_bitmap[i / (sizeof(size_t) * 8)];
		if (*v == (size_t)-1)
			continue;
		for (size_t j = 0; j < sizeof(size_t) * 8; ++j)
		{
			if (!(*v & (1 << j)))
			{
				*v |= (1 << j);
				return (i + j) * 0x100000;
			}
		}
	}
	return 0;
}

void xsrv_release_id_base(struct xsrv *xsrv, uint32_t base)
{
	base /= 0x100000;
	xsrv->id_base_bitmap[base / (sizeof(size_t) * 8)] &= ~(1 << (base % (sizeof(size_t) * 8)));
}

static int modifier_has_key(struct xsrv *xsrv, uint8_t modifier, uint8_t key)
{
	for (size_t i = 0; i < xsrv->keyboard.codes_per_modifier; ++i)
	{
		if (!xsrv->keyboard.modifiers[modifier][i])
			break;
		if (xsrv->keyboard.modifiers[modifier][i] == key)
			return 1;
	}
	return 0;
}

void xsrv_key_press(struct xsrv *xsrv, uint8_t key)
{
	if (xsrv->focus.window)
		window_key_press(xsrv, xsrv->focus.window, key);
	if (modifier_has_key(xsrv, ShiftMapIndex, key))
		xsrv->keybutmask |= ShiftMask;
	if (modifier_has_key(xsrv, LockMapIndex, key))
		xsrv->keybutmask |= LockMask;
	if (modifier_has_key(xsrv, ControlMapIndex, key))
		xsrv->keybutmask |= ControlMask;
	if (modifier_has_key(xsrv, Mod1MapIndex, key))
		xsrv->keybutmask |= Mod1Mask;
	if (modifier_has_key(xsrv, Mod2MapIndex, key))
		xsrv->keybutmask |= Mod2Mask;
	if (modifier_has_key(xsrv, Mod3MapIndex, key))
		xsrv->keybutmask |= Mod3Mask;
	if (modifier_has_key(xsrv, Mod4MapIndex, key))
		xsrv->keybutmask |= Mod4Mask;
	if (modifier_has_key(xsrv, Mod5MapIndex, key))
		xsrv->keybutmask |= Mod5Mask;
	xsrv->keyboard.state[key / 8] |= 1 << (key % 8);
}

void xsrv_key_release(struct xsrv *xsrv, uint8_t key)
{
	if (xsrv->focus.window)
		window_key_release(xsrv, xsrv->focus.window, key);
	if (modifier_has_key(xsrv, ShiftMapIndex, key))
		xsrv->keybutmask &= ~ShiftMask;
	if (modifier_has_key(xsrv, LockMapIndex, key))
		xsrv->keybutmask &= ~LockMask;
	if (modifier_has_key(xsrv, ControlMapIndex, key))
		xsrv->keybutmask &= ~ControlMask;
	if (modifier_has_key(xsrv, Mod1MapIndex, key))
		xsrv->keybutmask &= ~Mod1Mask;
	if (modifier_has_key(xsrv, Mod2MapIndex, key))
		xsrv->keybutmask &= ~Mod2Mask;
	if (modifier_has_key(xsrv, Mod3MapIndex, key))
		xsrv->keybutmask &= ~Mod3Mask;
	if (modifier_has_key(xsrv, Mod4MapIndex, key))
		xsrv->keybutmask &= ~Mod4Mask;
	if (modifier_has_key(xsrv, Mod5MapIndex, key))
		xsrv->keybutmask &= ~Mod5Mask;
	xsrv->keyboard.state[key / 8] &= ~(1 << (key % 8));
}

static struct window *get_ancestor(struct window *a, struct window *b)
{
	if (!a || !b)
		return NULL;
	while (a->tree_depth > b->tree_depth)
		a = a->parent;
	while (b->tree_depth > a->tree_depth)
		b = b->parent;
	while (a != b)
	{
		a = a->parent;
		b = b->parent;
	}
	return a;
}

static void enter_leave_events(struct xsrv *xsrv, struct window *a,
                               struct window *b)
{
	struct window *ancestor = get_ancestor(a, b);
	if (ancestor == b)
	{
		window_leave_notify(xsrv, a, NotifyAncestor);
		struct window *tmp = a->parent;
		while (tmp && tmp != b)
		{
			window_leave_notify(xsrv, tmp, NotifyVirtual);
			tmp = tmp->parent;
		}
		window_enter_notify(xsrv, b, NotifyInferior);
		window_keymap_notify(xsrv, b);
		return;
	}
	if (ancestor == a)
	{
		window_leave_notify(xsrv, a, NotifyInferior);
		struct window *tmp = b->parent;
		while (tmp && tmp != a) /* XXX reverse order */
		{
			window_leave_notify(xsrv, tmp, NotifyVirtual);
			tmp = tmp->parent;
		}
		window_enter_notify(xsrv, b, NotifyAncestor);
		window_keymap_notify(xsrv, b);
		return;
	}
	window_leave_notify(xsrv, a, NotifyNonlinear);
	struct window *tmp = a->parent;
	while (tmp != ancestor)
	{
		window_leave_notify(xsrv, tmp, NotifyNonlinearVirtual);
		tmp = tmp->parent;
	}
	tmp = b->parent;
	while (tmp != ancestor) /* XXX should be in reverse order */
	{
		window_enter_notify(xsrv, tmp, NotifyNonlinearVirtual);
		window_keymap_notify(xsrv, tmp);
		tmp = tmp->parent;
	}
	window_enter_notify(xsrv, b, NotifyNonlinear);
	window_keymap_notify(xsrv, b);
}

static void get_cursor_rect(struct xsrv *xsrv, uint16_t *x, uint16_t *y,
                            uint16_t *width, uint16_t *height)
{
	struct cursor *cursor = xsrv->pointer.cursor;
	*x = xsrv->pointer.x - cursor->xhot;
	*y = xsrv->pointer.y - cursor->yhot;
	if (cursor->mask)
	{
		*width = cursor->mask->drawable.width;
		*height = cursor->mask->drawable.height;
		*x -= cursor->maskx;
		*y -= cursor->masky;
	}
	else
	{
		*width = cursor->color->drawable.width;
		*height = cursor->color->drawable.height;
	}
}

void xsrv_cursor_motion(struct xsrv *xsrv, uint16_t x, uint16_t y)
{
	/* XXX acceleration */
	if (x >= xsrv->backend.width)
		x = xsrv->backend.width - 1;
	if (y >= xsrv->backend.height)
		y = xsrv->backend.height - 1;
	uint16_t old_x;
	uint16_t old_y;
	uint16_t old_width;
	uint16_t old_height;
	get_cursor_rect(xsrv, &old_x, &old_y, &old_width, &old_height);
	xsrv->pointer.x = x;
	xsrv->pointer.y = y;
	xsrv->pointer.wx = x;
	xsrv->pointer.wy = y;
	struct window *cursor_window = get_window_at(xsrv,
	                                             &xsrv->pointer.wx,
	                                             &xsrv->pointer.wy);
	if (xsrv->pointer_grab.client
	 && (xsrv->pointer_grab.event_mask & PointerMotionMask))
	{
		window_motion_notify(xsrv, xsrv->pointer_grab.window,
		                     NotifyNormal,
		                     xsrv->pointer.wx,
		                     xsrv->pointer.wy); /* XXX wx wy */
	}
	else if (cursor_window != xsrv->pointer.window)
	{
		enter_leave_events(xsrv, xsrv->pointer.window, cursor_window);
		xsrv->pointer.window = cursor_window;
	}
	else
	{
		window_motion_notify(xsrv, cursor_window, NotifyNormal,
		                     xsrv->pointer.wx, xsrv->pointer.wy);
	}
	xsrv->pointer.cursor = window_get_cursor(xsrv, xsrv->pointer.window);
	uint16_t new_x;
	uint16_t new_y;
	uint16_t new_width;
	uint16_t new_height;
	get_cursor_rect(xsrv, &new_x, &new_y, &new_width, &new_height);
	framebuffer_redraw(xsrv, old_x, old_y, old_width, old_height);
	framebuffer_redraw(xsrv, new_x, new_y, new_width, new_height);
}

void xsrv_button_press(struct xsrv *xsrv, uint8_t button)
{
	if (!button || button > 32)
		return;
	button--;
	if (xsrv->pointer.state[button])
	{
		fprintf(stderr, "double button press %u\n", button);
		return;
	}
	xsrv->pointer.state[button] = 1;
	button = xsrv->pointer.map[button];
	if (!button)
		return;
	if (xsrv->pointer_grab.client
	 && (xsrv->pointer_grab.event_mask & ButtonPressMask))
	{
		window_button_press(xsrv, xsrv->pointer_grab.window, button,
		                    xsrv->pointer.wx, xsrv->pointer.wy); /* XXX wx wy */
	}
	else if (xsrv->pointer.window)
	{
		window_button_press(xsrv, xsrv->pointer.window, button,
		                    xsrv->pointer.wx, xsrv->pointer.wy);
	}
	xsrv->keybutmask |= Button1Mask << (button - 1);
}

void xsrv_button_release(struct xsrv *xsrv, uint8_t button)
{
	if (!button || button > 32)
		return;
	button--;
	if (!xsrv->pointer.state[button])
	{
		fprintf(stderr, "double button release %u\n", button);
		return;
	}
	xsrv->pointer.state[button] = 0;
	button = xsrv->pointer.map[button];
	if (!button)
		return;
	if (xsrv->pointer_grab.client
	 && (xsrv->pointer_grab.event_mask & ButtonPressMask))
	{
		window_button_release(xsrv, xsrv->pointer_grab.window, button,
		                      xsrv->pointer.wx, xsrv->pointer.wy); /* XXX wx wy */
	}
	else if (xsrv->pointer.window)
	{
		window_button_release(xsrv, xsrv->pointer.window, button,
		                      xsrv->pointer.wx, xsrv->pointer.wy);
	}
	xsrv->keybutmask &= ~(Button1Mask << (button - 1));
}

static void focus_event(struct xsrv *xsrv, struct window *a, struct window *b,
                        struct window *p)
{
	(void)p;
	struct window *ancestor = get_ancestor(a, b);
	if (ancestor)
	{
		if (ancestor == b)
		{
			window_focus_out(xsrv, a, NotifyAncestor);
			struct window *tmp = a->parent;
			while (tmp && tmp != b)
			{
				window_focus_out(xsrv, tmp, NotifyVirtual);
				tmp = tmp->parent;
			}
			window_focus_in(xsrv, b, NotifyInferior);
			window_keymap_notify(xsrv, b);
			/* XXX p */
			return;
		}
		if (ancestor == a)
		{
			/* XXX p */
			window_focus_out(xsrv, a, NotifyInferior);
			struct window *tmp = b->parent;
			while (tmp && tmp != a) /* XXX reverse order */
			{
				window_focus_in(xsrv, tmp, NotifyVirtual);
				window_keymap_notify(xsrv, tmp);
				tmp = tmp->parent;
			}
			window_focus_in(xsrv, b, NotifyAncestor);
			window_keymap_notify(xsrv, b);
		}
	}
	/* XXX p */
	if (a)
	{
		window_focus_out(xsrv, a, NotifyNonlinear);
		struct window *end = ancestor ? ancestor : a->drawable.root;
		struct window *tmp = a->parent;
		while (tmp && tmp != end)
		{
			window_focus_out(xsrv, tmp, NotifyNonlinearVirtual);
			tmp = tmp->parent;
		}
	}
	if (b)
	{
		struct window *end = ancestor ? ancestor : b->drawable.root;
		struct window *tmp = b->parent;
		while (tmp && tmp != end) /* XXX should be in reverse order */
		{
			window_focus_in(xsrv, tmp, NotifyNonlinearVirtual);
			window_keymap_notify(xsrv, tmp);
			tmp = tmp->parent;
		}
		window_focus_in(xsrv, b, NotifyNonlinear);
		window_keymap_notify(xsrv, b);
	}
	/* XXX p */
}

void xsrv_set_focus(struct xsrv *xsrv, struct window *window,
                    uint8_t revert_to, uint32_t timestamp)
{
	xsrv->focus.revert_to = revert_to;
	xsrv->focus.timestamp = timestamp;
	if (window)
		window->object.refs++;
	focus_event(xsrv, xsrv->focus.window, window,
	            xsrv->pointer.window);
	object_free(xsrv, OBJECT(xsrv->focus.window));
	xsrv->focus.window = window;
}

void xsrv_revert_focus(struct xsrv *xsrv)
{
	switch (xsrv->focus.revert_to)
	{
		case RevertToParent:
		{
			struct window *tmp = xsrv->focus.window->parent;
			while (!window_visible(tmp)) /* XXX better algorithm */
				tmp = tmp->parent;
			focus_event(xsrv, xsrv->focus.window, tmp,
			            xsrv->pointer.window);
			xsrv->focus.revert_to = None;
			xsrv->focus.window = tmp;
			tmp->object.refs++;
			break;
		}
		case RevertToNone:
		{
			focus_event(xsrv, xsrv->focus.window, NULL,
			            xsrv->pointer.window);
			xsrv->focus.window = NULL;
			break;
		}
		case RevertToPointerRoot:
		{
			focus_event(xsrv, xsrv->focus.window,
			            xsrv->screens[0]->root,
			            xsrv->pointer.window);
			xsrv->focus.window = xsrv->screens[0]->root;
			xsrv->focus.window->object.refs++;
			break;
		}
	}
}

void xsrv_grab_pointer(struct xsrv *xsrv, struct client *client,
                       struct window *window, struct window *confine_to,
                       struct cursor *cursor, uint16_t event_mask,
                       uint8_t pointer_mode, uint8_t keyboard_mode,
                       uint8_t owner_events, uint32_t time)
{
	if (xsrv->pointer_grab.client)
	{
		object_free(xsrv, OBJECT(xsrv->pointer_grab.window));
		object_free(xsrv, OBJECT(xsrv->pointer_grab.confine_to));
		object_free(xsrv, OBJECT(xsrv->pointer_grab.cursor));
	}
	xsrv->pointer_grab.client = client;
	xsrv->pointer_grab.window = window;
	xsrv->pointer_grab.confine_to = confine_to;
	xsrv->pointer_grab.cursor = cursor;
	xsrv->pointer_grab.event_mask = event_mask;
	xsrv->pointer_grab.pointer_mode = pointer_mode;
	xsrv->pointer_grab.keyboard_mode = keyboard_mode;
	xsrv->pointer_grab.owner_events = owner_events;
	xsrv->pointer_grab.time = time;
	if (window)
		window->object.refs++;
	if (confine_to)
		confine_to->object.refs++;
	if (cursor)
		cursor->object.refs++;
}

void xsrv_ungrab_pointer(struct xsrv *xsrv)
{
	if (!xsrv->pointer_grab.window)
		return;
	object_free(xsrv, OBJECT(xsrv->pointer_grab.window));
	object_free(xsrv, OBJECT(xsrv->pointer_grab.confine_to));
	object_free(xsrv, OBJECT(xsrv->pointer_grab.cursor));
	xsrv->pointer_grab.client = NULL;
	xsrv->pointer_grab.window = NULL;
	xsrv->pointer_grab.confine_to = NULL;
	xsrv->pointer_grab.cursor = NULL;
	xsrv_cursor_motion(xsrv, xsrv->pointer.x, xsrv->pointer.y);
}

void delete_button_grab(struct xsrv *xsrv, struct button_grab *button_grab)
{
	object_free(xsrv, OBJECT(button_grab->pointer_grab.window));
	object_free(xsrv, OBJECT(button_grab->pointer_grab.confine_to));
	object_free(xsrv, OBJECT(button_grab->pointer_grab.cursor));
	TAILQ_REMOVE(&button_grab->pointer_grab.client->button_grabs,
	             button_grab, client_chain);
	TAILQ_REMOVE(&button_grab->pointer_grab.window->button_grabs,
	             button_grab, window_chain);
	free(button_grab);
}

uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_nsec + ts.tv_sec * 1000000000;
}

uint64_t millitime(void)
{
	return nanotime() / 1000000;
}

uint32_t npot32(uint32_t val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	return ++val;
}

static int poll_sockets(struct xsrv *xsrv)
{
	fd_set rfds;
	fd_set wfds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_SET(xsrv->sock, &rfds);
	FD_SET(xsrv->backend.fd, &rfds);
	int nfds = xsrv->sock + 1;
	if (xsrv->backend.fd >= nfds)
		nfds = xsrv->backend.fd + 1;
	struct client *client;
	TAILQ_FOREACH(client, &xsrv->clients, chain)
	{
		if (xsrv->grab_client && xsrv->grab_client != client)
			continue;
		switch (client->sock_state & (CLIENT_SOCK_RECV | CLIENT_SOCK_SEND))
		{
			case 0:
				FD_SET(client->fd, &wfds);
				FD_SET(client->fd, &rfds);
				break;
			case CLIENT_SOCK_RECV:
				FD_SET(client->fd, &wfds);
				break;
			case CLIENT_SOCK_SEND:
				FD_SET(client->fd, &rfds);
				break;
			case CLIENT_SOCK_RECV | CLIENT_SOCK_SEND:
				continue;
		}
		if (client->fd >= nfds)
			nfds = client->fd + 1;
	}
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	int ret = select(nfds, &rfds, &wfds, NULL,
	                 !xsrv->need_process ? NULL : &tv);
	xsrv->need_process = 0;
	if (ret == -1)
	{
		if (errno == EINTR)
			return 0;
		fprintf(stderr, "%s: select: %s\n", xsrv->progname,
		        strerror(errno));
		return 1;
	}
	if (FD_ISSET(xsrv->sock, &rfds))
	{
		if (handle_new_client(xsrv))
			return 1;
	}
	backend_poll(xsrv);
	TAILQ_FOREACH(client, &xsrv->clients, chain)
	{
		if (FD_ISSET(client->fd, &rfds))
			client->sock_state |= CLIENT_SOCK_RECV;
		if (FD_ISSET(client->fd, &wfds))
			client->sock_state |= CLIENT_SOCK_SEND;
	}
	return 0;
}

struct atom *atom_get(struct xsrv *xsrv, const char *name)
{
	uint32_t name_len = strlen(name);
	struct atom *atom;
	TAILQ_FOREACH(atom, &xsrv->atoms, chain)
	{
		if (atom->name_len == name_len
		 && !strcmp(atom->name, name))
			return atom;
	}
	return NULL;
}

struct atom *atom_get_id(struct xsrv *xsrv, uint32_t id)
{
	struct atom *atom;
	TAILQ_FOREACH(atom, &xsrv->atoms, chain)
	{
		if (atom->id == id)
			return atom;
	}
	return NULL;
}

struct atom *atom_new(struct xsrv *xsrv, const char *name)
{
	struct atom *atom = malloc(sizeof(*atom));
	if (!atom)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	atom->name = strdup(name);
	if (!atom->name)
	{
		free(atom);
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	atom->name_len = strlen(atom->name);
	atom->id = TAILQ_LAST(&xsrv->atoms, atom_head)->id + 1;
	TAILQ_INSERT_TAIL(&xsrv->atoms, atom, chain);
	return atom;
}

void test_fb_redraw(struct xsrv *xsrv, struct drawable *drawable,
                    int32_t min_x, int32_t min_y,
                    int32_t max_x, int32_t max_y)
{
	if (drawable->object.type != xsrv->obj_window)
		return;
	struct window *window = (struct window*)drawable;
	if (!window_visible(window))
		return;
	if (min_x > max_x || min_y > max_y
	 || max_x < 0 || max_y < 0)
		return;
	min_x--;
	max_x++;
	min_y--;
	max_y++;
	if (min_x < 0)
		min_x = 0;
	if (max_y < 0)
		max_y = 0;
	uint32_t rect_x = min_x;
	uint32_t rect_y = min_y;
	uint32_t rect_width = max_x - min_x;
	uint32_t rect_height = max_y - min_y;
	window_get_fb_rect(xsrv, window, &rect_x, &rect_y,
	                   &rect_width, &rect_height);
#if 0
	printf("redraw %" PRIu32 "x%" PRIu32 " - %" PRIu32 "x%" PRIu32 "\n",
	       rect_x, rect_y, rect_x + rect_width, rect_y + rect_height);
#endif
	framebuffer_redraw(xsrv, rect_x, rect_y,
	                   rect_width, rect_height);
}

static void usage(const char *progname)
{
	printf("%s [-d] [-h]\n", progname);
	printf("-d: daemonize\n");
	printf("-h: display this help\n");
}

int main(int argc, char **argv)
{
	int daemonize = 0;
	int c;

	while ((c = getopt(argc, argv, "dh")) != -1)
	{
		switch (c)
		{
			case 'd':
				daemonize = 1;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	signal(SIGPIPE, SIG_IGN);
	srand(nanotime());
	struct xsrv xsrv;
	memset(&xsrv, 0, sizeof(xsrv));
	xsrv.id_base_bitmap[0] = 0xFF; /* reserve 8 first bitmask */
	TAILQ_INIT(&xsrv.clients);
	TAILQ_INIT(&xsrv.atoms);
	xsrv.progname = argv[0];
	if (setup_sock(&xsrv)
	 || setup_backend(&xsrv)
	 || setup_object_types(&xsrv)
	 || setup_objects_map(&xsrv)
	 || setup_formats(&xsrv)
	 || setup_screens(&xsrv)
	 || setup_cursor(&xsrv)
	 || setup_fonts(&xsrv)
	 || setup_focus(&xsrv)
	 || setup_pointer(&xsrv)
	 || setup_keyboard(&xsrv)
	 || setup_atoms(&xsrv)
	 || setup_shm(&xsrv)
	 || setup_render(&xsrv))
		return EXIT_FAILURE;
	if (daemonize && daemon(1, 1))
	{
		fprintf(stderr, "%s: daemon: %s\n",
		        argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	/* XXX drop privileges */
	framebuffer_redraw(&xsrv, 0, 0, xsrv.backend.width,
	                   xsrv.backend.height);
	struct rect full_rect;
	full_rect.left = 0;
	full_rect.right = xsrv.backend.width - 1;
	full_rect.top = 0;
	full_rect.bottom = xsrv.backend.height - 1;
	framebuffer_update(&xsrv, &full_rect);
	backend_display(&xsrv, &full_rect);
	while (1)
	{
#ifdef DEBUG_TIMING
		uint64_t s, e;
		s = nanotime();
#endif
		if (poll_sockets(&xsrv))
			return EXIT_FAILURE;
#ifdef DEBUG_TIMING
		e = nanotime();
		printf("poll: %" PRIu64 " us\n", (e - s) / 1000);
		s = nanotime();
#endif
		TAILQ_HEAD(, client) clients_gc;
		TAILQ_INIT(&clients_gc);
		struct client *client;
		TAILQ_FOREACH(client, &xsrv.clients, chain)
		{
			if (xsrv.grab_client && xsrv.grab_client != client)
				continue;
			if (!client->sock_state
			 && !ringbuf_read_size(&client->wbuf))
				continue;
			if (client_run(&xsrv, client))
			{
				TAILQ_INSERT_TAIL(&clients_gc, client, gc_chain);
				continue;
			}
			if ((client->sock_state & CLIENT_SOCK_RECV)
			 || (client->sock_state & CLIENT_SOCK_SEND
			  && ringbuf_read_size(&client->wbuf)))
				xsrv.need_process = 1;
		}
		client = TAILQ_FIRST(&clients_gc);
		while (client)
		{
			TAILQ_REMOVE(&clients_gc, client, gc_chain);
			client_delete(&xsrv, client);
			client = TAILQ_FIRST(&clients_gc);
		}
#ifdef DEBUG_TIMING
		e = nanotime();
		printf("client process: %" PRIu64 " us\n", (e - s) / 1000);
#endif
		if (xsrv.redraw_rect.left <= xsrv.redraw_rect.right
		 && xsrv.redraw_rect.top <= xsrv.redraw_rect.bottom)
		{
#ifdef DEBUG_TIMING
			s = nanotime();
#endif
			framebuffer_update(&xsrv, &xsrv.redraw_rect);
#ifdef DEBUG_TIMING
			e = nanotime();
			printf("framebuffer: %" PRIu64 " us\n", (e - s) / 1000);
			s = nanotime();
#endif
			backend_display(&xsrv, &xsrv.redraw_rect);
#ifdef DEBUG_TIMING
			e = nanotime();
			printf("display: %" PRIu64 " us\n", (e - s) / 1000);
#endif
			xsrv.redraw_rect.left = -1;
			xsrv.redraw_rect.top = -1;
			xsrv.redraw_rect.right = 0;
			xsrv.redraw_rect.bottom = 0;
		}
	}
	return EXIT_SUCCESS;
}
