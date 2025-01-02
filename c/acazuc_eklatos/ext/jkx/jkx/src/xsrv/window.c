#include "xsrv.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static void window_destroy(struct xsrv *xsrv, struct object *object);
static void window_free(struct xsrv *xsrv, struct object *object);

static const struct object_def window_def =
{
	.name = "window",
	.destroy = window_destroy,
	.free = window_free,
};

void window_register(struct xsrv *xsrv)
{
	xsrv->obj_window = register_object_type(xsrv, &window_def);
}

struct window *window_new(struct xsrv *xsrv, struct client *client, uint32_t id,
                          struct window *parent, int16_t x, int16_t y,
                          uint16_t width, uint16_t height,
                          const struct format *format, uint16_t border_width,
                          struct visual *visual, uint16_t class,
                          struct window_attributes *attributes)
{
	struct window *window = malloc(sizeof(*window));
	if (!window)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	if (drawable_init(xsrv, &window->drawable, width, height, format,
	                   parent ? parent->drawable.root : NULL))
	{
		free(window);
		return NULL;
	}
	object_init(xsrv, client, &window->object, xsrv->obj_window, id);
	window->tree_depth = parent ? parent->tree_depth + 1 : 0;
	window->parent = parent;
	window->visual = visual;
	window->class = class;
	window->x = x;
	window->y = y;
	window->flags = 0;
	window->border_width = border_width;
	window->screen = parent ? parent->screen : NULL;
	window->attributes = *attributes;
	TAILQ_INIT(&window->children);
	TAILQ_INIT(&window->properties);
	TAILQ_INIT(&window->button_grabs);
	if (parent)
		TAILQ_INSERT_TAIL(&parent->children, window, chain);
	TAILQ_INIT(&window->events);
	return window;
}

static void window_free(struct xsrv *xsrv, struct object *object)
{
	struct window *window = (struct window*)object;
	object_free(xsrv, OBJECT(window->attributes.background_pixmap));
	object_free(xsrv, OBJECT(window->attributes.border_pixmap));
	object_free(xsrv, OBJECT(window->attributes.colormap));
	object_free(xsrv, OBJECT(window->attributes.cursor));
	free(window->drawable.data);
}

static void window_destroy(struct xsrv *xsrv, struct object *object)
{
	struct window *window = (struct window*)object;
	if (window->parent)
		TAILQ_REMOVE(&window->parent->children, window, chain);
	struct window_event *event = TAILQ_FIRST(&window->events);
	while (event)
	{
		TAILQ_REMOVE(&event->window->events, event, window_chain);
		TAILQ_REMOVE(&event->client->events, event, client_chain);
		free(event);
		event = TAILQ_FIRST(&window->events);
	}
	struct window *child = TAILQ_FIRST(&window->children);
	while (child)
	{
		object_destroy(xsrv, OBJECT(child));
		child = TAILQ_FIRST(&window->children);
	}
	window_unmap(xsrv, window);
	window_destroy_notify(xsrv, window);
	struct property *property = TAILQ_FIRST(&window->properties);
	while (property)
	{
		TAILQ_REMOVE(&window->properties, property, chain);
		free(property->data);
		free(property);
		property = TAILQ_FIRST(&window->properties);
	}
	struct button_grab *button_grab = TAILQ_FIRST(&window->button_grabs);
	while (button_grab)
	{
		delete_button_grab(xsrv, button_grab);
		button_grab = TAILQ_FIRST(&window->button_grabs);
	}
}

struct window *window_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct window*)object_get_typed(xsrv, id, xsrv->obj_window);
}

struct window *get_window_at(struct xsrv *xsrv, uint32_t *x, uint32_t *y)
{
	struct window *window = xsrv->screens[0]->root;
	struct window *child;
	do
	{
		TAILQ_FOREACH(child, &window->children, chain)
		{
			if (!(child->flags & WINDOW_MAPPED))
				continue;
			int32_t cl = child->x;
			int32_t cr = child->x + child->drawable.width
			           + child->border_width * 2;
			int32_t ct = child->y;
			int32_t cb = child->y + child->drawable.height
			           + child->border_width * 2;
			if ((cl > 0 && (uint32_t)cl > *x)
			 || (ct > 0 && (uint32_t)ct > *y)
			 || (cr < 0 || (uint32_t)cr <= *x)
			 || (cb < 0 || (uint32_t)cb <= *y))
				continue;
			*x -= cl + child->border_width;
			*y -= ct + child->border_width;
			window = child;
			break;
		}
	} while (child);
	*x += window->border_width;
	*y += window->border_width;
	return window;
}

