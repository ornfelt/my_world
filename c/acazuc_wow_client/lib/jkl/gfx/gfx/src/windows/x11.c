#include "windows/x11.h"

#include "window_vtable.h"

#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <jks/utf8.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

static const unsigned int cursors[GFX_CURSOR_LAST] =
{
	[GFX_CURSOR_ARROW] = XC_arrow,
	[GFX_CURSOR_CROSS] = XC_crosshair,
	[GFX_CURSOR_HAND] = XC_hand1,
	[GFX_CURSOR_IBEAM] = XC_xterm,
	[GFX_CURSOR_NO] = XC_X_cursor,
	[GFX_CURSOR_SIZEALL] = XC_fleur,
	[GFX_CURSOR_VRESIZE] = XC_sb_v_double_arrow,
	[GFX_CURSOR_HRESIZE] = XC_sb_h_double_arrow,
	[GFX_CURSOR_WAIT] = XC_watch,
};

static void process_event(struct gfx_x11_window *window, XEvent *event, XEvent *next);
static enum gfx_key_code get_key_code(uint32_t x_key_code);
static uint32_t get_mods(uint32_t state);
static void update_cursor(struct gfx_x11_window *window);

bool gfx_x11_create_window(struct gfx_x11_window *window, const char *title, uint32_t width, uint32_t height, XVisualInfo *vi)
{
	window->clipboard = NULL;
	window->root = DefaultRootWindow(window->display);
	Colormap cmap = XCreateColormap(window->display, window->root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | FocusChangeMask | EnterWindowMask | LeaveWindowMask | SelectionNotify | SelectionClear | SelectionRequest | StructureNotifyMask;
	window->window = XCreateWindow(window->display, window->root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	window->atoms[X11_ATOM_DELETE] = XInternAtom(window->display, "WM_DELETE_WINDOW", 0);
	window->atoms[X11_ATOM_ICON] = XInternAtom(window->display, "_NET_WM_ICON", 0);
	window->atoms[X11_ATOM_NAME] = XInternAtom(window->display, "_NET_WM_NAME", 0);
	window->atoms[X11_ATOM_ICON_NAME] = XInternAtom(window->display, "_NET_WM_ICON_NAME", 0);
	window->atoms[X11_ATOM_UTF8] = XInternAtom(window->display, "UTF8_STRING", 0);
	window->atoms[X11_ATOM_PID] = XInternAtom(window->display, "_NET_WM_PID", 0);
	window->atoms[X11_ATOM_WM_PROTOCOLS] = XInternAtom(window->display, "WM_PROTOCOLS", 0);
	window->atoms[X11_ATOM_PING] = XInternAtom(window->display, "_NET_WM_PING", 0);
	window->atoms[X11_ATOM_CLIPBOARD] = XInternAtom(window->display, "CLIPBOARD", 0);
	window->atoms[X11_ATOM_SELECTION] = XInternAtom(window->display, "DUMMY_SELECTION", 0);
	window->atoms[X11_ATOM_INCR] = XInternAtom(window->display, "INCR", 0);
	window->atoms[X11_ATOM_XSEL_DATA] = XInternAtom(window->display, "XSEL_DATA", 0);
	window->atoms[X11_ATOM_TARGETS] = XInternAtom(window->display, "TARGETS", 0);
	XSetWMProtocols(window->display, window->window, window->atoms, 1);
	uint32_t pid = getpid();
	XChangeProperty(window->display, window->window, window->atoms[X11_ATOM_PID], XA_CARDINAL, 32, PropModeReplace, (uint8_t*)&pid, 1);
	gfx_x11_set_title(window, title);
	window->xim = XOpenIM(window->display, NULL, NULL, NULL);
	if (window->xim)
		window->xic = XCreateIC(window->xim, XNClientWindow, window->window, XNFocusWindow, window->window, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);
	else
		window->xic = NULL;
	XFlush(window->display);
	window->blank_cursor = (Cursor)gfx_x11_create_native_cursor(window, GFX_CURSOR_BLANK);
	window->hidden_cursor = false;
	return true;
}

bool gfx_x11_ctr(struct gfx_x11_window *window, struct gfx_window *winref)
{
	window->winref = winref;
	window->display = XOpenDisplay(NULL);
	if (!window->display)
	{
		GFX_ERROR_CALLBACK("failed to open x11 display");
		return false;
	}
	return true;
}

void gfx_x11_dtr(struct gfx_x11_window *window)
{
	if (window->xic)
		XDestroyIC(window->xic);
	if (window->xim)
		XCloseIM(window->xim);
	if (window->blank_cursor)
		XFreeCursor(window->display, window->blank_cursor);
	if (window->display)
		XCloseDisplay(window->display);
	free(window->clipboard);
}

void gfx_x11_show(struct gfx_x11_window *window)
{
	XMapWindow(window->display, window->window);
	XFlush(window->display);
}

void gfx_x11_hide(struct gfx_x11_window *window)
{
	XUnmapWindow(window->display, window->window);
	XFlush(window->display);
}

static int event_predicate(Display *display, XEvent *event, XPointer window)
{
	(void)display;
	return event->type == GenericEvent || event->xany.window == ((struct gfx_x11_window*)window)->window;
}

void gfx_x11_poll_events(struct gfx_x11_window *window)
{
	XEvent queue[256];
	size_t queue_size = 0;
	while (XCheckIfEvent(window->display, &queue[queue_size], event_predicate, (XPointer)window))
	{
		queue_size++;
		if (queue_size == sizeof(queue) / sizeof(*queue))
			break;
	}
	for (size_t i = 0; i < queue_size; ++i)
		process_event(window, &queue[i], i == queue_size -1 ? NULL : &queue[i + 1]);
}

void gfx_x11_wait_events(struct gfx_x11_window *window)
{
	XEvent queue[256];
	size_t queue_size = 0;
	if (XIfEvent(window->display, &queue[queue_size], event_predicate, (XPointer)window))
		queue_size++;
	while (XCheckIfEvent(window->display, &queue[queue_size], event_predicate, (XPointer)window))
	{
		queue_size++;
		if (queue_size == sizeof(queue) / sizeof(*queue))
			break;
	}
	for (size_t i = 0; i < queue_size; ++i)
		process_event(window, &queue[i], i == queue_size -1 ? NULL : &queue[i + 1]);
}

void gfx_x11_grab_cursor(struct gfx_x11_window *window)
{
	unsigned char mask[XIMaskLen(XI_RawMotion)] = {0};
	XISetMask(mask, XI_RawMotion);
	XIEventMask event;
	event.deviceid = XIAllMasterDevices;
	event.mask = mask;
	event.mask_len = sizeof(mask);
	XISelectEvents(window->display, window->root, &event, 1);
	XGrabPointer(window->display, window->window, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, window->root, None, CurrentTime);
	window->hidden_cursor = true;
	update_cursor(window);
	window->winref->grabbed = true;
	window->winref->virtual_x = window->winref->mouse_x;
	window->winref->virtual_y = window->winref->mouse_y;
	window->prev_mouse_x = window->winref->mouse_x;
	window->prev_mouse_y = window->winref->mouse_y;
}

void gfx_x11_ungrab_cursor(struct gfx_x11_window *window)
{
	unsigned char mask[XIMaskLen(XI_RawMotion)] = {0};
	XIEventMask event;
	event.deviceid = XIAllMasterDevices;
	event.mask = mask;
	event.mask_len = sizeof(mask);
	XISelectEvents(window->display, window->root, &event, 1);
	XUngrabPointer(window->display, CurrentTime);
	window->hidden_cursor = false;
	update_cursor(window);
	gfx_x11_set_mouse_position(window, window->prev_mouse_x, window->prev_mouse_y);
	window->winref->grabbed = false;
}

void gfx_x11_set_title(struct gfx_x11_window *window, const char *title)
{
	Xutf8SetWMProperties(window->display, window->window, title, title, NULL, 0, NULL, NULL, NULL);
	size_t length = strlen(title);
	XChangeProperty(window->display, window->window, window->atoms[X11_ATOM_NAME], window->atoms[X11_ATOM_UTF8], 8, PropModeReplace, (const uint8_t*)title, length);
	XChangeProperty(window->display, window->window, window->atoms[X11_ATOM_ICON_NAME], window->atoms[X11_ATOM_UTF8], 8, PropModeReplace, (const uint8_t*)title, length);
	XFlush(window->display);
}

void gfx_x11_set_icon(struct gfx_x11_window *window, const void *data, uint32_t width, uint32_t height)
{
	long *image = GFX_MALLOC((width * height + 2) * sizeof(long));
	if (!image)
		return;
	image[0] = width;
	image[1] = height;
	for (size_t i = 0; i < width * height; ++i)
		image[2 + i] = ((long)((uint8_t*)data)[i * 4 + 0] << 16)
		             | ((long)((uint8_t*)data)[i * 4 + 1] << 8)
		             | ((long)((uint8_t*)data)[i * 4 + 2] << 0)
		             | ((long)((uint8_t*)data)[i * 4 + 3] << 24);
	XChangeProperty(window->display, window->window, window->atoms[X11_ATOM_ICON], XA_CARDINAL, 32, PropModeReplace, (uint8_t*)image, width * height + 2);
	XFlush(window->display);
	GFX_FREE(image);
}

void gfx_x11_resize(struct gfx_x11_window *window, uint32_t width, uint32_t height)
{
	XResizeWindow(window->display, window->window, width, height);
	XFlush(window->display);
}

static int selection_predicate(Display *display, XEvent *event, XPointer window)
{
	(void)display;
	struct gfx_x11_window *win = (struct gfx_x11_window*)window;
	return event->xany.window == win->window && event->type == SelectionNotify;
}

static int selection_incr_predicate(Display *display, XEvent *event, XPointer window)
{
	(void)display;
	struct gfx_x11_window *win = (struct gfx_x11_window*)window;
	return event->xany.window == win->window && event->type == PropertyNotify && event->xproperty.atom == win->atoms[X11_ATOM_XSEL_DATA] && event->xproperty.state == PropertyNewValue;
}

char *gfx_x11_get_clipboard(struct gfx_x11_window *window)
{
	Window owner = XGetSelectionOwner(window->display, window->atoms[X11_ATOM_CLIPBOARD]);
	if (owner == None)
		return strdup("");
	if (owner == window->window)
		return strdup(window->clipboard);
	XConvertSelection(window->display, window->atoms[X11_ATOM_CLIPBOARD], window->atoms[X11_ATOM_UTF8], window->atoms[X11_ATOM_SELECTION], window->window, CurrentTime);
	XEvent event;
	XIfEvent(window->display, &event, selection_predicate, (XPointer)window);
	if (event.xselection.property == None)
		return strdup("");
	Atom type;
	int format;
	unsigned long nitems;
	unsigned long bytesAfter;
	unsigned char *ret;
	XGetWindowProperty(window->display, window->window, window->atoms[X11_ATOM_SELECTION], 0, LONG_MAX, False, AnyPropertyType, &type, &format, &nitems, &bytesAfter, &ret);
	if (type == window->atoms[X11_ATOM_UTF8])
	{
		char *str = strndup((char*)ret, nitems + 1);
		XFree(ret);
		if (str)
			str[nitems] = '\0';
		return str;
	}
	else if (type == window->atoms[X11_ATOM_INCR])
	{
		char *str = GFX_MALLOC(1);
		if (!str)
			return NULL;
		str[0] = '\0';
		unsigned long len = 0;
		while (true)
		{
			XIfEvent(window->display, &event, selection_incr_predicate, (XPointer)window);
			XGetWindowProperty(window->display, window->window, window->atoms[X11_ATOM_SELECTION], 0, LONG_MAX, False, AnyPropertyType, &type, &format, &nitems, &bytesAfter, &ret);
			if (!nitems)
				return str;
			char *tmp = GFX_REALLOC(str, len + nitems);
			if (!tmp)
			{
				GFX_FREE(str);
				return NULL;
			}
			strncat(str, (char*)ret, nitems);
			len += nitems;
			str[nitems] = '\0';
			XFree(ret);
		}
		return str;
	}
	return strdup("");
}

void gfx_x11_set_clipboard(struct gfx_x11_window *window, const char *clipboard)
{
	window->clipboard = strdup(clipboard);
	XSetSelectionOwner(window->display, window->atoms[X11_ATOM_CLIPBOARD], window->window, CurrentTime);
}

gfx_cursor_t gfx_x11_create_native_cursor(struct gfx_x11_window *window, enum gfx_native_cursor native_cursor)
{
	if (native_cursor == GFX_CURSOR_BLANK)
	{
		XcursorImage *image = XcursorImageCreate(16, 16);
		if (!image)
			return NULL;
		image->xhot = 0;
		image->yhot = 0;
		memset(image->pixels, 0, 16 * 16 * 4);
		Cursor cursor = XcursorImageLoadCursor(window->display, image);
		XcursorImageDestroy(image);
		return (gfx_cursor_t)cursor;
	}
	return (gfx_cursor_t)XCreateFontCursor(window->display, cursors[native_cursor]);
}

gfx_cursor_t gfx_x11_create_cursor(struct gfx_x11_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	XcursorImage *image = XcursorImageCreate(width, height);
	if (!image)
		return NULL;
	image->xhot = xhot;
	image->yhot = yhot;
	for (size_t i = 0; i < width * height * 4; i += 4)
	{
		((uint8_t*)image->pixels)[i + 0] = ((uint8_t*)data)[i + 2];
		((uint8_t*)image->pixels)[i + 1] = ((uint8_t*)data)[i + 1];
		((uint8_t*)image->pixels)[i + 2] = ((uint8_t*)data)[i + 0];
		((uint8_t*)image->pixels)[i + 3] = ((uint8_t*)data)[i + 3];
	}
	Cursor cursor = XcursorImageLoadCursor(window->display, image);
	XcursorImageDestroy(image);
	return (gfx_cursor_t)cursor;
}

void gfx_x11_delete_cursor(struct gfx_x11_window *window, gfx_cursor_t cursor)
{
	if (!cursor)
		return;
	XFreeCursor(window->display, (Cursor)cursor);
}

void gfx_x11_set_cursor(struct gfx_x11_window *window, gfx_cursor_t cursor)
{
	window->cursor = (Cursor)cursor;
	update_cursor(window);
}

void gfx_x11_set_mouse_position(struct gfx_x11_window *window, int32_t x, int32_t y)
{
	XWarpPointer(window->display, None, window->window, 0, 0, 0, 0, x, y);
}

static void update_cursor(struct gfx_x11_window *window)
{
	if (window->hidden_cursor)
	{
		XDefineCursor(window->display, window->window, window->blank_cursor);
		return;
	}
	XDefineCursor(window->display, window->window, window->cursor);
}

static void process_event(struct gfx_x11_window *window, XEvent *event, XEvent *next)
{
	switch (event->type)
	{
		case ConfigureNotify:
			if (window->winref->width != event->xconfigure.width || window->winref->height != event->xconfigure.height)
			{
				window->winref->width = event->xconfigure.width;
				window->winref->height = event->xconfigure.height;
				if (window->winref->resize_callback)
				{
					struct gfx_resize_event evt;
					evt.used = false;
					evt.width = window->winref->width;
					evt.height = window->winref->height;
					window->winref->resize_callback(window->winref, &evt);
				}
			}
			if (window->winref->x != event->xconfigure.x || window->winref->y != event->xconfigure.y)
			{
				window->winref->x = event->xconfigure.x;
				window->winref->y = event->xconfigure.y;
				if (window->winref->move_callback)
				{
					struct gfx_move_event evt;
					evt.used = false;
					evt.x = window->winref->x;
					evt.y = window->winref->y;
					window->winref->move_callback(window->winref, &evt);
				}
			}
			break;
		case Expose:
			if (window->winref->expose_callback)
				window->winref->expose_callback(window->winref);
			break;
		case KeyPress:
		{
			KeySym xkey = XLookupKeysym(&event->xkey, 0);
			if (XFilterEvent(event, window->window))
				break;
			enum gfx_key_code key = get_key_code(xkey);
			if (window->winref->char_callback && key != GFX_KEY_BACKSPACE && key != GFX_KEY_ENTER && key != GFX_KEY_KP_ENTER)
			{
				char text[5] = {0};
				Status status;
				int len = Xutf8LookupString(window->xic, &event->xkey, text, sizeof(text), NULL, &status);
				if (len > 0 && (status == XLookupChars || status == XLookupBoth))
				{
					struct gfx_char_event evt;
					evt.used = false;
					char *iter = text;
					if (utf8_decode((const char**)&iter, &evt.codepoint))
					{
						for (size_t i = 0; i < 5; ++i)
							evt.utf8[i] = text[i];
						window->winref->char_callback(window->winref, &evt);
					}
					else
					{
						GFX_ERROR_CALLBACK("invalid event codepoint");
					}
				}
			}
			if (key == GFX_KEY_UNKNOWN)
				break;
			if (gfx_is_key_down(window->winref, key))
			{
				if (window->winref->key_press_callback)
				{
					struct gfx_key_event evt;
					evt.used = false;
					evt.key = key;
					evt.mods = get_mods(event->xkey.state);
					window->winref->key_press_callback(window->winref, &evt);
				}
			}
			else
			{
				window->winref->keys[key / 8] |= 1 << (key % 8);
				if (window->winref->key_down_callback)
				{
					struct gfx_key_event evt;
					evt.used = false;
					evt.key = key;
					evt.mods = get_mods(event->xkey.state);
					window->winref->key_down_callback(window->winref, &evt);
				}
			}
			break;
		}
		case KeyRelease:
		{
			if (next && next->type == KeyPress && next->xkey.time == event->xkey.time)
				break;
			KeySym xkey = XLookupKeysym(&event->xkey, 0);
			if (XFilterEvent(event, window->window))
				break;
			enum gfx_key_code key = get_key_code(xkey);
			if (key == GFX_KEY_UNKNOWN)
				break;
			if (!gfx_is_key_down(window->winref, key))
				break;
			window->winref->keys[key / 8] &= ~(1 << (key % 8));
			if (window->winref->key_up_callback)
			{
				struct gfx_key_event evt;
				evt.used = false;
				evt.key = key;
				evt.mods = get_mods(event->xkey.state);
				window->winref->key_up_callback(window->winref, &evt);
			}
			break;
		}
		case ButtonPress:
		{
			switch (event->xbutton.button)
			{
				case 4:
					if (window->winref->scroll_callback)
					{
						struct gfx_scroll_event evt;
						evt.used = false;
						evt.mouse_x = gfx_get_mouse_x(window->winref);
						evt.mouse_y = gfx_get_mouse_y(window->winref);
						evt.x = 0;
						evt.y = 1;
						window->winref->scroll_callback(window->winref, &evt);
					}
					break;
				case 5:
					if (window->winref->scroll_callback)
					{
						struct gfx_scroll_event evt;
						evt.used = false;
						evt.mouse_x = gfx_get_mouse_x(window->winref);
						evt.mouse_y = gfx_get_mouse_y(window->winref);
						evt.x = 0;
						evt.y = -1;
						window->winref->scroll_callback(window->winref, &evt);
					}
					break;
				case 6:
					if (window->winref->scroll_callback)
					{
						struct gfx_scroll_event evt;
						evt.used = false;
						evt.mouse_x = gfx_get_mouse_x(window->winref);
						evt.mouse_y = gfx_get_mouse_y(window->winref);
						evt.x = -1;
						evt.y = 0;
						window->winref->scroll_callback(window->winref, &evt);
					}
					break;
				case 7:
					if (window->winref->scroll_callback)
					{
						struct gfx_scroll_event evt;
						evt.used = false;
						evt.mouse_x = gfx_get_mouse_x(window->winref);
						evt.mouse_y = gfx_get_mouse_y(window->winref);
						evt.x = 1;
						evt.y = 0;
						window->winref->scroll_callback(window->winref, &evt);
					}
					break;
				default:
				{
					enum gfx_mouse_button button;
					switch (event->xbutton.button)
					{
						case Button1:
							button = GFX_MOUSE_BUTTON_LEFT;
							break;
						case Button2:
							button = GFX_MOUSE_BUTTON_MIDDLE;
							break;
						case Button3:
							button = GFX_MOUSE_BUTTON_RIGHT;
							break;
						default:
							button = (enum gfx_mouse_button)(event->xbutton.button - Button1 - 4 + GFX_MOUSE_BUTTON_LEFT);
							break;
					}
					if (button >= 0 && button < GFX_MOUSE_BUTTON_LAST)
						window->winref->mouse_buttons |= 1 << button;
					if (window->winref->mouse_down_callback)
					{
						struct gfx_mouse_event evt;
						evt.used = false;
						evt.x = gfx_get_mouse_x(window->winref);
						evt.y = gfx_get_mouse_y(window->winref);
						evt.button = button;
						evt.mods = get_mods(event->xbutton.state);
						window->winref->mouse_down_callback(window->winref, &evt);
					}
					break;
				}
			}
			break;
		}
		case ButtonRelease:
		{
			switch (event->xbutton.button)
			{
				case 4:
				case 5:
				case 6:
				case 7:
					break;
				default:
				{
					enum gfx_mouse_button button;
					switch (event->xbutton.button)
					{
						case Button1:
							button = GFX_MOUSE_BUTTON_LEFT;
							break;
						case Button2:
							button = GFX_MOUSE_BUTTON_MIDDLE;
							break;
						case Button3:
							button = GFX_MOUSE_BUTTON_RIGHT;
							break;
						default:
							button = (enum gfx_mouse_button)(event->xbutton.button - Button1 - 4 + GFX_MOUSE_BUTTON_LEFT);
							break;
					}
					if (button >= 0 && button < GFX_MOUSE_BUTTON_LAST)
						window->winref->mouse_buttons &= ~(1 << button);
					if (window->winref->mouse_up_callback)
					{
						struct gfx_mouse_event evt;
						evt.used = false;
						evt.x = gfx_get_mouse_x(window->winref);
						evt.y = gfx_get_mouse_y(window->winref);
						evt.button = button;
						evt.mods = get_mods(event->xbutton.state);
						window->winref->mouse_up_callback(window->winref, &evt);
					}
					break;
				}
			}
			break;
		}
		case MotionNotify:
			if (window->winref->grabbed)
				break;
			window->winref->mouse_x = event->xmotion.x;
			window->winref->mouse_y = event->xmotion.y;
			if (window->winref->mouse_move_callback)
			{
				struct gfx_pointer_event evt;
				evt.used = false;
				evt.x = event->xmotion.x;
				evt.y = event->xmotion.y;
				window->winref->mouse_move_callback(window->winref, &evt);
			}
			break;
		case FocusIn:
			if (window->winref->focus_in_callback)
				window->winref->focus_in_callback(window->winref);
			break;
		case FocusOut:
			if (window->winref->focus_out_callback)
				window->winref->focus_out_callback(window->winref);
			break;
		case EnterNotify:
			if (window->winref->cursor_enter_callback)
				window->winref->cursor_enter_callback(window->winref);
			break;
		case LeaveNotify:
			if (window->winref->cursor_leave_callback)
				window->winref->cursor_leave_callback(window->winref);
			break;
		case ClientMessage:
			if (event->xclient.message_type == window->atoms[X11_ATOM_WM_PROTOCOLS])
			{
				if ((Atom)event->xclient.data.l[0] == window->atoms[X11_ATOM_DELETE])
				{
					window->winref->close_requested = true;
					if (window->winref->close_callback)
						window->winref->close_callback(window->winref);
				}
				else if ((Atom)event->xclient.data.l[0] == window->atoms[X11_ATOM_PING])
				{
					event->xclient.window = window->root;
					XSendEvent(window->display, window->root, 0, SubstructureNotifyMask | SubstructureRedirectMask, event);
					XFlush(window->display);
				}
			}
			break;
		case SelectionClear:
			free(window->clipboard);
			window->clipboard = strdup("");
			break;
		case SelectionRequest:
		{
			XEvent reply;
			reply.type = SelectionNotify;
			reply.xselection.display = event->xselectionrequest.display;
			reply.xselection.requestor = event->xselectionrequest.requestor;
			reply.xselection.selection = event->xselectionrequest.selection;
			reply.xselection.target = event->xselectionrequest.target;
			reply.xselection.time = event->xselectionrequest.time;
			if (event->xselectionrequest.property == None)
			{
				reply.xselection.property = None;
				XSendEvent(window->display, reply.xselection.requestor, False, NoEventMask, &reply);
				break;
			}
			else if (event->xselectionrequest.target == window->atoms[X11_ATOM_UTF8] || event->xselectionrequest.target == XA_STRING)
			{
				XChangeProperty(window->display, event->xselectionrequest.requestor, event->xselectionrequest.property, event->xselectionrequest.target, 8, PropModeReplace, (const unsigned char*)window->clipboard, window->clipboard ? strlen(window->clipboard) : 0);
				reply.xselection.property = event->xselectionrequest.property;
				XSendEvent(window->display, reply.xselection.requestor, False, NoEventMask, &reply);
			}
			else if (event->xselectionrequest.target == window->atoms[X11_ATOM_TARGETS])
			{
				const Atom targets[] = {window->atoms[X11_ATOM_UTF8], XA_STRING, window->atoms[X11_ATOM_TARGETS]};
				XChangeProperty(window->display, event->xselectionrequest.requestor, event->xselectionrequest.property, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&targets, sizeof(targets) / sizeof(*targets));
				reply.xselection.property = event->xselectionrequest.property;
				XSendEvent(window->display, reply.xselection.requestor, False, NoEventMask, &reply);
			}
			else
			{
				reply.xselection.property = None;
				XSendEvent(window->display, reply.xselection.requestor, False, NoEventMask, &reply);
				break;
			}
			break;
		}
		case MapNotify: //XMapWindow
			break;
		case GenericEvent:
			switch (event->xcookie.evtype)
			{
				case XI_RawMotion:
					if (!window->winref->grabbed)
						break;
					if (!XGetEventData(window->display, &event->xcookie))
						break;
					XIRawEvent *raw_evt = (XIRawEvent*)event->xcookie.data;
					if (raw_evt->valuators.mask_len)
					{
						const double *delta = raw_evt->raw_values;
						if (XIMaskIsSet(raw_evt->valuators.mask, 0))
						{
							window->winref->virtual_x += *delta;
							delta++;
						}
						if (XIMaskIsSet(raw_evt->valuators.mask, 1))
							window->winref->virtual_y += *delta;
						if (window->winref->mouse_move_callback)
						{
							struct gfx_pointer_event evt;
							evt.used = false;
							evt.x = window->winref->virtual_x;
							evt.y = window->winref->virtual_y;
							window->winref->mouse_move_callback(window->winref, &evt);
						}
					}
					break;
			}
			XFreeEventData(window->display, &event->xcookie);
			break;
		case ReparentNotify:
			break;
		default:
			GFX_ERROR_CALLBACK("unhandled event: %u", event->type);
			break;
	}
}

static enum gfx_key_code get_key_code(uint32_t x_key_code)
{
	switch (x_key_code)
	{
		case XK_a:            return GFX_KEY_A;
		case XK_b:            return GFX_KEY_B;
		case XK_c:            return GFX_KEY_C;
		case XK_d:            return GFX_KEY_D;
		case XK_e:            return GFX_KEY_E;
		case XK_f:            return GFX_KEY_F;
		case XK_g:            return GFX_KEY_G;
		case XK_h:            return GFX_KEY_H;
		case XK_i:            return GFX_KEY_I;
		case XK_j:            return GFX_KEY_J;
		case XK_k:            return GFX_KEY_K;
		case XK_l:            return GFX_KEY_L;
		case XK_m:            return GFX_KEY_M;
		case XK_n:            return GFX_KEY_N;
		case XK_o:            return GFX_KEY_O;
		case XK_p:            return GFX_KEY_P;
		case XK_q:            return GFX_KEY_Q;
		case XK_r:            return GFX_KEY_R;
		case XK_s:            return GFX_KEY_S;
		case XK_t:            return GFX_KEY_T;
		case XK_u:            return GFX_KEY_U;
		case XK_v:            return GFX_KEY_V;
		case XK_w:            return GFX_KEY_W;
		case XK_x:            return GFX_KEY_X;
		case XK_y:            return GFX_KEY_Y;
		case XK_z:            return GFX_KEY_Z;
		case XK_0:            return GFX_KEY_0;
		case XK_1:            return GFX_KEY_1;
		case XK_2:            return GFX_KEY_2;
		case XK_3:            return GFX_KEY_3;
		case XK_4:            return GFX_KEY_4;
		case XK_5:            return GFX_KEY_5;
		case XK_6:            return GFX_KEY_6;
		case XK_7:            return GFX_KEY_7;
		case XK_8:            return GFX_KEY_8;
		case XK_9:            return GFX_KEY_9;
		case XK_KP_Insert:    return GFX_KEY_KP_0;
		case XK_KP_End:       return GFX_KEY_KP_1;
		case XK_KP_Down:      return GFX_KEY_KP_2;
		case XK_KP_Page_Down: return GFX_KEY_KP_3;
		case XK_KP_Left:      return GFX_KEY_KP_4;
		case XK_KP_Begin:     return GFX_KEY_KP_5;
		case XK_KP_Right:     return GFX_KEY_KP_6;
		case XK_KP_Home:      return GFX_KEY_KP_7;
		case XK_KP_Up:        return GFX_KEY_KP_8;
		case XK_KP_Page_Up:   return GFX_KEY_KP_9;
		case XK_KP_Divide:    return GFX_KEY_KP_DIVIDE;
		case XK_KP_Multiply:  return GFX_KEY_KP_MULTIPLY;
		case XK_KP_Subtract:  return GFX_KEY_KP_SUBTRACT;
		case XK_KP_Add:       return GFX_KEY_KP_ADD;
		case XK_KP_Equal:     return GFX_KEY_KP_EQUAL;
		case XK_KP_Delete:    return GFX_KEY_KP_DECIMAL;
		case XK_KP_Enter:     return GFX_KEY_KP_ENTER;
		case XK_F1:           return GFX_KEY_F1;
		case XK_F2:           return GFX_KEY_F2;
		case XK_F3:           return GFX_KEY_F3;
		case XK_F4:           return GFX_KEY_F4;
		case XK_F5:           return GFX_KEY_F5;
		case XK_F6:           return GFX_KEY_F6;
		case XK_F7:           return GFX_KEY_F7;
		case XK_F8:           return GFX_KEY_F8;
		case XK_F9:           return GFX_KEY_F9;
		case XK_F10:          return GFX_KEY_F10;
		case XK_F11:          return GFX_KEY_F11;
		case XK_F12:          return GFX_KEY_F12;
		case XK_F13:          return GFX_KEY_F13;
		case XK_F14:          return GFX_KEY_F14;
		case XK_F15:          return GFX_KEY_F15;
		case XK_F16:          return GFX_KEY_F16;
		case XK_F17:          return GFX_KEY_F17;
		case XK_F18:          return GFX_KEY_F18;
		case XK_F19:          return GFX_KEY_F19;
		case XK_F20:          return GFX_KEY_F20;
		case XK_F21:          return GFX_KEY_F21;
		case XK_F22:          return GFX_KEY_F22;
		case XK_F23:          return GFX_KEY_F23;
		case XK_F24:          return GFX_KEY_F24;
		case XK_Shift_L:      return GFX_KEY_LSHIFT;
		case XK_Shift_R:      return GFX_KEY_RSHIFT;
		case XK_Control_L:    return GFX_KEY_LCONTROL;
		case XK_Control_R:    return GFX_KEY_RCONTROL;
		case XK_Alt_L:        return GFX_KEY_LALT;
		case XK_Alt_R:        return GFX_KEY_RALT;
		case XK_Super_L:      return GFX_KEY_LSUPER;
		case XK_Super_R:      return GFX_KEY_RSUPER;
		case XK_Left:         return GFX_KEY_LEFT;
		case XK_Right:        return GFX_KEY_RIGHT;
		case XK_Up:           return GFX_KEY_UP;
		case XK_Down:         return GFX_KEY_DOWN;
		case XK_space:        return GFX_KEY_SPACE;
		case XK_BackSpace:    return GFX_KEY_BACKSPACE;
		case XK_Return:       return GFX_KEY_ENTER;
		case XK_Tab:          return GFX_KEY_TAB;
		case XK_Escape:       return GFX_KEY_ESCAPE;
		case XK_Pause:        return GFX_KEY_PAUSE;
		case XK_Delete:       return GFX_KEY_DELETE;
		case XK_Insert:       return GFX_KEY_INSERT;
		case XK_Home:         return GFX_KEY_HOME;
		case XK_Page_Up:      return GFX_KEY_PAGE_UP;
		case XK_Page_Down:    return GFX_KEY_PAGE_DOWN;
		case XK_End:          return GFX_KEY_END;
		case XK_comma:        return GFX_KEY_COMMA;
		case XK_period:       return GFX_KEY_PERIOD;
		case XK_slash:        return GFX_KEY_SLASH;
		case XK_apostrophe:   return GFX_KEY_APOSTROPHE;
		case XK_semicolon:    return GFX_KEY_SEMICOLON;
		case XK_grave:        return GFX_KEY_GRAVE;
		case XK_bracketleft:  return GFX_KEY_LBRACKET;
		case XK_bracketright: return GFX_KEY_RBRACKET;
		case XK_backslash:    return GFX_KEY_BACKSLASH;
		case XK_equal:        return GFX_KEY_EQUAL;
		case XK_minus:        return GFX_KEY_SUBTRACT;
		case XK_Scroll_Lock:  return GFX_KEY_SCROLL_LOCK;
		case XK_Num_Lock:     return GFX_KEY_NUM_LOCK;
		case XK_Caps_Lock:    return GFX_KEY_CAPS_LOCK;
		case XK_Print:        return GFX_KEY_PRINT;
	}
	return GFX_KEY_UNKNOWN;
}

static uint32_t get_mods(uint32_t state)
{
	uint32_t mods = 0;
	if (state & ControlMask)
		mods |= GFX_KEY_MOD_CONTROL;
	if (state & ShiftMask)
		mods |= GFX_KEY_MOD_SHIFT;
	if (state & LockMask)
		mods |= GFX_KEY_MOD_CAPS_LOCK;
	if (state & Mod1Mask)
		mods |= GFX_KEY_MOD_ALT;
	if (state & Mod2Mask)
		mods |= GFX_KEY_MOD_NUM_LOCK;
	if (state & Mod4Mask)
		mods |= GFX_KEY_MOD_SUPER;
	return mods;
}
