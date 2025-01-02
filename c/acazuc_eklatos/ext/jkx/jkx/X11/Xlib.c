#include <X11/Xlibint.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <xcb/int.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

Display *XOpenDisplay(const char *display_name)
{
	Display *display = malloc(sizeof(*display));
	if (!display)
		return NULL;
	display->conn = xcb_connect(display_name, &display->screen);
	if (!display->conn)
	{
		free(display);
		return NULL;
	}
	display->synchronize = True;
	display->error_handler = NULL;
	display->cursor_font = XLoadFont(display, "cursor");
	display->black.pixel = 0x00000000;
	display->black.red = 0;
	display->black.green = 0;
	display->black.blue = 0;
	display->black.flags = 0;
	display->black.pad = 0;
	display->white.pixel = 0xFFFFFFFF;
	display->white.red = 0xFFFF;
	display->white.green = 0xFFFF;
	display->white.blue = 0xFFFF;
	display->white.flags = 0;
	display->white.pad = 0;
	XDisplayKeycodes(display, &display->min_keycode, &display->max_keycode);
	display->keysyms = XGetKeyboardMapping(display, display->min_keycode,
	                                       display->max_keycode - display->min_keycode,
	                                       &display->keysyms_per_keycode);
	if (!display->keysyms)
	{
		xcb_disconnect(display->conn);
		free(display);
		return NULL;
	}
	const xcb_setup_t *setup = xcb_get_setup(display->conn);
	display->screens_count = xcb_setup_roots_length(setup);
	display->screens = malloc(sizeof(*display->screens)
	                        * display->screens_count);
	if (!display->screens)
	{
		free(display->keysyms);
		xcb_disconnect(display->conn);
		free(display);
		return NULL;
	}
	xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
	for (int i = 0; i < display->screens_count; ++i)
	{
		display->screens[i].display = display;
		display->screens[i].screen = it.data;
		XGCValues gc_values;
		gc_values.background = it.data->black_pixel;
		gc_values.foreground = it.data->white_pixel;
		display->screens[i].default_gc = XCreateGC(display,
		                                           it.data->root,
		                                           GCForeground | GCBackground,
		                                           &gc_values);
		xcb_screen_next(&it);
	}
	return display;
}

void XCloseDisplay(Display *display)
{
	if (!display)
		return;
	free(display->screens);
	free(display->keysyms);
	xcb_disconnect(display->conn);
	free(display);
}

void XFree(void *ptr)
{
	free(ptr);
}

unsigned long XBlackPixel(Display *display, int screen)
{
	return BlackPixelOfScreen(ScreenOfDisplay(display, screen));
}

unsigned long XWhitePixel(Display *display, int screen)
{
	return WhitePixelOfScreen(ScreenOfDisplay(display, screen));
}

int XConnectionNumber(Display *display)
{
	return xcb_get_file_descriptor(display->conn);
}

Colormap XDefaultColormap(Display *display, int screen)
{
	return ScreenOfDisplay(display, screen)->screen->default_colormap;
}

int XDefaultDepth(Display *display, int screen)
{
	return ScreenOfDisplay(display, screen)->screen->root_depth;
}

int *XListDepths(Display *display, int screen, int *count)
{
	Screen *scr = ScreenOfDisplay(display, screen);
	*count = xcb_screen_allowed_depths_length(scr->screen);
	int *ret = malloc(sizeof(*ret) * *count);
	if (!ret)
		return NULL;
	xcb_depth_iterator_t it = xcb_screen_allowed_depths_iterator(scr->screen);
	for (int i = 0; i < *count; ++i)
	{
		ret[i] = it.data->depth;
		xcb_depth_next(&it);
	}
	return ret;
}

GC XDefaultGC(Display *display, int screen)
{
	return DefaultGCOfScreen(ScreenOfDisplay(display, screen));
}

Window XDefaultRootWindow(Display *display)
{
	return RootWindowOfScreen(DefaultScreenOfDisplay(display));
}

Screen *XDefaultScreenOfDisplay(Display *display)
{
	return ScreenOfDisplay(display, DefaultScreen(display));
}

int XDefaultScreen(Display *display)
{
	return display->screen;
}

Visual *XDefaultVisual(Display *display, int screen)
{
	return DefaultVisualOfScreen(ScreenOfDisplay(display, screen));
}

int XDisplayCells(Display *display, int screen)
{
	return CellsOfScreen(ScreenOfDisplay(display, screen));
}

int XDisplayPlanes(Display *display, int screen)
{
	return PlanesOfScreen(ScreenOfDisplay(display, screen));
}

char *XDisplayString(Display *display)
{
	return display->conn->display;
}

long XMaxRequestSize(Display *display)
{
	return xcb_get_setup(display->conn)->maximum_request_length;
}

long XExtendedMaxRequestSize(Display *display)
{
	return XMaxRequestSize(display);
}

unsigned long XLastKnownRequestProcessed(Display *display)
{
	(void)display;
	/* XXX */
	return 0;
}

unsigned long XNextRequest(Display *display)
{
	return display->conn->sequence + 1;
}

int XProtocolVersion(Display *display)
{
	return xcb_get_setup(display->conn)->protocol_major_version;
}

int XProtocolRevision(Display *display)
{
	return xcb_get_setup(display->conn)->protocol_minor_version;
}

int XQLength(Display *display)
{
	(void)display;
	/* XXX */
	return 0;
}

Window XRootWindow(Display *display, int screen)
{
	return ScreenOfDisplay(display, screen)->screen->root;
}

int XScreenCount(Display *display)
{
	return display->screens_count;
}

Screen *XScreenOfDisplay(Display *display, int screen)
{
	if (screen < 0 || screen >= display->screens_count)
		return NULL;
	return &display->screens[screen];
}

char *XServerVendor(Display *display)
{
	return xcb_setup_vendor(xcb_get_setup(display->conn));
}

int XVendorRelease(Display *display)
{
	return xcb_get_setup(display->conn)->release_number;
}

int XImageByteOrder(Display *display)
{
	return xcb_get_setup(display->conn)->image_byte_order;
}

int XBitmapBitOrder(Display *display)
{
	return xcb_get_setup(display->conn)->bitmap_format_bit_order;
}

int XBitmapPad(Display *display)
{
	return xcb_get_setup(display->conn)->bitmap_format_scanline_pad;
}

int XBitmapUnit(Display *display)
{
	return xcb_get_setup(display->conn)->bitmap_format_scanline_unit;
}

int XDisplayHeight(Display *display, int screen)
{
	return HeightOfScreen(ScreenOfDisplay(display, screen));
}

int XDisplayHeightMM(Display *display, int screen)
{
	return HeightMMOfScreen(ScreenOfDisplay(display, screen));
}

int XDisplayWidth(Display *display, int screen)
{
	return WidthOfScreen(ScreenOfDisplay(display, screen));
}

int XDisplayWidthMM(Display *display, int screen)
{
	return WidthMMOfScreen(ScreenOfDisplay(display, screen));
}

int XDisplayKeycodes(Display *display, int *min_keycodes,
                            int *max_keycodes)
{
	*min_keycodes = xcb_get_setup(display->conn)->min_keycode;
	*max_keycodes = xcb_get_setup(display->conn)->max_keycode;
	return Success;
}

VisualID XVisualIDFromVisual(Visual *visual)
{
	return visual->visual.visual_id;
}

XPixmapFormatValues *XListPixmapFormats(Display *display, int *count)
{
	const xcb_setup_t *setup = xcb_get_setup(display->conn);
	*count = xcb_setup_pixmap_formats_length(setup);
	XPixmapFormatValues *formats = malloc(sizeof(*formats) * *count);
	if (!formats)
		return NULL;
	xcb_format_iterator_t it = xcb_setup_pixmap_formats_iterator(setup);
	for (int i = 0; i < *count; ++i)
	{
		xcb_format_t *format = it.data;
		formats[i].depth = format->depth;
		formats[i].bits_per_pixel = format->bpp;
		formats[i].scanline_pad = format->scanline_pad;
		xcb_format_next(&it);
	}
	return formats;
}

unsigned long XDisplayMotionBufferSize(Display *display)
{
	const xcb_setup_t *setup = xcb_get_setup(display->conn);
	return setup->motion_buffer_size;
}