int window_visible(struct window *window)
{
	if (!(window->flags & WINDOW_MAPPED))
		return 0;
	if (!window->parent)
		return 1;
	return window_visible(window->parent);
}

void window_map(struct xsrv *xsrv, struct client *client, struct window *window)
{
	if (window->flags & WINDOW_MAPPED)
		return;
	if (window->class == InputOnly)
		return;
	if (!window->attributes.override_redirect
	 && window_map_request(xsrv, client, window))
		return;
	window->flags |= WINDOW_MAPPED;
	window_redraw(xsrv, window);
	window_map_notify(xsrv, window);
	/* XXX enter notify */
	/* XXX visibility notify */
	window_expose(xsrv, window);
}

void window_unmap(struct xsrv *xsrv, struct window *window)
{
	if (!(window->flags & WINDOW_MAPPED))
		return;
	window->flags &= ~WINDOW_MAPPED;
	window_redraw(xsrv, window);
	window_unmap_notify(xsrv, window);
	/* XXX leave notify */
	/* XXX visibility notify */
	if (window == xsrv->pointer.window)
		xsrv_cursor_motion(xsrv, xsrv->pointer.x, xsrv->pointer.y);
	if (xsrv->focus.window == window)
	{
		xsrv_revert_focus(xsrv);
	}
	else if (xsrv->focus.window)
	{
		struct window *tmp = xsrv->focus.window;
		while (tmp->tree_depth > window->tree_depth)
			tmp = tmp->parent;
		if (tmp == window)
			xsrv_revert_focus(xsrv);
	}
	if (xsrv->pointer_grab.window == window)
	{
		xsrv_ungrab_pointer(xsrv);
	}
	else if (xsrv->pointer_grab.window)
	{
		struct window *tmp = xsrv->pointer_grab.window;
		while (tmp->tree_depth > window->tree_depth)
			tmp = tmp->parent;
		if (tmp == window)
			xsrv_ungrab_pointer(xsrv);
	}
	/* XXX pointer grab confine_to */
}

struct cursor *window_get_cursor(struct xsrv *xsrv, struct window *window)
{
	if (window->attributes.cursor)
		return window->attributes.cursor;
	return window_get_cursor(xsrv, window->parent);
}

static void copy_data(struct window *window, void *data, uint32_t dst_pitch,
                      uint32_t width, uint32_t height,
                      int32_t dst_x, int32_t dst_y)
{
	int32_t src_x;
	int32_t src_y;
	if (width > window->drawable.width)
		width = window->drawable.width;
	if (height > window->drawable.height)
		height = window->drawable.height;
	if (dst_x < 0)
	{
		if ((uint32_t)-dst_x >= width)
			return;
		width += dst_x;
		src_x = -dst_x;
		dst_x = 0;
	}
	else
	{
		src_x = 0;
	}
	if (dst_y < 0)
	{
		if ((uint32_t)-dst_y >= height)
			return;
		height += dst_y;
		src_y = -dst_y;
		dst_y = 0;
	}
	else
	{
		src_y = 0;
	}
	if (data == window->drawable.data
	 && dst_pitch == window->drawable.pitch
	 && src_x == dst_x
	 && src_y == dst_y)
		return;
	uint32_t bpp = window->drawable.format->bpp / 8;
	uint8_t *src = window->drawable.data;
	src += src_x * bpp;
	src += src_y * window->drawable.pitch;
	uint8_t *dst = data;
	dst += dst_x * bpp;
	dst += dst_y * dst_pitch;
	uint32_t bytes = width * bpp;
	for (uint32_t y = 0; y < height; ++y)
	{
		memcpy(dst, src, bytes);
		dst += dst_pitch;
		src += window->drawable.pitch;
	}
}

