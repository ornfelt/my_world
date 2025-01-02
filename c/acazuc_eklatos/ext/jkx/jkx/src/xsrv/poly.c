#include "xsrv.h"

#include <stdlib.h>

/*
 * http://members.chello.at/~easyfilter/bresenham.html
 */

static void draw_point(struct xsrv *xsrv, struct drawable *drawable,
                      struct gcontext *gcontext, int16_t x, int16_t y)
{
	if (x < 0 || (uint16_t)x >= drawable->width
	 || y < 0 || (uint16_t)y >= drawable->height)
		return;
	drawable_set_pixel(xsrv, drawable, x, y, gcontext->values.foreground);
}

void poly_point(struct xsrv *xsrv, struct drawable *drawable,
                struct gcontext *gcontext, int16_t x, int16_t y)
{
	draw_point(xsrv, drawable, gcontext, x, y);
}

static void draw_line(struct xsrv *xsrv, struct drawable *drawable,
                      struct gcontext *gcontext, int16_t x1, int16_t y1,
                      int16_t x2, int16_t y2)
{
	int32_t dx =  abs(x2 - x1);
	int32_t dy = -abs(y2 - y1);
	int32_t sx = x1 < x2 ? 1 : -1;
	int32_t sy = y1 < y2 ? 1 : -1;
	int32_t err = dx + dy;
	int32_t x = x1;
	int32_t y = y1;
	while (1)
	{
		draw_point(xsrv, drawable, gcontext, x, y);
		if (x == x2 && y == y2)
			break;
		int32_t err2 = err * 2;
		if (err2 >= dy)
		{
			err += dy;
			x += sx;
		}
		if (err2 <= dx)
		{
			err += dx;
			y += sy;
		}
	}
}

void poly_line(struct xsrv *xsrv, struct drawable *drawable,
               struct gcontext *gcontext, int16_t x1, int16_t y1,
               int16_t x2, int16_t y2)
{
	draw_line(xsrv, drawable, gcontext, x1, y1, x2, y2);
}

void poly_rect(struct xsrv *xsrv, struct drawable *drawable,
               struct gcontext *gcontext, int16_t x, int16_t y,
               uint16_t width, uint16_t height)
{
	draw_line(xsrv, drawable, gcontext, x        , y         , x + width, y);
	draw_line(xsrv, drawable, gcontext, x + width, y         , x + width, y + height);
	draw_line(xsrv, drawable, gcontext, x + width, y + height, x        , y + height);
	draw_line(xsrv, drawable, gcontext, x        , y + height, x        , y);
}

void poly_arc(struct xsrv *xsrv, struct drawable *drawable,
              struct gcontext *gcontext, int16_t x, int16_t y,
              uint16_t width, uint16_t height, int16_t angle1, int16_t angle2)
{
	(void)angle1; /* XXX */
	(void)angle2; /* XXX */
	int32_t x1 = x;
	int32_t y1 = y;
	int32_t x2 = x + width;
	int32_t y2 = y + height;
	int32_t b1 = height & 1;
	int32_t dx = 4 * (1 - width) * height * height;
	int32_t dy = 4 * (b1 + 1) * width * width;
	int32_t err = dx + dy + b1 * width * width;
	y1 += (height + 1) / 2;
	y2 = y1 - b1;
	int32_t a = 8 * width * width;
	int32_t b = 8 * height * height;
	do
	{
		draw_point(xsrv, drawable, gcontext, x1, y1);
		draw_point(xsrv, drawable, gcontext, x2, y1);
		draw_point(xsrv, drawable, gcontext, x2, y2);
		draw_point(xsrv, drawable, gcontext, x1, y2);
		int32_t err2 = err * 2;
		if (err2 <= dy)
		{
			y1++;
			y2--;
			dy += a;
			err += dy;
		}
		if (err2 >= dx || err2 > dy)
		{
			x1++;
			x2--;
			dx += b;
			err += dx;
		}
	} while (x1 <= x2);

	while (y1 - y2 < height)
	{
		draw_point(xsrv, drawable, gcontext, x1 - 1, y1);
		draw_point(xsrv, drawable, gcontext, x2 + 1, y1);
		draw_point(xsrv, drawable, gcontext, x2 + 1, y2);
		draw_point(xsrv, drawable, gcontext, x1 - 1, y2);
		y1++;
		y2--;
	}
}

