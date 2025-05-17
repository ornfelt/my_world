#include "xsrv.h"

#include <string.h>

void
framebuffer_redraw(struct xsrv *xsrv,
                   int16_t x,
                   int16_t y,
                   uint16_t width,
                   uint16_t height)
{
	uint32_t left;
	uint32_t right;
	uint32_t top;
	uint32_t bottom;

	if (!width || !height)
		return;
	if (x < 0)
	{
		if (width < -x)
			return;
		width += x;
		x = 0;
	}
	if (y < 0)
	{
		if (height < -y)
			return;
		height += y;
		y = 0;
	}
	left = x;
	right = x + width;
	top = y;
	bottom = y + height;
	if (left >= xsrv->backend.width)
		return;
	if (right >= xsrv->backend.width)
		right = xsrv->backend.width - 1;
	if (top >= xsrv->backend.height)
		return;
	if (bottom >= xsrv->backend.height)
		bottom = xsrv->backend.height - 1;
	if (left > right)
		return;
	if (top > bottom)
		return;
	if (left < xsrv->redraw_rect.left)
		xsrv->redraw_rect.left = left;
	if (top < xsrv->redraw_rect.top)
		xsrv->redraw_rect.top = top;
	if (right > xsrv->redraw_rect.right)
		xsrv->redraw_rect.right = right;
	if (bottom > xsrv->redraw_rect.bottom)
		xsrv->redraw_rect.bottom = bottom;
}

static int
rect_intersect(struct rect *inter, struct rect *a, struct rect *b)
{
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

	uint32_t max_left = MAX(a->left, b->left);
	uint32_t min_right = MIN(a->right, b->right);
	if (max_left > min_right)
		return 0;
	uint32_t max_top = MAX(a->top, b->top);
	uint32_t min_bottom = MIN(a->bottom, b->bottom);
	if (max_top > min_bottom)
		return 0;
	inter->left = max_left;
	inter->right = min_right;
	inter->top = max_top;
	inter->bottom = min_bottom;
	return 1;

#undef MIN
#undef MAX
}

static int
draw_left_border(struct xsrv *xsrv,
                 struct rect *rect,
                 struct window *window,
                 int32_t adj_x,
                 int32_t adj_y)
{
	if (rect->left >= window->border_width)
		return 0;
	uint8_t *base = (uint8_t*)xsrv->backend.data;
	base += (adj_x + rect->left) * 4;
	base += (adj_y + rect->top) * xsrv->backend.pitch;
	do
	{
		uint8_t *dst = base;
		for (size_t y = rect->top; y <= rect->bottom; ++y)
		{
			*(uint32_t*)dst = window->attributes.border_pixel;
			dst += xsrv->backend.pitch;
		}
		++rect->left;
		if (rect->left > rect->right)
			return 1;
		base += 4;
	} while (rect->left < window->border_width);
	return 0;
}

static int
draw_right_border(struct xsrv *xsrv,
                  struct rect *rect,
                  struct window *window,
                  int32_t adj_x,
                  int32_t adj_y)
{
	uint32_t min_border = window->drawable.width + window->border_width;
	if (rect->right < min_border)
		return 0;
	uint8_t *base = (uint8_t*)xsrv->backend.data;
	base += (adj_x + rect->right) * 4;
	base += (adj_y + rect->top) * xsrv->backend.pitch;
	do
	{
		uint8_t *dst = base;
		for (size_t y = rect->top; y <= rect->bottom; ++y)
		{
			*(uint32_t*)dst = window->attributes.border_pixel;
			dst += xsrv->backend.pitch;
		}
		--rect->right;
		if (rect->right < rect->left)
			return 1;
		base -= 4;
	} while (rect->right >= min_border);
	return 0;
}

static int
draw_top_border(struct xsrv *xsrv,
                struct rect *rect,
                struct window *window,
                int32_t adj_x,
                int32_t adj_y)
{
	if (rect->top >= window->border_width)
		return 0;
	uint8_t *base = (uint8_t*)xsrv->backend.data;
	base += (adj_x + rect->left) * 4;
	base += (adj_y + rect->top) * xsrv->backend.pitch;
	do
	{
		uint8_t *dst = base;
		for (size_t x = rect->left; x <= rect->right; ++x)
		{
			*(uint32_t*)dst = window->attributes.border_pixel;
			dst += 4;
		}
		++rect->top;
		if (rect->top > rect->bottom)
			return 1;
		base += xsrv->backend.pitch;
	} while (rect->top < window->border_width);
	return 0;
}