int window_resize(struct xsrv *xsrv, struct window *window,
                  uint16_t width, uint16_t height)
{
	struct drawable *drawable = &window->drawable;
	if (width == drawable->width && height == drawable->height)
		return 0;
	uint32_t alloc_width = npot32(width);
	uint32_t alloc_height = npot32(height);
	uint32_t tmp = alloc_width * drawable->format->bpp;
	tmp = tmp + drawable->format->scanline_pad - 1;
	tmp -= tmp % drawable->format->scanline_pad;
	uint32_t alloc_pitch = tmp / 8;
	if (window->attributes.bit_gravity == ForgetGravity)
	{
		/* only realloc if current block is too small
		 * or if we are more than 2 times smaller to avoid
		 * back-and-forth allocation on power of two edge
		 */
		bool alloc = false;
		if (alloc_width < drawable->alloc_width / 2)
		{
			alloc_width *= 2;
			alloc = true;
		}
		if (alloc_height < drawable->alloc_height / 2)
		{
			alloc_height *= 2;
			alloc = true;
		}
		if (alloc
		 || !drawable->alloc_width
		 || !drawable->alloc_height
		 || alloc_width > drawable->alloc_width
		 || alloc_height > drawable->alloc_height)
		{
			void *data = realloc(drawable->data,
			                     alloc_height * alloc_pitch + 1);
			if (!data)
			{
				fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
				        strerror(errno));
				return 1;
			}
			drawable->data = data;
			drawable->pitch = alloc_pitch;
			drawable->alloc_width = alloc_width;
			drawable->alloc_height = alloc_height;
		}
		drawable->width = width;
		drawable->height = height;
		window_expose(xsrv, window);
		return 0;
	}
	/* only realloc if current block is too small
	 * or if we are more than 2 times smaller to avoid
	 * back-and-forth allocation on power of two edge
	 */
	bool alloc = false;
	if (alloc_width < drawable->alloc_width / 2)
	{
		alloc_width *= 2;
		alloc = true;
	}
	if (alloc_height < drawable->alloc_height / 2)
	{
		alloc_height *= 2;
		alloc = true;
	}
	void *data = drawable->data;
	uint32_t pitch = drawable->pitch;
	if (alloc
	 || !drawable->alloc_width
	 || !drawable->alloc_height
	 || alloc_width > drawable->alloc_width
	 || alloc_height > drawable->alloc_height)
	{
		data = malloc(alloc_height * alloc_pitch + 1);
		if (!data)
		{
			fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
			        strerror(errno));
			return 1;
		}
		memset(data, 0, alloc_height * alloc_pitch + 1);
		pitch = alloc_pitch;
	}
	switch (window->attributes.bit_gravity)
	{
		default:
		case ForgetGravity:
			break;
		case NorthWestGravity:
			copy_data(window, data, pitch, width, height,
			          0,
			          0);
			break;
		case NorthGravity:
			copy_data(window, data, pitch, width, height,
			          (width - window->drawable.width) / 2,
			          0);
			break;
		case NorthEastGravity:
			copy_data(window, data, pitch, width, height,
			          width - window->drawable.width,
			          0);
			break;
		case WestGravity:
			copy_data(window, data, pitch, width, height,
			          0,
			          (height - window->drawable.height) / 2);
			break;
		case StaticGravity: /* XXX keep same data relative to root */
		case CenterGravity:
			copy_data(window, data, pitch, width, height,
			          (width - window->drawable.width) / 2,
			          (height - window->drawable.height) / 2);
			break;
		case EastGravity:
			copy_data(window, data, pitch, width, height,
			          width - window->drawable.width,
			          (height - window->drawable.height) / 2);
			break;
		case SouthWestGravity:
			copy_data(window, data, pitch, width, height,
			          0,
			          height - window->drawable.height);
			break;
		case SouthGravity:
			copy_data(window, data, pitch, width, height,
			          (width - window->drawable.width) / 2,
			          height - window->drawable.height);
			break;
		case SouthEastGravity:
			copy_data(window, data, pitch, width, height,
			          width - window->drawable.width,
			          height - window->drawable.height);
			break;
	}
	if (data != drawable->data)
	{
		free(drawable->data);
		drawable->data = data;
		drawable->pitch = pitch;
		drawable->alloc_width = alloc_width;
		drawable->alloc_height = alloc_height;
	}
	drawable->width = width;
	drawable->height = height;
	window_expose(xsrv, window);
	return 0;
}