unsigned long XBlackPixelOfScreen(Screen *screen)
{
	return screen->screen->black_pixel;
}

unsigned long XWhitePixelOfScreen(Screen *screen)
{
	return screen->screen->white_pixel;
}

int XCellsOfScreen(Screen *screen)
{
	(void)screen;
	/* XXX */
	return 0;
}

Colormap XDefaultColormapOfScreen(Screen *screen)
{
	return screen->screen->default_colormap;
}

int XDefaultDepthOfScreen(Screen *screen)
{
	(void)screen;
	/* XXX */
	return 0;
}

GC XDefaultGCOfScreen(Screen *screen)
{
	return screen->default_gc;
}

Visual *XDefaultVisualOfScreen(Screen *screen)
{
	xcb_depth_iterator_t it = xcb_screen_allowed_depths_iterator(screen->screen);
	while (it.rem)
	{
		xcb_depth_t *depth = it.data;
		xcb_visualtype_iterator_t visual_it = xcb_depth_visuals_iterator(depth);
		while (visual_it.rem)
		{
			xcb_visualtype_t *visual = visual_it.data;
			if (visual->visual_id == screen->screen->root_visual)
				return (Visual*)visual;
			xcb_visualtype_next(&visual_it);
		}
		xcb_depth_next(&it);
	}
	return NULL;
}

int XDoesBackingStore(Screen *screen)
{
	return screen->screen->backing_stores;
}

Bool XDoesSaveUnders(Screen *screen)
{
	return screen->screen->save_unders;
}

Display *XDisplayOfScreen(Screen *screen)
{
	return screen->display;
}

int XScreenNumberOfScreen(Screen *screen)
{
	(void)screen;
	/* XXX */
	return 1;
}

long XEventMaskOfScreen(Screen *screen)
{
	return screen->screen->current_input_mask;
}

int XHeightOfScreen(Screen *screen)
{
	return screen->screen->height_in_pixels;
}

int XHeightMMOfScreen(Screen *screen)
{
	return screen->screen->height_in_millimeters;
}

int XMaxCmapsOfScreen(Screen *screen)
{
	return screen->screen->max_installed_maps;
}

int XMinCmapsOfScreen(Screen *screen)
{
	return screen->screen->min_installed_maps;
}

int XPlanesOfScreen(Screen *screen)
{
	return screen->screen->root_depth;
}

Window XRootWindowOfScreen(Screen *screen)
{
	return screen->screen->root;
}

int XWidthOfScreen(Screen *screen)
{
	return screen->screen->width_in_pixels;
}

int XWidthMMOfScreen(Screen *screen)
{
	return screen->screen->width_in_millimeters;
}

char **XListExtensions(Display *display, int *nextensions)
{
	REPLY_REQ(display, list_extensions);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	*nextensions = xcb_list_extensions_names_length(reply);
	char **extensions = malloc(sizeof(*extensions) * (*nextensions + 1));
	if (!extensions)
	{
		free(reply);
		return NULL;
	}
	xcb_str_iterator_t it = xcb_list_extensions_names_iterator(reply);
	for (int i = 0; i < *nextensions; ++i)
	{
		int len = xcb_str_name_length(it.data);
		extensions[i] = malloc(len + 1);
		if (!extensions[i])
		{
			for (int j = 0; j < i; ++j)
				free(extensions[j]);
			free(extensions);
			free(reply);
			return NULL;
		}
		memcpy(extensions[i], xcb_str_name(it.data), len);
		extensions[i][len] = '\0';
		xcb_str_next(&it);
	}
	extensions[*nextensions] = NULL;
	free(reply);
	return extensions;
}

Bool XQueryExtension(Display *display,
                     char *name,
                     int *major_opcode,
                     int *first_event,
                     int *first_error)
{
	REPLY_REQ(display, query_extension, strlen(name), name);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	if (major_opcode)
		*major_opcode = reply->major_opcode;
	if (first_event)
		*first_event = reply->first_event;
	if (first_error)
		*first_error = reply->first_error;
	Bool ret = reply->present;
	free(reply);
	return ret;
}

int XIfEvent(Display *display,
             XEvent *event_return,
             Bool (*predicate)(),
             XPointer arg)
{
	(void)display;
	(void)event_return;
	(void)predicate;
	(void)arg;
	/* XXX */
	return False;
}

Bool XCheckIfEvent(Display *display,
                   XEvent *event_return,
                   Bool (*predicate)(),
                   XPointer arg)
{
	(void)display;
	(void)event_return;
	(void)predicate;
	(void)arg;
	/* XXX */
	return False;
}

Bool XPeekIfEvent(Display *display,
                  XEvent *event_return,
                  Bool (*predicate)(),
                  XPointer arg)
{
	(void)display;
	(void)event_return;
	(void)predicate;
	(void)arg;
	/* XXX */
	return False;
}

int XNextEvent(Display *display, XEvent *event_return)
{
	xcb_generic_event_t *event = xcb_wait_for_event(display->conn);
	if (!event)
		return BadAlloc;
	xlib_copy_event(display, event_return, event);
	free(event);
	return Success;
}

int XPeekEvent(Display *display, XEvent *event_return)
{
	xcb_generic_event_t *event = xcb_peek_event(display->conn);
	if (!event)
		return BadAlloc; /* XXX */
	xlib_copy_event(display, event_return, event);
	free(event);
	return Success;
}

int XPutBackEvent(Display *display, XEvent *event)
{
	(void)display;
	(void)event;
	/* XXX */
	return False;
}

int XFlush(Display *display)
{
	if (!xcb_flush(display->conn))
		return BadRequest;
	return Success;
}

int XPending(Display *display)
{
	return xcb_pending_event(display->conn);
}

int XSync(Display *display, Bool discard)
{
	(void)discard; /* XXX */
	XGetInputFocus(display, NULL, NULL);
	return Success;
}

int XSynchronize(Display *display, int onoff)
{
	int prev = display->synchronize;
	display->synchronize = onoff;
	return prev;
}

XErrorHandler XSetErrorHandler(Display *display, XErrorHandler handler)

{
	XErrorHandler prev = display->error_handler;
	display->error_handler = handler;
	return prev;
}

static void gen_window_attributes(uint32_t *values,
                                  unsigned valuemask,
                                  XSetWindowAttributes *attributes)
{
	size_t n = 0;
	if (valuemask & CWBackPixmap)
		values[n++] = attributes->background_pixmap;
	if (valuemask & CWBackPixel)
		values[n++] = attributes->background_pixel;
	if (valuemask & CWBorderPixmap)
		values[n++] = attributes->border_pixmap;
	if (valuemask & CWBorderPixel)
		values[n++] = attributes->border_pixel;
	if (valuemask & CWBitGravity)
		values[n++] = attributes->bit_gravity;
	if (valuemask & CWWinGravity)
		values[n++] = attributes->win_gravity;
	if (valuemask & CWBackingStore)
		values[n++] = attributes->backing_store;
	if (valuemask & CWBackingPlanes)
		values[n++] = attributes->backing_planes;
	if (valuemask & CWBackingPixel)
		values[n++] = attributes->backing_pixel;
	if (valuemask & CWOverrideRedirect)
		values[n++] = attributes->override_redirect;
	if (valuemask & CWSaveUnder)
		values[n++] = attributes->save_under;
	if (valuemask & CWEventMask)
		values[n++] = attributes->event_mask;
	if (valuemask & CWDontPropagate)
		values[n++] = attributes->do_not_propagate_mask;
	if (valuemask & CWColormap)
		values[n++] = attributes->colormap;
	if (valuemask & CWCursor)
		values[n++] = attributes->cursor;
}

Window XCreateWindow(Display *display,
                     Window parent,
                     int x,
                     int y,
                     unsigned width,
                     unsigned height,
                     unsigned border_width,
                     int depth,
                     unsigned _class,
                     Visual *visual,
                     unsigned valuemask,
                     XSetWindowAttributes *attributes)
{
	uint32_t values[15];
	gen_window_attributes(values, valuemask, attributes);
	Window id = xcb_generate_id(display->conn);
	return XID_REQ(display, id, create_window,
	               depth,
	               id,
	               parent,
	               x,
	               y,
	               width,
	               height,
	               border_width,
	               _class,
	               visual ? visual->visual.visual_id : 0,
	               valuemask,
	               values);
}