void poly_fill_rect(struct xsrv *xsrv, struct drawable *drawable,
                    struct gcontext *gcontext, int16_t x, int16_t y,
                    uint16_t width, uint16_t height)
{
	for (uint16_t xx = 0; xx < width; ++xx)
	{
		for (uint16_t yy = 0; yy < height; ++yy)
		{
			draw_point(xsrv, drawable, gcontext, x + xx, y + yy);
		}
	}
}

static void draw_glyph(struct xsrv *xsrv, struct drawable *drawable,
                       struct gcontext *gcontext, struct font_glyph *glyph,
                       int16_t x, int16_t y, uint32_t *minx, uint32_t *miny,
                       uint32_t *maxx, uint32_t *maxy)
{
	int32_t dst_x = x + glyph->left_bearing;
	int32_t dst_y = y - glyph->ascent;
	if (dst_x >= drawable->width || dst_y >= drawable->height)
		return;
	uint16_t src_x;
	uint16_t src_y;
	uint16_t width = glyph->right_bearing - glyph->left_bearing;
	uint16_t height = glyph->ascent + glyph->descent;
	if (dst_x < 0)
	{
		if (-dst_x >= width)
			return;
		width += dst_x;
		src_x = -dst_x;
		dst_x = 0;
	}
	else
	{
		src_x = 0;
		if (dst_x + width > drawable->width)
			width = drawable->width - dst_x;
	}
	if (dst_y < 0)
	{
		if (-dst_y >= height)
			return;
		height += dst_y;
		src_y = -dst_y;
		dst_y = 0;
	}
	else
	{
		src_y = 0;
		if (dst_y + height > drawable->height)
			height = drawable->height - dst_x;
	}
	if ((uint32_t)dst_x < *minx)
		*minx = dst_x;
	if ((uint32_t)dst_y < *miny)
		*miny = dst_y;
	if ((uint32_t)dst_x + width > *maxx)
		*maxx = dst_x + width;
	if ((uint32_t)dst_y + height > *maxy)
		*maxy = dst_y + height;
	for (size_t yy = 0; yy < height; ++yy)
	{
		if (src_y + yy >= glyph->pixmap->drawable.height
		 || src_y + yy >= drawable->height)
			break;
		for (size_t xx = 0; xx < width; ++xx)
		{
			if (src_x + xx >= glyph->pixmap->drawable.width
			 || src_x + xx >= drawable->width)
				break;
			uint8_t font_pixel = drawable_get_pixel(xsrv,
			                                        &glyph->pixmap->drawable,
			                                        src_x + xx,
			                                        src_y + yy);
			uint32_t value = font_pixel
			               ? gcontext->values.foreground
			               : gcontext->values.background;
			drawable_set_pixel(xsrv, drawable,
			                   dst_x + xx, dst_y + yy, value);
		}
	}
}

void poly_text8(struct xsrv *xsrv, struct drawable *drawable,
                struct gcontext *gcontext, int16_t x, int16_t y,
                uint8_t text_len, const uint8_t *text,
                uint32_t *minx, uint32_t *miny,
                uint32_t *maxx, uint32_t *maxy)
{
	if (!gcontext->values.font)
		return;
	struct font_def *font_def = gcontext->values.font->def;
	*minx = UINT32_MAX;
	*miny = UINT32_MAX;
	*maxx = 0;
	*maxy = 0;
	for (size_t i = 0; i < text_len; ++i)
	{
		struct font_glyph *glyph = font_def->glyphs[text[i]];
		if (!glyph)
			continue;
		draw_glyph(xsrv, drawable, gcontext, glyph, x, y,
		           minx, miny, maxx, maxy);
		x += glyph->width;
	}
}