void window_get_fb_rect(struct xsrv *xsrv, struct window *window,
                        uint32_t *x, uint32_t *y,
                        uint32_t *width, uint32_t *height)
{
begin:
	(void)xsrv;
	if (!(window->flags & WINDOW_MAPPED))
	{
		*width = 0;
		*height = 0;
		return;
	}
	if (*x + *width > window->drawable.width)
		*width = window->drawable.width - *x;
	if (*y + *height > window->drawable.height)
		*height = window->drawable.height - *y;
	if (!window->parent)
		return;
	int16_t left = window->x + window->border_width;
	if (left < 0)
	{
		uint16_t abs = -left;
		if (abs > *x + *width)
		{
			*width = 0;
			return;
		}
		if (abs > *x)
		{
			*width -= abs - *x;
			*x = 0;
		}
		else
		{
			*x -= abs;
		}
	}
	else
	{
		*x += left;
	}
	int16_t top = window->y + window->border_width;
	if (top < 0)
	{
		uint16_t abs = -top;
		if (abs > *height)
		{
			*height = 0;
			return;
		}
		if (abs > *y)
		{
			*height -= abs - *y;
			*y = 0;
		}
		else
		{
			*y -= abs;
		}
	}
	else
	{
		*y += top;
	}
	window = window->parent;
	goto begin;
}

void window_get_full_rect(struct xsrv *xsrv, struct window *window,
                          uint32_t *x, uint32_t *y,
                          uint32_t *width, uint32_t *height)
{
	if (window->x < 0)
	{
		*x = 0;
		*width = window->drawable.width + window->border_width * 2;
		if ((uint32_t)-window->x >= *width)
		{
			*x = 0;
			*y = 0;
			*width = 0;
			*height = 0;
			return;
		}
		*width += window->x;
	}
	else
	{
		*x = window->x;
		*width = window->drawable.width + window->border_width * 2;
	}
	if (window->y < 0)
	{
		*y = 0;
		*height = window->drawable.height + window->border_width * 2;
		if ((uint32_t)-window->y >= *height)
		{
			*x = 0;
			*y = 0;
			*width = 0;
			*height = 0;
			return;
		}
		*height += window->y;
	}
	else
	{
		*y = window->y;
		*height = window->drawable.height + window->border_width * 2;
	}
	window_get_fb_rect(xsrv, window->parent, x, y, width, height);
}

void update_tree_depth(struct window *window)
{
	window->tree_depth = window->parent->tree_depth + 1;
	struct window *child;
	TAILQ_FOREACH(child, &window->children, chain)
		update_tree_depth(child);
}

void window_reparent(struct xsrv *xsrv, struct client *client,
                     struct window *window, struct window *parent,
                     int16_t x, int16_t y)
{
	struct window *oldparent = window->parent;
	uint32_t old_x;
	uint32_t old_y;
	uint32_t old_width;
	uint32_t old_height;
	window_get_full_rect(xsrv, window, &old_x, &old_y, &old_width,
	                     &old_height);
	int mapped = window->flags & WINDOW_MAPPED;
	window_unmap(xsrv, window);
	TAILQ_REMOVE(&window->parent->children, window, chain);
	TAILQ_INSERT_TAIL(&parent->children, window, chain);
	window->parent = parent;
	update_tree_depth(window);
	window->x = x;
	window->y = y;
	uint32_t new_x;
	uint32_t new_y;
	uint32_t new_width;
	uint32_t new_height;
	window_get_full_rect(xsrv, window, &new_x, &new_y, &new_width,
	                     &new_height);
	framebuffer_redraw(xsrv, old_x, old_y, old_width, old_height);
	framebuffer_redraw(xsrv, new_x, new_y, new_width, new_height);
	window_reparent_notify(xsrv, window, oldparent);
	if (mapped)
		window_map(xsrv, client, window);
}

struct property *window_get_property(struct window *window, struct atom *prop)
{
	struct property *property;
	TAILQ_FOREACH(property, &window->properties, chain)
	{
		if (property->property == prop)
			return property;
	}
	return NULL;
}

void window_delete_property(struct xsrv *xsrv, struct window *window,
                            struct atom *prop)
{
	struct property *property;
	TAILQ_FOREACH(property, &window->properties, chain)
	{
		if (property->property == prop)
		{
			TAILQ_REMOVE(&window->properties, property, chain);
			free(property->data);
			free(property);
			window_property_notify(xsrv, window, prop,
			                       PropertyDelete);
			return;
		}
	}
}