Window XCreateSimpleWindow(Display *display,
                           Window parent,
                           int x,
                           int y,
                           unsigned width,
                           unsigned height,
                           unsigned border_width,
                           unsigned long border,
                           unsigned long background)
{
	XSetWindowAttributes attributes;
	attributes.background_pixel = background;
	attributes.border_pixel = border;
	return XCreateWindow(display, parent, x, y, width, height, border_width,
	                     0, InputOutput, NULL, CWBorderPixel | CWBackPixel,
	                     &attributes);
}

int XDestroyWindow(Display *display, Window window)
{
	return REQUEST(display, destroy_window, window);
}

int XDestroySubwindows(Display *display, Window window)
{
	return REQUEST(display, destroy_subwindows, window);
}

int XMapWindow(Display *display, Window window)
{
	return REQUEST(display, map_window, window);
}

int XUnmapWindow(Display *display, Window window)
{
	return REQUEST(display, unmap_window, window);
}

int XMapSubwindows(Display *display, Window window)
{
	return REQUEST(display, map_subwindows, window);
}

int XUnmapSubwindows(Display *display, Window window)
{
	return REQUEST(display, unmap_subwindows, window);
}

int XMapRaised(Display *display, Window window)
{
	return XMapWindow(display, window)
	    || XRaiseWindow(display, window);
}

Atom XInternAtom(Display *display, char *atom_name, Bool only_if_exists)
{
	REPLY_REQ(display, intern_atom,
	          only_if_exists,
	          strlen(atom_name),
	          atom_name);
	Atom ret;
	if (error)
	{
		xlib_handle_error(display, error);
		ret = error->response_type;
		free(error);
	}
	else if (!reply)
	{
		ret = None;
	}
	else
	{
		ret = reply->atom;
		free(reply);
	}
	return ret;
}

char *XGetAtomName(Display *display, Atom atom)
{
	REPLY_REQ(display, get_atom_name,
	          atom);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	char *name = malloc(reply->name_len + 1);
	if (name)
	{
		memcpy(name, xcb_get_atom_name_name(reply), reply->name_len);
		name[reply->name_len] = '\0';
	}
	free(reply);
	return name;
}

Status XGetAtomNames(Display *display, Atom *atoms, int count, char **names)
{
	if (count < 0)
		return 0;
	if (!count)
		return 1;
	xcb_get_atom_name_cookie_t *cookies = malloc(sizeof(*cookies) * count);
	if (!cookies)
		return 0;
	for (int i = 0; i < count; ++i)
		cookies[i] = xcb_get_atom_name(display->conn, atoms[i]);
	int i;
	for (i = 0; i < count; ++i)
	{
		xcb_generic_error_t *error = NULL;
		xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(display->conn,
		                                                           cookies[i],
		                                                           &error);
		if (error)
		{
			xlib_handle_error(display, error);
			free(error);
			goto err;
		}
		if (!reply)
			goto err;
		char *name = malloc(reply->name_len + 1);
		if (name)
		{
			memcpy(name, xcb_get_atom_name_name(reply), reply->name_len);
			name[reply->name_len] = '\0';
		}
		names[i] = name;
		free(reply);
	}
	free(cookies);
	return 1;

err:
	free(cookies);
	for (int j = 0; j < i; ++j)
		free(names[j]);
	return 0;
}

int XChangeProperty(Display *display,
                    Window window,
                    Atom property,
                    Atom type,
                    int format,
                    int mode,
                    const uint8_t *data,
                    int nelements)
{
	return REQUEST(display, change_property,
	               mode,
	               window,
	               property,
	               type,
	               format,
	               nelements,
	               data);
}

int XGetWindowProperty(Display *display,
                       Window window,
                       Atom property,
                       long long_offset,
                       long long_length,
                       Bool delete,
                       Atom req_type,
                       Atom *actual_type,
                       int *actual_format,
                       unsigned long *nitems,
                       unsigned long *bytes_after,
                       unsigned char **prop)
{
	REPLY_REQ(display, get_property,
	          delete,
	          window,
	          property,
	          req_type,
	          long_offset,
	          long_length);
	if (error)
	{
		xlib_handle_error(display, error);
		int ret = error->response_type;
		free(error);
		return ret;
	}
	if (!reply)
		return BadAlloc;
	if (actual_type)
		*actual_type = reply->type;
	if (actual_format)
		*actual_format = reply->format;
	if (nitems)
		*nitems = reply->value_len;
	if (bytes_after)
		*bytes_after = reply->bytes_after;
	if (prop)
	{
		size_t bytes = reply->value_len * reply->format / 8;
		*prop = malloc(bytes + 1);
		if (!*prop)
		{
			free(reply);
			return BadAlloc;
		}
		memcpy(*prop, xcb_get_property_value(reply), bytes);
		(*prop)[bytes] = 0;
	}
	free(reply);
	return Success;
}

Atom *XListProperties(Display *display, Window window, int *nprops)
{
	REPLY_REQ(display, list_properties,
	          window);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	*nprops = reply->atoms_len;
	Atom *props = malloc(sizeof(*props) * *nprops);
	if (!props)
	{
		free(reply);
		return NULL;
	}
	memcpy(props, xcb_list_properties_atoms(reply),
	       sizeof(*props) * *nprops);
	free(reply);
	return props;
}

int XDeleteProperty(Display *display, Window window, Atom property)
{
	return REQUEST(display, delete_property,
	               window,
	               property);
}

int XRotateWindowProperties(Display *display,
                            Window window,
                            Atom *properties,
                            int nprops,
                            int npositions)
{
	return REQUEST(display, rotate_properties,
	               window,
	               nprops,
	               npositions,
	               properties);
}

int XConfigureWindow(Display *display,
                     Window window,
                     unsigned value_mask,
                     XWindowChanges *values)
{
	uint32_t v[7];
	size_t n = 0;
	if (value_mask & CWX)
		v[n++] = values->x;
	if (value_mask & CWY)
		v[n++] = values->y;
	if (value_mask & CWWidth)
		v[n++] = values->width;
	if (value_mask & CWHeight)
		v[n++] = values->height;
	if (value_mask & CWBorderWidth)
		v[n++] = values->border_width;
	if (value_mask & CWSibling)
		v[n++] = values->sibling;
	if (value_mask & CWStackMode)
		v[n++] = values->stack_mode;
	return REQUEST(display, configure_window,
	               window,
	               value_mask,
	               v);
}

int XMoveWindow(Display *display, Window window, int x, int y)
{
	XWindowChanges changes;
	changes.x = x;
	changes.y = y;
	return XConfigureWindow(display, window, CWX | CWY, &changes);
}

int XResizeWindow(Display *display,
                  Window window,
                  unsigned width,
                  unsigned height)
{
	XWindowChanges changes;
	changes.width = width;
	changes.height = height;
	return XConfigureWindow(display, window, CWWidth | CWHeight, &changes);
}

int XMoveResizeWindow(Display *display,
                      Window window,
                      int x,
                      int y,
                      unsigned width,
                      unsigned height)
{
	XWindowChanges changes;
	changes.x = x;
	changes.y = y;
	changes.width = width;
	changes.height = height;
	return XConfigureWindow(display, window, CWX | CWY | CWWidth | CWHeight,
	                        &changes);
}

int XSetWindowBorderWidth(Display *display, Window window, unsigned width)
{
	XWindowChanges changes;
	changes.border_width = width;
	return XConfigureWindow(display, window, CWBorderWidth, &changes);
}

int XRaiseWindow(Display *display, Window window)
{
	XWindowChanges changes;
	changes.stack_mode = Above;
	return XConfigureWindow(display, window, CWStackMode, &changes);
}

int XLowerWindow(Display *display, Window window)
{
	XWindowChanges changes;
	changes.stack_mode = Below;
	return XConfigureWindow(display, window, CWStackMode, &changes);
}

int XCirculateSubwindows(Display *display, Window window, int direction)
{
	return REQUEST(display, circulate_window,
	               direction,
	               window);
}

int XCirculateSubwindowsUp(Display *display, Window window)
{
	return XCirculateSubwindows(display, window, RaiseLowest);
}

int XCirculateSubwindowsDown(Display *display, Window window)
{
	return XCirculateSubwindows(display, window, LowerHighest);
}