static int
draw_bottom_border(struct xsrv *xsrv,
                   struct rect *rect,
                   struct window *window,
                   int32_t adj_x,
                   int32_t adj_y)
{
	uint32_t min_border = window->drawable.height + window->border_width;
	if (rect->bottom < min_border)
		return 0;
	uint8_t *base = (uint8_t*)xsrv->backend.data;
	base += (adj_x + rect->left) * 4;
	base += (adj_y + rect->bottom) * xsrv->backend.pitch;
	do
	{
		uint8_t *dst = base;
		for (size_t x = rect->left; x <= rect->right; ++x)
		{
			*(uint32_t*)dst = window->attributes.border_pixel;
			dst += 4;
		}
		--rect->bottom;
		if (rect->bottom < rect->top)
			return 1;
		base -= xsrv->backend.pitch;
	} while (rect->bottom >= min_border);
	return 0;
}

void
redraw_rect(struct xsrv *xsrv,
            struct rect rect,
            struct window *window,
            struct window *child,
            int32_t adj_x,
            int32_t adj_y)
{
	for (; child; child = TAILQ_NEXT(child, chain))
	{
		if (!child->drawable.width
		 || !child->drawable.height
		 || !(child->flags & WINDOW_MAPPED))
			continue;
		struct rect inter;
		{
			struct rect child_rect;
			int32_t left = child->x + window->border_width;
			int32_t top = child->y + window->border_width;
			int32_t right = left + child->drawable.width + child->border_width * 2 - 1;
			int32_t bottom = top + child->drawable.height + child->border_width * 2 - 1;
			if (right < 0 || bottom < 0)
				continue;
			child_rect.left = left > 0 ? left : 0;
			child_rect.right = right;
			child_rect.top = top > 0 ? top : 0;
			child_rect.bottom = bottom;
			if (!rect_intersect(&inter, &rect, &child_rect))
				continue;
		}
		if (inter.left > rect.left)
		{
			struct rect left_rect;
			left_rect.left = rect.left;
			left_rect.right = inter.left - 1;
			left_rect.top = rect.top;
			left_rect.bottom = rect.bottom;
			redraw_rect(xsrv,
			            left_rect,
			            window,
			            child,
			            adj_x,
			            adj_y);
			rect.left = inter.left;
		}
		if (inter.right < rect.right)
		{
			struct rect right_rect;
			right_rect.left = inter.right + 1;
			right_rect.right = rect.right;
			right_rect.top = rect.top;
			right_rect.bottom = rect.bottom;
			redraw_rect(xsrv,
			            right_rect,
			            window,
			            child,
			            adj_x,
			            adj_y);
			rect.right = inter.right;
		}
		if (inter.top > rect.top)
		{
			struct rect top_rect;
			top_rect.top = rect.top;
			top_rect.bottom = inter.top - 1;
			top_rect.left = rect.left;
			top_rect.right = rect.right;
			redraw_rect(xsrv,
			            top_rect,
			            window,
			            child,
			            adj_x,
			            adj_y);
			rect.right = inter.right;
		}
		if (inter.bottom < rect.bottom)
		{
			struct rect bottom_rect;
			bottom_rect.top = inter.bottom + 1;
			bottom_rect.bottom = rect.bottom;
			bottom_rect.left = rect.left;
			bottom_rect.right = rect.right;
			redraw_rect(xsrv,
			            bottom_rect,
			            window,
			            child,
			            adj_x,
			            adj_y);
			rect.bottom = inter.bottom;
		}
		int32_t adjust_x = child->x + window->border_width;
		int32_t adjust_y = child->y + window->border_width;
		inter.left -= adjust_x;
		inter.right -= adjust_x;
		inter.top -= adjust_y;
		inter.bottom -= adjust_y;
		redraw_rect(xsrv,
		            inter,
		            child,
		            TAILQ_FIRST(&child->children),
		            adj_x + adjust_x,
		            adj_y + adjust_y);
		return;
	}
	if (window->border_width)
	{
		if (draw_left_border(xsrv, &rect, window, adj_x, adj_y))
			return;
		if (draw_right_border(xsrv, &rect, window, adj_x, adj_y))
			return;
		if (draw_top_border(xsrv, &rect, window, adj_x, adj_y))
			return;
		if (draw_bottom_border(xsrv, &rect, window, adj_x, adj_y))
			return;
	}
	uint8_t *dst = (uint8_t*)xsrv->backend.data;
	dst += (adj_y + rect.top) * xsrv->backend.pitch;
	dst += (adj_x + rect.left) * 4;
	uint8_t *src = (uint8_t*)window->drawable.data;
	src += (rect.top - window->border_width) * window->drawable.pitch;
	src += (rect.left - window->border_width) * 4;
	uint32_t bytes = (rect.right - rect.left + 1) * 4;
	for (uint32_t y = rect.top; y <= rect.bottom; ++y)
	{
		memcpy(dst, src, bytes);
		dst += xsrv->backend.pitch;
		src += window->drawable.pitch;
	}
}