struct button_grab *window_get_button_grab(struct xsrv *xsrv,
                                           struct window *window,
                                           uint8_t button,
                                           uint16_t modifiers)
{
	(void)xsrv;
	struct button_grab *grab;
	TAILQ_FOREACH(grab, &window->button_grabs, window_chain)
	{
		if (button != AnyButton && grab->button != button)
			continue;
		if (!(grab->modifiers & modifiers))
			continue;
		return grab;
	}
	return NULL;
}

void window_remove_button_grab(struct xsrv *xsrv, struct window *window,
                               uint8_t button, uint16_t modifiers)
{
	(void)xsrv;
	struct button_grab *grab;
	struct button_grab *next;
	for (grab = TAILQ_FIRST(&window->button_grabs);
	     grab && (next = TAILQ_NEXT(grab, window_chain), 1);
	     grab = next)
	{
		if (button != AnyButton && grab->button != button)
			continue;
		if (modifiers != AnyModifier && grab->modifiers != modifiers)
			continue;
		delete_button_grab(xsrv, grab);
	}
}

void window_redraw(struct xsrv *xsrv, struct window *window)
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	window_get_full_rect(xsrv, window, &x, &y, &width, &height);
	framebuffer_redraw(xsrv, x, y, width, height);
}

void window_set_cursor(struct xsrv *xsrv, struct window *window,
                       struct cursor *cursor)
{
	object_free(xsrv, OBJECT(window->attributes.cursor));
	window->attributes.cursor = cursor;
	if (xsrv->pointer.window == window)
	{
		xsrv_cursor_motion(xsrv, xsrv->pointer.x, xsrv->pointer.y);
	}
	else if (xsrv->pointer.window)
	{
		struct window *tmp = xsrv->pointer.window;
		while (tmp->tree_depth > window->tree_depth)
			tmp = tmp->parent;
		if (tmp == window)
			xsrv_cursor_motion(xsrv, xsrv->pointer.x,
			                   xsrv->pointer.y);
	}
}

void window_clear(struct xsrv *xsrv, struct window *window,
                  int16_t x, int16_t y, uint16_t width, uint16_t height)
{
	if (x < 0)
	{
		if ((uint16_t)-x >= width)
			return;
		width += x;
		x = 0;
	}
	else if ((uint16_t)x >= window->drawable.width)
	{
		return;
	}
	if (x + width > window->drawable.width)
		width = window->drawable.width - x;
	if (y < 0)
	{
		if ((uint16_t)-y >= height)
			return;
		height += y;
		y = 0;
	}
	else if ((uint16_t)y >= window->drawable.height)
	{
		return;
	}
	if (y + height > window->drawable.height)
		height = window->drawable.height - y;
	/* XXX use background pixmap / color */
	uint8_t *dst = window->drawable.data;
	dst += y * window->drawable.pitch;
	dst += x * window->drawable.format->bpp / 8;
	uint32_t cpy_len = width * window->drawable.format->bpp / 8;
	for (uint32_t yy = 0; yy < height; ++yy)
	{
		memset(dst, 0, cpy_len);
		dst += window->drawable.pitch;
	}
}

void window_button_press(struct xsrv *xsrv, struct window *window,
                         uint8_t button, uint32_t wx, uint32_t wy)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & ButtonPressMask))
			continue;
		/* XXX child */
		ev_button_press(xsrv, event->client, button, millitime(),
		                window->drawable.root, window, NULL,
		                xsrv->pointer.x, xsrv->pointer.y, wx, wy,
		                xsrv->keybutmask, 1);
	}
}

void window_button_release(struct xsrv *xsrv, struct window *window,
                           uint8_t button, uint32_t wx, uint32_t wy)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & ButtonReleaseMask))
			continue;
		/* XXX child */
		ev_button_release(xsrv, event->client, button, millitime(),
		                  window->drawable.root, window, NULL,
		                  xsrv->pointer.x, xsrv->pointer.y, wx, wy,
		                  xsrv->keybutmask, 1);
	}
}

void window_motion_notify(struct xsrv *xsrv, struct window *window,
                          uint8_t detail, uint32_t wx, uint32_t wy)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & PointerMotionMask))
			continue;
		/* XXX child */
		ev_motion_notify(xsrv, event->client, detail, millitime(),
		                 window->drawable.root, window, NULL,
		                 xsrv->pointer.x, xsrv->pointer.y, wx, wy,
		                 xsrv->keybutmask, 1);
	}
}