Status XGetGeometry(Display *display,
                    Drawable drawable,
                    Window *root,
                    int *x,
                    int *y,
                    unsigned *width,
                    unsigned *height,
                    unsigned *border_width,
                    unsigned *depth)
{
	REPLY_REQ(display, get_geometry, drawable);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	*root = reply->root;
	*x = reply->x;
	*y = reply->y;
	*width = reply->width;
	*height = reply->height;
	*border_width = reply->border_width;
	*depth = reply->depth;
	free(reply);
	return True;
}

Status XGetWindowAttributes(Display *display,
                            Window window,
                            XWindowAttributes *attributes)
{
	if (!XGetGeometry(display, window,
	                  &attributes->root,
	                  &attributes->x,
	                  &attributes->y,
	                  &attributes->width,
	                  &attributes->height,
	                  &attributes->border_width,
	                  &attributes->depth))
		return False;
	REPLY_REQ(display, get_window_attributes, window);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	attributes->class = reply->_class;
	attributes->bit_gravity = reply->bit_gravity;
	attributes->win_gravity = reply->win_gravity;
	attributes->backing_store = reply->backing_store;
	attributes->backing_planes = reply->backing_planes;
	attributes->backing_pixel = reply->backing_pixel;
	attributes->save_under = reply->save_under;
	attributes->colormap = reply->colormap;
	attributes->map_installed = reply->map_is_installed;
	attributes->map_state = reply->map_state;
	attributes->all_event_masks = reply->all_event_masks;
	attributes->your_event_mask = reply->your_event_mask;
	attributes->do_not_propagate_mask = reply->do_not_propagate_mask;
	attributes->override_redirect = reply->override_redirect;
	const xcb_setup_t *setup = xcb_get_setup(display->conn);
	xcb_screen_iterator_t screen_it = xcb_setup_roots_iterator(setup);
	for (size_t i = 0; screen_it.rem; xcb_screen_next(&screen_it), ++i)
	{
		xcb_screen_t *screen = screen_it.data;
		xcb_depth_iterator_t depth_it = xcb_screen_allowed_depths_iterator(screen);
		for (size_t j = 0; depth_it.rem; xcb_depth_next(&depth_it), ++j)
		{
			xcb_depth_t *depth = depth_it.data;
			xcb_visualtype_iterator_t visual_it = xcb_depth_visuals_iterator(depth);
			for (size_t k = 0; visual_it.rem; xcb_visualtype_next(&visual_it), ++k)
			{
				xcb_visualtype_t *visualtype = visual_it.data;
				if (visualtype->visual_id != reply->visual)
					continue;
				attributes->visual = (Visual*)visualtype;
				attributes->screen = ScreenOfDisplay(display, i);
				goto end;
			}
		}
	}
	attributes->visual = NULL;
	attributes->screen = NULL;
end:
	free(reply);
	return True;
}

int XChangeWindowAttributes(Display *display,
                            Window window,
                            unsigned long valuemask,
                            XSetWindowAttributes *attributes)
{
	uint32_t values[15];
	gen_window_attributes(values, valuemask, attributes);
	return REQUEST(display, change_window_attributes,
	               window,
	               valuemask,
	               values);
}

int XSetWindowBackground(Display *display,
                         Window window,
                         unsigned long background_pixel)
{
	XSetWindowAttributes attributes;
	attributes.background_pixel = background_pixel;
	return XChangeWindowAttributes(display, window, CWBackPixel,
	                               &attributes);
}

int XSetWindowBackgroundPixmap(Display *display,
                               Window window,
                               Pixmap background_pixmap)
{
	XSetWindowAttributes attributes;
	attributes.background_pixmap = background_pixmap;
	return XChangeWindowAttributes(display, window, CWBackPixmap,
	                               &attributes);
}

int XSetWindowBorder(Display *display,
                     Window window,
                     unsigned long border_pixel)
{
	XSetWindowAttributes attributes;
	attributes.border_pixel = border_pixel;
	return XChangeWindowAttributes(display, window, CWBorderPixel,
	                               &attributes);
}

int XSetWindowBorderPixmap(Display *display,
                           Window window,
                           Pixmap border_pixmap)
{
	XSetWindowAttributes attributes;
	attributes.border_pixmap = border_pixmap;
	return XChangeWindowAttributes(display, window, CWBorderPixmap,
	                               &attributes);
}

int XSetWindowColormap(Display *display,
                       Window window,
                       Colormap colormap)
{
	XSetWindowAttributes attributes;
	attributes.colormap = colormap;
	return XChangeWindowAttributes(display, window, CWColormap,
	                               &attributes);
}

int XDefineCursor(Display *display, Window window, Cursor cursor)
{
	XSetWindowAttributes attributes;
	attributes.cursor = cursor;
	return XChangeWindowAttributes(display, window, CWCursor, &attributes);
}

int XUndefineCursor(Display *display, Window window)
{
	return XDefineCursor(display, window, None);
}

Status XSelectInput(Display *display, Window window, uint32_t mask)
{
	XSetWindowAttributes attributes;
	attributes.event_mask = mask;
	return XChangeWindowAttributes(display, window, CWEventMask,
	                               &attributes);
}

int XReparentWindow(Display *display,
                    Window window,
                    Window parent,
                    int x,
                    int y)
{
	return REQUEST(display, reparent_window,
	               window,
	               parent,
	               x,
	               y);
}

static void gen_gc_values(uint32_t *v, unsigned valuemask, XGCValues *values)
{
	uint32_t n = 0;
	if (valuemask & GCFunction)
		v[n++] = values->function;
	if (valuemask & GCPlaneMask)
		v[n++] = values->plane_mask;
	if (valuemask & GCForeground)
		v[n++] = values->foreground;
	if (valuemask & GCBackground)
		v[n++] = values->background;
	if (valuemask & GCLineWidth)
		v[n++] = values->line_width;
	if (valuemask & GCLineStyle)
		v[n++] = values->line_style;
	if (valuemask & GCCapStyle)
		v[n++] = values->cap_style;
	if (valuemask & GCJoinStyle)
		v[n++] = values->join_style;
	if (valuemask & GCFillStyle)
		v[n++] = values->fill_style;
	if (valuemask & GCFillRule)
		v[n++] = values->fill_rule;
	if (valuemask & GCTile)
		v[n++] = values->tile;
	if (valuemask & GCStipple)
		v[n++] = values->stipple;
	if (valuemask & GCTileStipXOrigin)
		v[n++] = values->ts_x_origin;
	if (valuemask & GCTileStipYOrigin)
		v[n++] = values->ts_y_origin;
	if (valuemask & GCFont)
		v[n++] = values->font;
	if (valuemask & GCSubwindowMode)
		v[n++] = values->subwindow_mode;
	if (valuemask & GCGraphicsExposures)
		v[n++] = values->graphics_exposures;
	if (valuemask & GCClipXOrigin)
		v[n++] = values->clip_x_origin;
	if (valuemask & GCClipYOrigin)
		v[n++] = values->clip_y_origin;
	if (valuemask & GCClipMask)
		v[n++] = values->clip_mask;
	if (valuemask & GCDashOffset)
		v[n++] = values->dash_offset;
	if (valuemask & GCDashList)
		v[n++] = values->dashes;
	if (valuemask & GCArcMode)
		v[n++] = values->arc_mode;
}

GC XCreateGC(Display *display,
             Drawable drawable,
             unsigned long valuemask,
             XGCValues *values)
{
	GC gc = malloc(sizeof(*gc));
	if (!gc)
		return NULL;
	gc->gc = xcb_generate_id(display->conn);
	gc->display = display;
	gc->batch_mask = 0;
	uint32_t v[23];
	gen_gc_values(v, valuemask, values);
	XID xid = XID_REQ(display, gc->gc, create_gc,
	                  gc->gc,
	                  drawable,
	                  valuemask,
	                  v);
	if (!xid)
	{
		free(gc);
		return NULL;
	}
	return gc;
}

int XCopyGC(Display *display, GC src, GC dst, unsigned long valuemask)
{
	return REQUEST(display, copy_gc,
	               src->gc,
	               dst->gc,
	               valuemask);
}

int XFreeGC(Display *display, GC gc)
{
	if (!gc)
		return Success;
	Status status = REQUEST(display, free_gc, gc->gc);
	free(gc);
	return status;
}