static int
get_cursor_pixel(struct xsrv *xsrv,
                 struct cursor *cursor,
                 int16_t cx,
                 int16_t cy,
                 uint32_t *pixel)
{
	if (cursor->mask)
	{
		if (cx < 0 || cx >= cursor->mask->drawable.width
		 || cy < 0 || cy >= cursor->mask->drawable.height)
			return 0;
		if (!drawable_get_pixel(xsrv, &cursor->mask->drawable, cx, cy))
			return 0;
		cx -= cursor->maskx;
		cy -= cursor->masky;
	}
	if (cx >= 0 && cx < cursor->color->drawable.width
	 && cy >= 0 && cy < cursor->color->drawable.height
	 && drawable_get_pixel(xsrv, &cursor->color->drawable, cx, cy))
	{
		*pixel = ((cursor->fore_red   & 0xFF00) << 8)
		       | ((cursor->fore_green & 0xFF00) << 0)
		       | ((cursor->fore_blue  & 0xFF00) >> 8);
	}
	else
	{
		*pixel = ((cursor->back_red   & 0xFF00) << 8)
		       | ((cursor->back_green & 0xFF00) << 0)
		       | ((cursor->back_blue  & 0xFF00) >> 8);
	}
	return 1;
}

static void
redraw_cursor(struct xsrv *xsrv)
{
	struct rect cursor_rect;
	struct cursor *cursor = xsrv->pointer.cursor;
	int32_t cursor_left;
	int32_t cursor_right;
	int32_t cursor_top;
	int32_t cursor_bottom;
	int32_t offset_x;
	int32_t offset_y;

	if (cursor->mask)
	{
		cursor_left = xsrv->pointer.x - cursor->xhot - cursor->maskx;
		cursor_right = cursor_left + cursor->mask->drawable.width - 1;
		cursor_top = xsrv->pointer.y - cursor->yhot - cursor->masky;
		cursor_bottom = cursor_top + cursor->mask->drawable.height - 1;
	}
	else
	{
		cursor_left = xsrv->pointer.x - cursor->xhot;
		cursor_right = cursor_left + cursor->color->drawable.width - 1;
		cursor_top = xsrv->pointer.y - cursor->yhot;
		cursor_bottom = cursor_top + cursor->color->drawable.height - 1;
	}
	if (cursor_right < 0 || cursor_bottom < 0)
		return;
	if (cursor_left < 0)
	{
		cursor_rect.left = 0;
		offset_x = -cursor_left;
	}
	else
	{
		cursor_rect.left = cursor_left;
		offset_x = 0;
	}
	if (cursor_top < 0)
	{
		cursor_rect.top = 0;
		offset_y = -cursor_top;
	}
	else
	{
		cursor_rect.top = cursor_top;
		offset_y = 0;
	}
	cursor_rect.right = cursor_right;
	cursor_rect.bottom = cursor_bottom;
	struct rect inter;
	if (!rect_intersect(&inter, &xsrv->redraw_rect, &cursor_rect))
		return;
	offset_x += inter.left - cursor_rect.left;
	offset_y += inter.top - cursor_rect.top;
	uint32_t width = inter.right - inter.left;
	uint32_t height = inter.bottom - inter.top;
	uint8_t *dst = (uint8_t*)xsrv->backend.data;
	dst += inter.top * xsrv->backend.pitch;
	dst += inter.left * 4;
	for (uint32_t y = 0; y <= height; ++y)
	{
		uint32_t *dst_line = (uint32_t*)dst;
		for (uint32_t x = 0; x <= width; ++x)
		{
			uint32_t pixel;
			if (!get_cursor_pixel(xsrv,
			                      cursor,
			                      offset_x + x,
			                      offset_y + y,
			                      &pixel))
				continue;
			dst_line[x] = pixel;
		}
		dst += xsrv->backend.pitch;
	}
}

void
framebuffer_update(struct xsrv *xsrv, struct rect *rect)
{
	redraw_rect(xsrv,
	            *rect,
	            xsrv->screens[0]->root,
	            TAILQ_FIRST(&xsrv->screens[0]->root->children),
	            0,
	            0);
	redraw_cursor(xsrv);
}