void window_key_press(struct xsrv *xsrv, struct window *window, uint8_t key)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & KeyPressMask))
			continue;
		/* XXX child */
		ev_key_press(xsrv, event->client, key, millitime(),
		             window->drawable.root, window, NULL,
		             xsrv->pointer.x, xsrv->pointer.y,
		             xsrv->pointer.x - window->x,
		             xsrv->pointer.y - window->y,
		             xsrv->keybutmask, 1);
	}
}

void window_key_release(struct xsrv *xsrv, struct window *window, uint8_t key)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & KeyReleaseMask))
			continue;
		/* XXX child */
		ev_key_release(xsrv, event->client, key, millitime(),
		               window->drawable.root, window, NULL,
		               xsrv->pointer.x, xsrv->pointer.y,
		               xsrv->pointer.x - window->x,
		               xsrv->pointer.y - window->y,
		               xsrv->keybutmask, 1);
	}
}

void window_enter_notify(struct xsrv *xsrv, struct window *window,
                         uint8_t detail)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & EnterWindowMask))
			continue;
		/* XXX child */
		ev_enter_notify(xsrv, event->client, detail, millitime(),
		                window->drawable.root, window, NULL,
		                xsrv->pointer.x, xsrv->pointer.y,
		                xsrv->pointer.x - window->x,
		                xsrv->pointer.y - window->y,
		                xsrv->keybutmask, 1);
	}
}

void window_leave_notify(struct xsrv *xsrv, struct window *window,
                         uint8_t detail)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & LeaveWindowMask))
			continue;
		/* XXX child */
		ev_leave_notify(xsrv, event->client, detail, millitime(),
		                window->drawable.root, window, NULL,
		                xsrv->pointer.x, xsrv->pointer.y,
		                xsrv->pointer.x - window->x,
		                xsrv->pointer.y - window->y,
		                xsrv->keybutmask, 1);
	}
}

int window_map_request(struct xsrv *xsrv, struct client *client,
                       struct window *window)
{
	if (!window->parent)
		return 0;
	struct window_event *event;
	TAILQ_FOREACH(event, &window->parent->events, window_chain)
	{
		if (!(event->mask & SubstructureRedirectMask))
			continue;
		if (client == event->client)
			return 0;
		ev_map_request(xsrv, event->client, window->parent, window);
		return 1;
	}
	return 0;
}

void window_map_notify(struct xsrv *xsrv, struct window *window)
{
	if (window->parent)
	{
		struct window_event *event;
		TAILQ_FOREACH(event, &window->parent->events, window_chain)
		{
			if (!(event->mask & SubstructureNotifyMask))
				continue;
			ev_map_notify(xsrv, event->client, window->parent,
			              window,
			              window->attributes.override_redirect);
			break;
		}
	}
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & StructureNotifyMask))
			continue;
		ev_map_notify(xsrv, event->client, window, window,
		              window->attributes.override_redirect);
	}
}

void window_unmap_notify(struct xsrv *xsrv, struct window *window)
{
	if (window->parent)
	{
		struct window_event *event;
		TAILQ_FOREACH(event, &window->parent->events, window_chain)
		{
			if (!(event->mask & SubstructureNotifyMask))
				continue;
			ev_unmap_notify(xsrv, event->client, window->parent,
			              window,
			              window->attributes.override_redirect);
			break;
		}
	}
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & StructureNotifyMask))
			continue;
		ev_unmap_notify(xsrv, event->client, window, window,
		                window->attributes.override_redirect);
	}
}

void window_focus_in(struct xsrv *xsrv, struct window *window, uint8_t detail)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & FocusChangeMask))
			continue;
		/* XXX mode */
		ev_focus_in(xsrv, event->client, detail, window, NotifyNormal);
	}
}

void window_focus_out(struct xsrv *xsrv, struct window *window, uint8_t detail)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & FocusChangeMask))
			continue;
		/* XXX mode */
		ev_focus_out(xsrv, event->client, detail, window, NotifyNormal);
	}
}

void window_keymap_notify(struct xsrv *xsrv, struct window *window)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & KeymapStateMask))
			continue;
		ev_keymap_notify(xsrv, event->client, &xsrv->keyboard.state[1]);
	}
}

