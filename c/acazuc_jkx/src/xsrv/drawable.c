#include "xsrv.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int
drawable_init(struct xsrv *xsrv,
              struct drawable *drawable,
              uint16_t width,
              uint16_t height,
              const struct format *format,
              struct window *root)
{
	uint32_t alloc_width = npot32(width);
	uint32_t alloc_height = npot32(height);
	uint32_t tmp = alloc_width * format->bpp;
	tmp = tmp + format->scanline_pad - 1;
	tmp -= tmp % format->scanline_pad;
	drawable->pitch = tmp / 8;
	drawable->data = malloc(alloc_height * drawable->pitch);
	if (!drawable->data)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return 1;
	}
	memset(drawable->data, 0, alloc_height * drawable->pitch);
	drawable->width = width;
	drawable->height = height;
	drawable->format = format;
	drawable->alloc_width = alloc_width;
	drawable->alloc_height = alloc_height;
	drawable->root = root;
	return 0;
}

struct drawable *
drawable_get(struct xsrv *xsrv, uint32_t id)
{
	struct object *object;

	object = object_get(xsrv, id);
	if (!object)
		return NULL;
	if (object->type != xsrv->obj_window
	 && object->type != xsrv->obj_pixmap)
	{
		object->refs--;
		return NULL;
	}
	return (struct drawable*)object;
}

static void *
get_y_ptr(struct drawable *drawable, uint16_t y)
{
	return &((uint8_t*)drawable->data)[y * drawable->pitch];
}

static void *
get_x_ptr(struct drawable *drawable, void *lptr, uint16_t x)
{
	return &((uint8_t*)lptr)[x * drawable->format->bpp / 8];
}

static void *
get_xy_ptr(struct drawable *drawable, uint16_t x, uint16_t y)
{
	return get_x_ptr(drawable, get_y_ptr(drawable, y), x);
}

uint32_t
drawable_get_pixel(struct xsrv *xsrv,
                   struct drawable *drawable,
                   uint16_t x,
                   uint16_t y)
{
	(void)xsrv;
	switch (drawable->format->depth)
	{
		case 1:
		{
			uint8_t *ptr = get_xy_ptr(drawable, x, y);
			return (*ptr >> (x & 7)) & 1;
		}
		case 4:
		{
			uint8_t *ptr = get_xy_ptr(drawable, x, y);
			return (*ptr >> (4 * (x & 1))) & 0xF;
		}
		case 8:
		{
			uint8_t *ptr = get_xy_ptr(drawable, x, y);
			return *ptr;
		}
		case 15:
		{
			uint16_t *ptr = get_xy_ptr(drawable, x, y);
			return *ptr & 0x7FFF;
		}
		case 16:
		{
			uint16_t *ptr = get_xy_ptr(drawable, x, y);
			return *ptr;
		}
		case 24:
		{
			uint32_t *ptr = get_xy_ptr(drawable, x, y);
			return *ptr & 0xFFFFFF;
		}
		case 32:
		{
			uint32_t *ptr = get_xy_ptr(drawable, x, y);
			return *ptr;
		}
	}
	return 0;
}

void
drawable_set_pixel(struct xsrv *xsrv,
                   struct drawable *drawable,
                   uint16_t x,
                   uint16_t y,
                   uint32_t value)
{
	(void)xsrv;
	switch (drawable->format->depth)
	{
		case 1:
		{
			uint8_t *ptr = get_xy_ptr(drawable, x, y);
			uint8_t shift = (x & 7);
			*ptr = (*ptr & ~(1 << shift)) | ((value & 1) << shift);
			break;
		}
		case 4:
		{
			uint8_t *ptr = get_xy_ptr(drawable, x, y);
			uint8_t shift = 4 * (x & 1);
			*ptr = (*ptr & ~(0xF << shift)) | ((value & 0xF) << shift);
			break;
		}
		case 8:
		{
			uint8_t *ptr = get_xy_ptr(drawable, x, y);
			*ptr = value;
			break;
		}
		case 15:
		{
			uint16_t *ptr = get_xy_ptr(drawable, x, y);
			*ptr = value & 0x7FFF;
			break;
		}
		case 16:
		{
			uint16_t *ptr = get_xy_ptr(drawable, x, y);
			*ptr = value;
			break;
		}
		case 24:
		{
			uint32_t *ptr = get_xy_ptr(drawable, x, y);
			*ptr = value & 0xFFFFFF;
			break;
		}
		case 32:
		{
			uint32_t *ptr = get_xy_ptr(drawable, x, y);
			*ptr = value;
			break;
		}
	}
}