int XDrawPoints(Display *display,
                Drawable drawable,
                GC gc,
                XPoint *points,
                int npoints,
                int mode)
{
	XFlushGC(gc);
	return REQUEST(display, poly_point,
	               mode,
	               drawable,
	               gc->gc,
	               npoints,
	               (xcb_point_t*)points);
}

int XDrawPoint(Display *display, Drawable drawable, GC gc, int x, int y)
{
	XFlushGC(gc);
	XPoint point;
	point.x = x;
	point.y = y;
	return XDrawPoints(display, drawable, gc, &point, 1, CoordModeOrigin);
}

int XDrawLines(Display *display,
               Drawable drawable,
               GC gc,
               XPoint *points,
               int npoints,
               int mode)
{
	XFlushGC(gc);
	return REQUEST(display, poly_line,
	               mode,
	               drawable,
	               gc->gc,
	               npoints,
	               (xcb_point_t*)points);
}

int XDrawLine(Display *display,
              Drawable drawable,
              GC gc,
              int x1,
              int y1,
              int x2,
              int y2)
{
	XFlushGC(gc);
	XPoint points[2];
	points[0].x = x1;
	points[0].y = y1;
	points[1].x = x2;
	points[1].y = y2;
	return XDrawLines(display, drawable, gc, points, 2, CoordModeOrigin);
}

int XDrawSegments(Display *display,
                  Drawable drawable,
                  GC gc,
                  XSegment *segments,
                  int nsegments)
{
	XFlushGC(gc);
	return REQUEST(display, poly_segment,
	               drawable,
	               gc->gc,
	               nsegments,
	               (xcb_segment_t*)segments);
}

int XDrawRectangles(Display *display,
                    Drawable drawable,
                    GC gc,
                    XRectangle *rectangles,
                    int nrectangles)
{
	XFlushGC(gc);
	return REQUEST(display, poly_rectangle,
	               drawable,
	               gc->gc,
	               nrectangles,
	               (xcb_rectangle_t*)rectangles);
}

int XDrawRectangle(Display *display,
                   Drawable drawable,
                   GC gc,
                   int x,
                   int y,
                   unsigned width,
                   unsigned height)
{
	XFlushGC(gc);
	XRectangle rectangle;
	rectangle.x = x;
	rectangle.y = y;
	rectangle.width = width;
	rectangle.height = height;
	return XDrawRectangles(display, drawable, gc, &rectangle, 1);
}

int XDrawArcs(Display *display,
              Drawable drawable,
              GC gc,
              XArc *arcs,
              int narcs)
{
	XFlushGC(gc);
	return REQUEST(display, poly_arc,
	               drawable,
	               gc->gc,
	               narcs,
	               (xcb_arc_t*)arcs);
}

int XDrawArc(Display *display,
             Drawable drawable,
             GC gc,
             int x,
             int y,
             unsigned width,
             unsigned height,
             int angle1,
             int angle2)
{
	XFlushGC(gc);
	XArc arc;
	arc.x = x;
	arc.y = y;
	arc.width = width;
	arc.height = height;
	arc.angle1 = angle1;
	arc.angle2 = angle2;
	return XDrawArcs(display, drawable, gc, &arc, 1);
}

int XFillPolygon(Display *display,
                 Drawable drawable, GC gc,
                 XPoint *points,
                 int npoints,
                 int shape,
                 int mode)
{
	XFlushGC(gc);
	return REQUEST(display, fill_poly,
	               drawable,
	               gc->gc,
	               shape,
	               mode,
	               npoints,
	               (xcb_point_t*)points);
}

int XFillRectangles(Display *display,
                    Drawable drawable,
                    GC gc,
                    XRectangle *rectangles,
                    int nrectangles)
{
	XFlushGC(gc);
	return REQUEST(display, poly_fill_rectangle,
	               drawable,
	               gc->gc,
	               nrectangles,
	               (xcb_rectangle_t*)rectangles);
}

int XFillRectangle(Display *display,
                   Drawable drawable,
                   GC gc,
                   int x,
                   int y,
                   unsigned width,
                   unsigned height)
{
	XFlushGC(gc);
	XRectangle rectangle;
	rectangle.x = x;
	rectangle.y = y;
	rectangle.width = width;
	rectangle.height = height;
	return XFillRectangles(display, drawable, gc, &rectangle, 1);
}

int XFillArcs(Display *display,
              Drawable drawable,
              GC gc,
              XArc *arcs,
              int narcs)
{
	XFlushGC(gc);
	return REQUEST(display, poly_fill_arc,
	               drawable,
	               gc->gc,
	               narcs,
	               (xcb_arc_t*)arcs);
}

int XFillArc(Display *display,
             Drawable drawable,
             GC gc,
             int x,
             int y,
             unsigned width,
             unsigned height,
             int angle1,
             int angle2)
{
	XFlushGC(gc);
	XArc arc;
	arc.x = x;
	arc.y = y;
	arc.width = width;
	arc.height = height;
	arc.angle1 = angle1;
	arc.angle2 = angle2;
	return XFillArcs(display, drawable, gc, &arc, 1);
}

Font XLoadFont(Display *display, char *name)
{
	Font font = xcb_generate_id(display->conn);
	return XID_REQ(display, font, open_font,
	               font,
	               strlen(name),
	               name);
}

int XUnloadFont(Display *display, Font font)
{
	return REQUEST(display, close_font, font);
}

int XDrawString(Display *display,
                Drawable drawable,
                GC gc,
                int x,
                int y,
                char *string,
                int length)
{
	(void)length; /* XXX */
	XFlushGC(gc);
	return REQUEST(display, image_text_8,
	               drawable,
	               gc->gc,
	               x,
	               y,
	               string);
}

int XQueryTextExtents(Display *display,
                      XID font,
                      char *string,
                      int nchars,
                      int *direction,
                      int *ascent,
                      int *descent,
                      XCharStruct *overall)
{
	(void)display;
	(void)font;
	(void)string;
	(void)nchars;
	(void)direction;
	(void)ascent;
	(void)descent;
	(void)overall;
	/* XXX */
	return Success;
	/*xcb_query_text_extents_cookie_t cookie;
	cookie = xcb_query_text_extents(display->conn, font, nchars, string);
	xcb_generic_error_t *error = NULL;
	xcb_query_text_extents_reply_t *reply = xcb_query_text_extents_reply(display->conn,
	                                                                     cookie, &error);
	if (error)
	{
		xlib_handle_error(display, error);
		int ret = error->response_type;
		free(error);
		return ret;
	}
	*direction = reply->draw_direction;
	*ascent = reply->font_ascent;
	*descent = reply->font_descent;
	overall->lbearing = reply->overall_left;
	overall->rbearing = reply->overall_right;
	overall->width = reply->overall_width;
	overall->ascent = reply->overall_ascent;
	overall->descent = reply->overall_descent;
	overall->attributes = 0;
	return Success;*/
}

Cursor XCreateGlyphCursor(Display *display,
                          Font source_font,
                          Font mask_font,
                          unsigned source_char,
                          unsigned mask_char,
                          XColor *foreground_color,
                          XColor *background_color)
{
	Cursor cursor = xcb_generate_id(display->conn);
	return XID_REQ(display, cursor, create_glyph_cursor,
	               cursor,
	               source_font,
	               mask_font,
	               source_char,
	               mask_char,
	               foreground_color->red,
	               foreground_color->green,
	               foreground_color->blue,
	               background_color->red,
	               background_color->green,
	               background_color->blue);
}

Cursor XCreateFontCursor(Display *display, unsigned int shape)
{
	return XCreateGlyphCursor(display, display->cursor_font,
	                          display->cursor_font, shape, shape + 1,
	                          &display->black, &display->white);
}

Cursor XCreatePixmapCursor(Display *display,
                           Pixmap source,
                           Pixmap mask,
                           XColor *foreground_color,
                           XColor *background_color,
                           unsigned x,
                           unsigned y)
{
	Cursor cursor = xcb_generate_id(display->conn);
	return XID_REQ(display, cursor, create_cursor,
	               cursor,
	               source,
	               mask,
	               foreground_color->red,
	               foreground_color->green,
	               foreground_color->blue,
	               background_color->red,
	               background_color->green,
	               background_color->blue,
	               x,
	               y);

}