void window_reparent_notify(struct xsrv *xsrv, struct window *window,
                            struct window *oldparent)
{
	if (window->parent)
	{
		struct window_event *event;
		TAILQ_FOREACH(event, &window->parent->events, window_chain)
		{
			if (!(event->mask & SubstructureNotifyMask))
				continue;
			ev_reparent_notify(xsrv, event->client, window->parent,
			                   window, window->parent,
			                   window->x, window->y,
			                   window->attributes.override_redirect);
			break;
		}
	}
	if (oldparent)
	{
		struct window_event *event;
		TAILQ_FOREACH(event, &oldparent->events, window_chain)
		{
			if (!(event->mask & SubstructureNotifyMask))
				continue;
			ev_reparent_notify(xsrv, event->client, oldparent,
			                   window, window->parent,
			                   window->x, window->y,
			                   window->attributes.override_redirect);
			break;
		}
	}
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & StructureNotifyMask))
			continue;
		ev_reparent_notify(xsrv, event->client, window,
		                   window, window->parent,
		                   window->x, window->y,
		                   window->attributes.override_redirect);
	}
}

int window_resize_request(struct xsrv *xsrv, struct client *client,
                          struct window *window, uint16_t width,
                          uint16_t height)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (event->client == client)
			continue;
		if (!(event->mask & ResizeRedirectMask))
			continue;
		ev_resize_request(xsrv, event->client, window, width, height);
		return 1;
	}
	return 0;
}

void window_expose(struct xsrv *xsrv, struct window *window)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & ExposureMask))
			continue;
		/* XXX count, x, y, width, height */
		ev_expose(xsrv, event->client, window, 0, 0,
		          window->drawable.width, window->drawable.height, 0);
	}
}

void window_create_notify(struct xsrv *xsrv, struct window *window)
{
	if (!window->parent)
		return;
	struct window_event *event;
	TAILQ_FOREACH(event, &window->parent->events, window_chain)
	{
		if (!(event->mask & SubstructureNotifyMask))
			continue;
		ev_create_notify(xsrv, event->client, window->parent,
		                 window, window->x, window->y,
		                 window->drawable.width,
		                 window->drawable.height,
		                 window->border_width,
		                 window->attributes.override_redirect);
		break;
	}
}

void window_destroy_notify(struct xsrv *xsrv, struct window *window)
{
	if (window->parent)
	{
		struct window_event *event;
		TAILQ_FOREACH(event, &window->parent->events, window_chain)
		{
			if (!(event->mask & SubstructureNotifyMask))
				continue;
			ev_destroy_notify(xsrv, event->client, window->parent,
			                  window);
			break;
		}
	}
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & StructureNotifyMask))
			continue;
		ev_destroy_notify(xsrv, event->client, window, window);
	}
}

int window_configure_request(struct xsrv *xsrv, struct client *client,
                             struct window *window,  uint32_t value_mask,
                             int16_t x, int16_t y,
                             uint16_t width, uint16_t height,
                             uint16_t border_width, struct window *sibling,
                             uint8_t stack_mode)
{
	if (!window->parent)
		return 0;
	struct window_event *event;
	TAILQ_FOREACH(event, &window->parent->events, window_chain)
	{
		if (event->client == client)
			continue;
		if (!(event->mask & SubstructureNotifyMask))
			continue;
		ev_configure_request(xsrv, event->client, stack_mode,
		                     window->parent, window, sibling, x, y,
		                     width, height, border_width, value_mask);
		return 1;
	}
	return 0;
}

void window_configure_notify(struct xsrv *xsrv, struct window *window)
{
	if (window->parent)
	{
		struct window_event *event;
		TAILQ_FOREACH(event, &window->parent->events, window_chain)
		{
			if (!(event->mask & SubstructureNotifyMask))
				continue;
			ev_configure_notify(xsrv, event->client, window->parent,
			                    window, NULL, window->x, window->y,
			                    window->drawable.width,
			                    window->drawable.height,
			                    window->border_width,
			                    window->attributes.override_redirect);
			break;
		}
	}
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & StructureNotifyMask))
			continue;
		ev_configure_notify(xsrv, event->client, window,
		                    window, NULL, window->x, window->y,
		                    window->drawable.width,
		                    window->drawable.height,
		                    window->border_width,
		                    window->attributes.override_redirect);
	}
}

void window_property_notify(struct xsrv *xsrv, struct window *window,
                            struct atom *property, uint8_t state)
{
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (!(event->mask & PropertyChangeMask))
			continue;
		ev_property_notify(xsrv, event->client, window, property,
		                   millitime(), state);
	}
}