int XRecolorCursor(Display *display,
                   Cursor cursor,
                   XColor *foreground_color,
                   XColor *background_color)
{
	return REQUEST(display, recolor_cursor,
	               cursor,
	               foreground_color->red,
	               foreground_color->green,
	               foreground_color->blue,
	               background_color->red,
	               background_color->green,
	               background_color->blue);
}

int XFreeCursor(Display *display, Cursor cursor)
{
	return REQUEST(display, free_cursor,
	               cursor);
}

Pixmap XCreatePixmap(Display *display,
                     Drawable drawable,
                     unsigned width,
                     unsigned height,
                     unsigned depth)
{
	Pixmap pixmap = xcb_generate_id(display->conn);
	return XID_REQ(display, pixmap, create_pixmap,
	               depth,
	               pixmap,
	               drawable,
	               width,
	               height);
}

int XFreePixmap(Display *display, Pixmap pixmap)
{
	return REQUEST(display, free_pixmap, pixmap);
}

XImage *XCreateImage(Display *display,
                     Visual *visual,
                     unsigned int depth,
                     int format,
                     int offset,
                     char *data,
                     unsigned int width,
                     unsigned int height,
                     int bitmap_pad,
                     int bytes_per_line)
{
	(void)display; /* XXX */
	(void)visual; /* XXX */
	XImage *image = malloc(sizeof(*image));
	if (!image)
		return NULL;
	image->width = width;
	image->height = height;
	image->xoffset = offset;
	image->format = format;
	image->data = data;
	image->byte_order = LSBFirst; /* XXX */
	image->bitmap_unit = 8;
	image->bitmap_bit_order = LSBFirst; /* XXX */
	image->bitmap_pad = bitmap_pad;
	image->depth = depth;
	image->bytes_per_line = bytes_per_line;
	image->bits_per_pixel = bitmap_pad; /* XXX */
	image->red_mask = 0xFF0000; /* XXX */
	image->green_mask = 0xFF00; /* XXX */
	image->blue_mask = 0xFF; /* XXX */
	image->obdata = NULL;
	return image;
}

void XDestroyImage(XImage *image)
{
	free(image->data);
	free(image);
}

int XPutImage(Display *display,
              Drawable drawable,
              GC gc,
              XImage *image,
              int src_x,
              int src_y,
              int dst_x,
              int dst_y,
              unsigned width,
              unsigned height)
{
	(void)src_x; /* XXX */
	(void)src_y; /* XXX */
	XFlushGC(gc);
	return REQUEST(display, put_image,
	               image->format,
	               drawable,
	               gc->gc,
	               width,
	               height,
	               dst_x,
	               dst_y,
	               image->xoffset,
	               image->depth,
	               width * height * image->bits_per_pixel / 8,
	               (uint8_t*)image->data);
}

XImage *XGetImage(Display *display,
                  Drawable drawable,
                  int x,
                  int y,
                  unsigned width,
                  unsigned height,
                  unsigned long plane_mask,
                  int format)
{
	REPLY_REQ(display, get_image,
	          format,
	          drawable,
	          x,
	          y,
	          width,
	          height,
	          plane_mask);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	XImage *image = malloc(sizeof(*image));
	if (!image)
	{
		free(reply);
		return NULL;
	}
	image->width = width;
	image->height = height;
	image->xoffset = 0;
	image->format = format;
	image->data = malloc(width * height * 4);
	if (!image->data)
	{
		free(image);
		free(reply);
		return NULL;
	}
	memcpy(image->data, xcb_get_image_data(reply), width * height * 4);
	image->byte_order = LSBFirst; /* XXX */
	image->bitmap_unit = 8; /* XXX */
	image->bitmap_bit_order = LSBFirst; /* XXX */
	image->bitmap_pad = 32; /* XXX */
	image->depth = reply->depth;
	image->bytes_per_line = width * 4; /* XXX */
	image->bits_per_pixel = 32; /* XXX */
	image->red_mask = 0xFF0000; /* XXX */
	image->green_mask = 0xFF00; /* XXX */
	image->blue_mask = 0xFF; /* XXX */
	image->obdata = NULL;
	free(reply);
	return image;
}

int XCopyArea(Display *display,
              Drawable src,
              Drawable dst,
              GC gc,
              int src_x,
              int src_y,
              unsigned width,
              unsigned height,
              int dst_x,
              int dst_y)
{
	XFlushGC(gc);
	return REQUEST(display, copy_area,
	               src,
	               dst,
	               gc->gc,
	               src_x,
	               src_y,
	               dst_x,
	               dst_y,
	               width,
	               height);
}

int XClearArea(Display *display,
               Window window,
               int x,
               int y,
               unsigned width,
               unsigned height,
               Bool exposures)
{
	return REQUEST(display, clear_area,
	               exposures,
	               window,
	               x,
	               y,
	               width,
	               height);
}

int XClearWindow(Display *display, Window window)
{
	return XClearArea(display, window, 0, 0, 0, 0, False);
}

char **XListFonts(Display *display,
                  char *pattern,
                  int maxnames,
                  int *count)
{
	REPLY_REQ(display, list_fonts,
	          maxnames,
	          strlen(pattern),
	          pattern);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	*count = xcb_list_fonts_names_length(reply);
	char **fonts = malloc(sizeof(*fonts) * (*count + 1));
	if (!fonts)
	{
		free(reply);
		return NULL;
	}
	xcb_str_iterator_t it = xcb_list_fonts_names_iterator(reply);
	for (int i = 0; i < *count; ++i)
	{
		int len = xcb_str_name_length(it.data);
		fonts[i] = malloc(len + 1);
		if (!fonts[i])
		{
			for (int j = 0; j < i; ++j)
				free(fonts[j]);
			free(fonts);
			free(reply);
			return NULL;
		}
		memcpy(fonts[i], xcb_str_name(it.data), len);
		fonts[i][it.data->name_len] = '\0';
		xcb_str_next(&it);
	}
	fonts[*count] = NULL;
	free(reply);
	return fonts;
}

KeySym *XGetKeyboardMapping(Display *display,
                            KeyCode first,
                            int count,
                            int *keysyms_per_keycode)
{
	REPLY_REQ(display, get_keyboard_mapping, first, count);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	int keysyms_count = xcb_get_keyboard_mapping_keysyms_length(reply);
	KeySym *keysyms = malloc(sizeof(*keysyms) * keysyms_count);
	if (!keysyms)
	{
		free(reply);
		return NULL;
	}
	xcb_keysym_t *xcb_syms = xcb_get_keyboard_mapping_keysyms(reply);
	for (int i = 0; i < keysyms_count; ++i)
		keysyms[i] = xcb_syms[i];
	*keysyms_per_keycode = reply->keysyms_per_keycode;
	free(reply);
	return keysyms;
}

int XChangeKeyboardMapping(Display *display,
                           int first_keycode,
                           int keysyms_per_keycode,
                           KeySym *keysyms,
                           int num_codes)
{
	return REQUEST(display, change_keyboard_mapping,
	               num_codes,
	               first_keycode,
	               keysyms_per_keycode,
	               keysyms);
}

void XRefreshKeyboardMapping(XMappingEvent *event)
{
	free(event->display->keysyms);
	event->display->keysyms = XGetKeyboardMapping(event->display,
	                                              event->display->min_keycode,
	                                              event->display->max_keycode - event->display->min_keycode,
	                                              &event->display->keysyms_per_keycode);
}

KeySym XLookupKeysym(XKeyEvent *event, int index)
{
	Display *display = event->display;
	if (event->keycode < (unsigned)display->min_keycode
	 || event->keycode > (unsigned)display->max_keycode)
		return NoSymbol;
	uint8_t code = event->keycode - display->min_keycode;
	uint8_t offset = index ? 2 : 0;
	uint32_t keyoff = display->keysyms_per_keycode * code + offset;
	switch (event->state & (ShiftMask | LockMask))
	{
		case 0:
			return display->keysyms[keyoff];
		case ShiftMask:
			return display->keysyms[keyoff + 1];
		case LockMask:
		{
			uint32_t sym = display->keysyms[keyoff];
			if (sym >= 'a' && sym <= 'z')
				return sym + 'A' - 'a';
			return sym;
		}
		case ShiftMask | LockMask:
		{
			uint32_t sym = display->keysyms[keyoff + 1];
			if (sym >= 'a' && sym <= 'z')
				return sym + 'A' - 'a';
			return sym;
		}
	}
	return NoSymbol;
}

XModifierKeymap *XGetModifierMapping(Display *display)
{
	REPLY_REQ(display, get_modifier_mapping);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	XModifierKeymap *modmap = XNewModifiermap(reply->keycodes_per_modifier);
	if (!modmap)
	{
		free(reply);
		return NULL;
	}
	xcb_keycode_t *keycodes = xcb_get_modifier_mapping_keycodes(reply);
	memcpy(modmap->modifiermap, keycodes, 8 * reply->keycodes_per_modifier);
	free(reply);
	return modmap;
}

int XSetModifierMapping(Display *display, XModifierKeymap *modmap)
{
	REPLY_REQ(display, set_modifier_mapping,
	           modmap->max_keypermod,
	           modmap->modifiermap);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return MappingFailed;
	}
	if (!reply)
		return MappingFailed;
	int ret = reply->status;
	free(reply);
	return ret;
}

XModifierKeymap *XNewModifiermap(int max_keys_per_mod)
{
	if (max_keys_per_mod < 0)
		return NULL;
	XModifierKeymap *modmap = malloc(sizeof(*modmap));
	if (!modmap)
		return NULL;
	modmap->modifiermap = malloc(sizeof(*modmap->modifiermap)
	                           * max_keys_per_mod * 8);
	if (!modmap->modifiermap)
	{
		free(modmap);
		return NULL;
	}
	modmap->max_keypermod = max_keys_per_mod;
	return modmap;
}

void XFreeModifiermap(XModifierKeymap *modmap)
{
	if (!modmap)
		return;
	free(modmap->modifiermap);
	free(modmap);
}

XModifierKeymap *XInsertModifiermapEntry(XModifierKeymap *modmap,
                                         KeyCode keycode,
                                         int modifier)
{
	if (!modmap || modifier < 0 || modifier > 7)
		return modmap;
	(void)keycode;
	/* XXX */
	return modmap;
}

XModifierKeymap *XDeleteModifiermapEntry(XModifierKeymap *modmap,
                                         KeyCode keycode,
                                         int modifier)
{
	if (!modmap || modifier < 0 || modifier > 7)
		return modmap;
	(void)keycode;
	/* XXX */
	return modmap;
}

Pixmap XCreateBitmapFromData(Display *display,
                             Drawable drawable,
                             char *data,
                             unsigned width,
                             unsigned height)
{
	XImage *image = XCreateImage(display, NULL, 1, XYPixmap, 0, data,
	                             width, height, 8, (width + 7) / 8);
	image->bits_per_pixel = 1;
	Pixmap pixmap = XCreatePixmap(display, drawable, width, height, 1);
	GC gc = XCreateGC(display, pixmap, 0, NULL);
	XPutImage(display, pixmap, gc, image, 0, 0, 0, 0, width, height);
	image->data = NULL;
	XDestroyImage(image);
	XFreeGC(display, gc);
	return pixmap;
}

void XFreeStringList(char **list)
{
	if (!list)
		return;
	for (size_t i = 0; list[i]; ++i)
		free(list[i]);
	free(list);
}

void XFlushGC(GC gc)
{
	if (!gc->batch_mask)
		return;
	XChangeGC(gc->display, gc, gc->batch_mask, &gc->batch_values);
	gc->batch_mask = 0;
}

GContext XGContextFromGC(GC gc)
{
	return gc->gc;
}

int XChangeGC(Display *display,
              GC gc,
              unsigned long mask,
              XGCValues *values)
{
	uint32_t v[23];
	gen_gc_values(v, mask, values);
	return REQUEST(display, change_gc,
	               gc->gc,
	               mask,
	               v);
}

int XSetFunction(Display *display, GC gc, int function)
{
	(void)display;
	gc->batch_mask |= GCFunction;
	gc->batch_values.function = function;
	return Success;
}

int XSetPlaneMask(Display *display, GC gc, unsigned long plane_mask)
{
	(void)display;
	gc->batch_mask |= GCPlaneMask;
	gc->batch_values.plane_mask = plane_mask;
	return Success;
}

int XSetForeground(Display *display, GC gc, unsigned long foreground)
{
	(void)display;
	gc->batch_mask |= GCForeground;
	gc->batch_values.foreground = foreground;
	return Success;
}

int XSetBackground(Display *display, GC gc, unsigned long background)
{
	(void)display;
	gc->batch_mask |= GCBackground;
	gc->batch_values.background = background;
	return Success;
}

int XSetLineAttributes(Display *display,
                       GC gc,
                       unsigned int line_width,
                       int line_style,
                       int cap_style,
                       int join_style)
{
	(void)display;
	gc->batch_mask |= GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle;
	gc->batch_values.line_width = line_width;
	gc->batch_values.line_style = line_style;
	gc->batch_values.cap_style = cap_style;
	gc->batch_values.join_style = join_style;
	return Success;
}

int XSetFillStyle(Display *display, GC gc, int fill_style)
{
	(void)display;
	gc->batch_mask |= GCFillStyle;
	gc->batch_values.fill_style = fill_style;
	return Success;
}

int XSetFillRule(Display *display, GC gc, int fill_rule)
{
	(void)display;
	gc->batch_mask |= GCFillRule;
	gc->batch_values.fill_rule = fill_rule;
	return Success;
}

int XSetArcMode(Display *display, GC gc, int arc_mode)
{
	(void)display;
	gc->batch_mask |= GCArcMode;
	gc->batch_values.arc_mode = arc_mode;
	return Success;
}

int XSetTile(Display *display, GC gc, Pixmap tile)
{
	(void)display;
	gc->batch_mask |= GCTile;
	gc->batch_values.tile = tile;
	return Success;
}

int XSetStipple(Display *display, GC gc, Pixmap stipple)
{
	(void)display;
	gc->batch_mask |= GCStipple;
	gc->batch_values.stipple = stipple;
	return Success;
}

int XSetTSOrigin(Display *display, GC gc, int ts_x_origin, int ts_y_origin)
{
	(void)display;
	gc->batch_mask |= GCTileStipXOrigin | GCTileStipYOrigin;
	gc->batch_values.ts_x_origin = ts_x_origin;
	gc->batch_values.ts_y_origin = ts_y_origin;
	return Success;
}

int XSetFont(Display *display, GC gc, Font font)
{
	(void)display;
	gc->batch_mask |= GCFont;
	gc->batch_values.font = font;
	return Success;
}

int XSetClipOrigin(Display *display, GC gc, int clip_x_origin, int clip_y_origin)
{
	(void)display;
	gc->batch_mask |= GCClipXOrigin | GCClipYOrigin;
	gc->batch_values.clip_x_origin = clip_x_origin;
	gc->batch_values.clip_y_origin = clip_y_origin;
	return Success;
}

int XSetClipMask(Display *display, GC gc, Pixmap pixmap)
{
	(void)display;
	gc->batch_mask |= GCClipMask;
	gc->batch_values.clip_mask = pixmap;
	return Success;
}

int XSetClipRectangles(Display *display,
                       GC gc,
                       int clip_x_origin,
                       int clip_y_origin,
                       XRectangle *rectangles,
                       int n,
                       int ordering)
{
	return REQUEST(display, set_clip_rectangles,
	               ordering,
	               gc->gc,
	               clip_x_origin,
	               clip_y_origin,
	               n,
	               (xcb_rectangle_t*)rectangles);
}

int XSetDashes(Display *display, GC gc, int dash_offset, char *dash_list, int n)
{
	return REQUEST(display, set_dashes,
	               gc->gc,
	               dash_offset,
	               n,
	               (uint8_t*)dash_list);
}

int XSetRegion(Display *display, GC gc, Region r)
{
	(void)display;
	(void)gc;
	(void)r;
	/* XXX */
	return Success;
}

int XBell(Display *display, int percent)
{
	return REQUEST(display, bell, percent);
}

Bool XQueryPointer(Display *display,
                   Window win,
                   Window *root,
                   Window *window,
                   int *root_x,
                   int *root_y,
                   int *win_x,
                   int *win_y,
                   unsigned *mask)
{
	REPLY_REQ(display, query_pointer, win);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	if (root)
		*root = reply->root;
	if (window)
		*window = reply->child;
	if (root_x)
		*root_x = reply->root_x;
	if (root_y)
		*root_y = reply->root_y;
	if (win_x)
		*win_x = reply->win_x;
	if (win_y)
		*win_y = reply->win_y;
	if (mask)
		*mask = reply->mask;
	free(reply);
	return True;
}

int XWarpPointer(Display *display,
                 Window src_w,
                 Window dst_w,
                 int src_x,
                 int src_y,
                 unsigned src_width,
                 unsigned src_height,
                 int dst_x,
                 int dst_y)
{
	return REQUEST(display, warp_pointer,
	               src_w,
	               dst_w,
	               src_x,
	               src_y,
	               src_width,
	               src_height,
	               dst_x,
	               dst_y);
}

Bool XGetInputFocus(Display *display, Window *window, int *revert_to)
{
	REPLY_REQ(display, get_input_focus);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	if (window)
		*window = reply->focus;
	if (revert_to)
		*revert_to = reply->revert_to;
	free(reply);
	return True;
}

int XSetInputFocus(Display *display, Window window, int revert_to, Time time)
{
	return REQUEST(display, set_input_focus,
	               revert_to,
	               window,
	               time);
}

int XGrabServer(Display *display)
{
	return REQUEST(display, grab_server);
}

int XUngrabServer(Display *display)
{
	return REQUEST(display, ungrab_server);
}

Status XQueryTree(Display *display,
                  Window window,
                  Window *root,
                  Window *parent,
                  Window **children,
                  unsigned *nchildren)
{
	REPLY_REQ(display, query_tree, window);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	if (root)
		*root = reply->root;
	if (parent)
		*parent = reply->parent;
	if (nchildren)
		*nchildren = reply->children_len;
	if (children)
	{
		*children = malloc(sizeof(**children) * *nchildren);
		if (!*children)
		{
			free(reply);
			return False;
		}
		memcpy(*children, xcb_query_tree_children(reply),
		       sizeof(**children) * *nchildren);
	}
	free(reply);
	return True;
}

int XGrabButton(Display *display,
                unsigned button,
                unsigned modifiers,
                Window grab_window,
                Bool owner_events,
                unsigned event_mask,
                int pointer_mode,
                int keyboard_mode,
                Window confine_to,
                Cursor cursor)
{
	return REQUEST(display, grab_button,
	               owner_events,
	               grab_window,
	               event_mask,
	               pointer_mode,
	               keyboard_mode,
	               confine_to,
	               cursor,
	               button,
	               modifiers);
}

int XUngrabButton(Display *display,
                  unsigned button,
                  unsigned modifiers,
                  Window grab_window)
{
	return REQUEST(display, ungrab_button,
	               button,
	               grab_window,
	               modifiers);
}

int XGrabPointer(Display *display,
                 Window grab_window,
                 Bool owner_events,
                 unsigned event_mask,
                 int pointer_mode,
                 int keyboard_mode,
                 Window confine_to,
                 Cursor cursor,
                 Time time)
{
	REPLY_REQ(display, grab_pointer,
	          owner_events,
	          grab_window,
	          event_mask,
	          pointer_mode,
	          keyboard_mode,
	          confine_to,
	          cursor,
	          time);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return Success;
	}
	if (!reply)
		return Success;
	int ret = reply->status;
	free(reply);
	return ret;
}

int XUngrabPointer(Display *display, Time time)
{
	return REQUEST(display, ungrab_pointer,
	               time);
}

int XSetPointerMapping(Display *display, uint8_t *map, int nmap)
{
	REPLY_REQ(display, set_pointer_mapping,
	          nmap, map);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return Success;
	}
	if (!reply)
		return Success;
	int ret = reply->status;
	free(reply);
	return ret;
}

int XGetPointerMapping(Display *display, uint8_t *map, int nmap)
{
	REPLY_REQ(display, get_pointer_mapping);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return 0;
	}
	if (!reply)
		return 0;
	int ret = reply->map_len;
	if (nmap < 0)
		nmap = 0;
	if (nmap > reply->map_len)
		nmap = reply->map_len;
	memcpy(map, xcb_get_pointer_mapping_map(reply), nmap);
	free(reply);
	return ret;
}

int XChangePointerControl(Display *display,
                          Bool do_accel,
                          Bool do_threshold,
                          int accel_numerator,
                          int accel_denominator,
                          int threshold)
{
	return REQUEST(display, change_pointer_control,
	               accel_numerator,
	               accel_denominator,
	               threshold,
	               do_accel,
	               do_threshold);
}

int XGetPointerControl(Display *display,
                       int *accel_numerator,
                       int *accel_denominator,
                       int *threshold)
{
	REPLY_REQ(display, get_pointer_control);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return 0;
	}
	if (!reply)
		return 0;
	if (accel_numerator)
		*accel_numerator = reply->acceleration_numerator;
	if (accel_denominator)
		*accel_denominator = reply->acceleration_denominator;
	if (threshold)
		*threshold = reply->threshold;
	free(reply);
	return 1;
}

Status XQueryBestSize(Display *display,
                      int class,
                      Drawable drawable,
                      unsigned width,
                      unsigned height,
                      unsigned *best_width,
                      unsigned *best_height)
{
	REPLY_REQ(display, query_best_size,
	          class,
	          drawable,
	          width,
	          height);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return 0;
	}
	if (!reply)
		return 0;
	if (best_width)
		*best_width = reply->width;
	if (best_height)
		*best_height = reply->height;
	free(reply);
	return 1;
}

Status XQueryBestTile(Display *display,
                      Drawable drawable,
                      unsigned width,
                      unsigned height,
                      unsigned *best_width,
                      unsigned *best_height)
{
	return XQueryBestSize(display, TileShape, drawable,
	                      width, height, best_width, best_height);
}

Status XQueryBestStipple(Display *display,
                         Drawable drawable,
                         unsigned width,
                         unsigned height,
                         unsigned *best_width,
                         unsigned *best_height)
{
	return XQueryBestSize(display, StippleShape, drawable,
	                      width, height, best_width, best_height);
}

Status XQueryBestCursor(Display *display,
                        Drawable drawable,
                        unsigned width,
                        unsigned height,
                        unsigned *best_width,
                        unsigned *best_height)
{
	return XQueryBestSize(display, CursorShape, drawable,
	                      width, height, best_width, best_height);
}

int XChangeKeyboardControl(Display *display,
                           unsigned long value_mask,
                           XKeyboardControl *values)
{
	uint32_t v[8];
	size_t n = 0;
	if (value_mask & KBKeyClickPercent)
		v[n++] = values->key_click_percent;
	if (value_mask & KBBellPercent)
		v[n++] = values->bell_percent;
	if (value_mask & KBBellPitch)
		v[n++] = values->bell_pitch;
	if (value_mask & KBBellDuration)
		v[n++] = values->bell_duration;
	if (value_mask & KBLed)
		v[n++] = values->led;
	if (value_mask & KBLedMode)
		v[n++] = values->led_mode;
	if (value_mask & KBKey)
		v[n++] = values->key;
	if (value_mask & KBAutoRepeatMode)
		v[n++] = values->auto_repeat_mode;
	return REQUEST(display, change_keyboard_control,
	               value_mask,
	               v);
}

int XGetKeyboardControl(Display *display, XKeyboardState *values)
{
	REPLY_REQ(display, get_keyboard_control);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return 0;
	}
	if (!reply)
		return 0;
	if (values)
	{
		values->key_click_percent = reply->key_click_percent;
		values->bell_percent = reply->bell_percent;
		values->bell_pitch = reply->bell_pitch;
		values->bell_duration = reply->bell_duration;
		values->led_mask = reply->led_mask;
		values->global_auto_repeat = reply->global_auto_repeat;
		for (uint8_t i = 0; i < 32; ++i)
			values->auto_repeats[i] = reply->auto_repeats[i];
	}
	free(reply);
	return 1;
}

int XAutoRepeatOn(Display *display)
{
	XKeyboardControl values;
	values.auto_repeat_mode = AutoRepeatModeOn;
	return XChangeKeyboardControl(display, KBAutoRepeatMode, &values);
}

int XAutoRepeatOff(Display *display)
{
	XKeyboardControl values;
	values.auto_repeat_mode = AutoRepeatModeOff;
	return XChangeKeyboardControl(display, KBAutoRepeatMode, &values);
}
