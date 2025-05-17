#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define BOOL_STR(v) ((v) ? "YES" : "NO")

static void
print_any(XAnyEvent *e, const char *name)
{
	printf("%s event, serial %lu, synthetic %s window 0x%x\n",
	       name,
	       e->serial,
	       BOOL_STR(e->send_event),
	       e->window);
}

static void
print_key(XKeyEvent *e)
{
	KeySym sym;

	sym = XLookupKeysym(e, 0);
	printf("    root 0x%x, subw 0x%x, time %u, (%d,%d), root:(%d,%d)\n",
	       e->root,
	       e->subwindow,
	       e->time,
	       e->x,
	       e->y,
	       e->x_root,
	       e->y_root);
	printf("    state 0x%x, keycode %u (keysym 0x%x, %c), same_screen %s\n",
	       e->state,
	       e->keycode,
	       sym,
	       (sym < 0x80 && isprint(sym)) ? sym : ' ',
	       BOOL_STR(e->same_screen));
}

static void
print_button(XButtonEvent *e)
{
	printf("    root 0x%x, subw 0x%x, time %u, (%d,%d), root:(%d,%d)\n",
	       e->root,
	       e->subwindow,
	       e->time,
	       e->x,
	       e->y,
	       e->x_root,
	       e->y_root);
	printf("    state 0x%x, button %u, same_screen %s\n",
	       e->state,
	       e->button,
	       BOOL_STR(e->same_screen));
}

static void
print_motion(XMotionEvent *e)
{
	printf("    root 0x%x, subw 0x%x, time %u, (%d,%d), root:(%d,%d)\n",
	       e->root,
	       e->subwindow,
	       e->time,
	       e->x,
	       e->y,
	       e->x_root,
	       e->y_root);
	printf("    state 0x%x, is_hint %u, same_screen %s\n",
	       e->state,
	       e->is_hint,
	       BOOL_STR(e->same_screen));
}

static const char *
notify_details[] =
{
	[NotifyAncestor]         = "NotifyAncestor",
	[NotifyVirtual]          = "NotifyVirtual",
	[NotifyInferior]         = "NotifyInferior",
	[NotifyNonlinear]        = "NotifyNonlinear",
	[NotifyNonlinearVirtual] = "NotifyNonlinearVirtual",
};

static const char *
notify_modes[] =
{
	[NotifyNormal] = "NotifyNormal",
	[NotifyGrab]   = "NotifyGrab",
	[NotifyUngrab] = "NotifyUngrab",
};

static void
print_crossing(XCrossingEvent *e)
{
	printf("    root 0x%x, subw 0x%x, time %u, (%d,%d), root:(%d,%d)\n",
	       e->root,
	       e->subwindow,
	       e->time,
	       e->x,
	       e->y,
	       e->x_root,
	       e->y_root);
	printf("    mode %s, detail %s, same_screen %s\n",
	       notify_modes[e->mode],
	       notify_details[e->detail],
	       BOOL_STR(e->same_screen));
	printf("    focus %s, state %x\n",
	       BOOL_STR(e->focus),
	       e->state);
}

static void
print_focus(XFocusChangeEvent *e)
{
	printf("    mode %s, detail %s\n",
	       notify_modes[e->mode],
	       notify_details[e->detail]);
}

static void
print_keymap(XKeymapEvent *e)
{
	printf("    keys:  ");
	for (int i = 0; i < 16; ++i)
		printf("%-3u ", e->key_vector[i]);
	printf("\n");
	printf("           ");
	for (int i = 16; i < 32; ++i)
		printf("%-3u ", e->key_vector[i]);
	printf("\n");
}

static void
print_expose(XExposeEvent *e)
{
	printf("    (%d,%d), width %d, height %d, count %d\n",
	       e->x,
	       e->y,
	       e->width,
	       e->height,
	       e->count);
}

static void
print_graphics_expose(XGraphicsExposeEvent *e)
{
	printf("    (%d,%d), width %d, height %d, count %d\n",
	       e->x,
	       e->y,
	       e->width,
	       e->height,
	       e->count);
	printf("    major %d, minor %d\n",
	       e->major_code,
	       e->minor_code);
}

static void
print_no_expose(XNoExposeEvent *e)
{
	printf("    major %d, minor %d\n", e->major_code, e->minor_code);
}

static void
print_visibility(XVisibilityEvent *e)
{
	printf("    state %d\n", e->state);
}

static void
print_create(XCreateWindowEvent *e)
{
	printf("    (%d,%d), width %d, height %d\n",
	       e->x,
	       e->y,
	       e->width,
	       e->height);
	printf("    border_width %d, above 0x%x, override %s\n",
	       e->border_width,
	       e->parent,
	       BOOL_STR(e->override_redirect));
}

static void
print_destroy(XDestroyWindowEvent *e)
{
	printf("    event 0x%x, window 0x%x\n", e->event, e->window);
}

static void
print_unmap(XUnmapEvent *e)
{
	printf("    event 0x%x, window 0x%x, from_configure %s\n",
	       e->event,
	       e->window,
	       BOOL_STR(e->from_configure));
}

static void
print_map(XMapEvent *e)
{
	printf("    event 0x%x, window 0x%x, override %s\n",
	       e->event,
	       e->window,
	       BOOL_STR(e->override_redirect));
}

static void
print_map_request(XMapRequestEvent *e)
{
	printf("    parent 0x%x\n", e->parent);
}

static void
print_reparent(XReparentEvent *e)
{
	printf("    (%d,%d), override %s\n",
	       e->x,
	       e->y,
	       BOOL_STR(e->override_redirect));
}

static void
print_configure(XConfigureEvent *e)
{
	printf("    (%d,%d), width %d, height %d\n",
	       e->x,
	       e->y,
	       e->width,
	       e->height);
	printf("    border_width %d, above 0x%x, override %s\n",
	       e->border_width,
	       e->above,
	       BOOL_STR(e->override_redirect));
}

static const char *
stack_modes[] =
{
	[Above]    = "Above",
	[Below]    = "Below",
	[TopIf]    = "TopIf",
	[BottomIf] = "BottomIf",
	[Opposite] = "Opposite",
};

static void
print_configure_request(XConfigureRequestEvent *e)
{
	printf("    (%d,%d), width %d, height %d\n",
	       e->x,
	       e->y,
	       e->width,
	       e->height);
	printf("    border_width %d, above 0x%x, detail %s, value 0x%lx\n",
	       e->border_width,
	       e->parent,
	       stack_modes[e->detail],
	       e->value_mask);
}

static void
print_gravity(XGravityEvent *e)
{
	printf("    (%d,%d)\n", e->x, e->y);
}

static void
print_resize(XResizeRequestEvent *e)
{
	printf("    width %d, height %d\n", e->width, e->height);
}

static const char *
places[] =
{
	[PlaceOnTop]    = "PlaceOnTop",
	[PlaceOnBottom] = "PlaceOnBottom",
};

static void
print_circulate(XCirculateEvent *e)
{
	printf("    event %x, window %x, place %s\n",
	       e->event,
	       e->window,
	       places[e->place]);
}

static void
print_circulate_request(XCirculateRequestEvent *e)
{
	printf("    parent %x, window %x, place %s\n",
	       e->parent,
	       e->window,
	       places[e->place]);
}

static const char *
property_states[] =
{
	[PropertyNewValue] = "PropertyNewValue",
	[PropertyDelete]  = "PropertyDelete",
};

static void
print_property(XPropertyEvent *e)
{
	char *name;

	name = XGetAtomName(e->display, e->atom);
	printf("    atom 0x%x (%s), time %u, state %s\n",
	       e->atom, name ? name : "",
	       e->time,
	       property_states[e->state]);
	free(name);
}

static void
print_selection_clear(XSelectionClearEvent *e)
{
	printf("    selection 0x%x, time %u\n", e->selection, e->time);
}

static void
print_selection_request(XSelectionRequestEvent *e)
{
	printf("    selection 0x%x, target 0x%x, property 0x%x, time %u\n",
	       e->selection,
	       e->target,
	       e->property,
	       e->time);
}

static void
print_selection(XSelectionEvent *e)
{
	printf("    selection 0x%x, target 0x%x, property 0x%x, time %u\n",
	       e->selection,
	       e->target,
	       e->property,
	       e->time);
}

static const char *
colormap_states[] =
{
	[ColormapUninstalled] = "ColormapUninstalled",
	[ColormapInstalled]   = "ColormapInstalled",
};

static void
print_colormap(XColormapEvent *e)
{
	printf("    colormap 0x%x, new %s, state %s\n",
	       e->colormap,
	       BOOL_STR(e->new),
	       colormap_states[e->state]);
}

static void
print_client(XClientMessageEvent *e)
{
	printf("    type 0x%x, format %d\n", e->type, e->format);
}

static const char *
mapping_requests[] =
{
	[MappingModifier] = "MappingModifier",
	[MappingKeyboard] = "MappingKeyboard",
	[MappingPointer]  = "MappingPointer",
};

static void
print_mapping(XMappingEvent *e)
{
	printf("    request %s, first_keycode 0x%x, count %d\n",
	       mapping_requests[e->request],
	       e->first_keycode,
	       e->count);
}

typedef void (*print_fn_t)(void *e);

static struct
{
	const char *name;
	print_fn_t fn;
} fns[] =
{
#define EV(name, fn) [name] = {#name, (print_fn_t)fn}
	EV(KeyPress, print_key),
	EV(KeyRelease, print_key),
	EV(ButtonPress, print_button),
	EV(ButtonRelease, print_button),
	EV(MotionNotify, print_motion),
	EV(EnterNotify, print_crossing),
	EV(LeaveNotify, print_crossing),
	EV(FocusIn, print_focus),
	EV(FocusOut, print_focus),
	EV(KeymapNotify, print_keymap),
	EV(Expose, print_expose),
	EV(GraphicsExpose, print_graphics_expose),
	EV(NoExpose, print_no_expose),
	EV(VisibilityNotify, print_visibility),
	EV(CreateNotify, print_create),
	EV(DestroyNotify, print_destroy),
	EV(UnmapNotify, print_unmap),
	EV(MapNotify, print_map),
	EV(MapRequest, print_map_request),
	EV(ReparentNotify, print_reparent),
	EV(ConfigureNotify, print_configure),
	EV(ConfigureRequest, print_configure_request),
	EV(GravityNotify, print_gravity),
	EV(ResizeRequest, print_resize),
	EV(CirculateNotify, print_circulate),
	EV(CirculateRequest, print_circulate_request),
	EV(PropertyNotify, print_property),
	EV(SelectionClear, print_selection_clear),
	EV(SelectionRequest, print_selection_request),
	EV(SelectionNotify, print_selection),
	EV(ColormapNotify, print_colormap),
	EV(ClientMessage, print_client),
	EV(MappingNotify, print_mapping),
#undef EV
};

int
main(int argc, char **argv)
{
	XSetWindowAttributes swa;
	XVisualInfo *vi;
	Display *display;
	Window window;
	Window root;
	unsigned mask;
	int nitems;

	(void)argc;
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "%s: failed to open display\n", argv[0]);
		return EXIT_FAILURE;
	}
	vi = XGetVisualInfo(display, 0, NULL, &nitems);
	if (!vi)
	{
		fprintf(stderr, "%s: failed to get vi\n", argv[0]);
		return EXIT_FAILURE;
	}
	root = XRootWindow(display, 0);
	swa.event_mask = KeyPressMask
	               | KeyReleaseMask
	               | ButtonPressMask
	               | ButtonReleaseMask
	               | EnterWindowMask
	               | LeaveWindowMask
	               | PointerMotionMask
	               | Button1MotionMask
	               | Button2MotionMask
	               | Button3MotionMask
	               | Button4MotionMask
	               | Button5MotionMask
	               | ButtonMotionMask
	               | KeymapStateMask
	               | ExposureMask
	               | VisibilityChangeMask
	               | StructureNotifyMask
	               | SubstructureNotifyMask
	               | FocusChangeMask
	               | PropertyChangeMask
	               | ColormapChangeMask
	               | OwnerGrabButtonMask;
	mask = CWEventMask;
	window = XCreateWindow(display,
	                       root,
	                       0,
	                       0,
	                       640,
	                       480,
	                       0,
	                       vi->depth,
	                       InputOutput,
	                       vi->visual,
	                       mask,
	                       &swa);
	if (!window)
	{
		fprintf(stderr, "%s: failed to create window\n", argv[0]);
		return EXIT_FAILURE;
	}
	XChangeProperty(display,
	                window,
	                XA_WM_NAME,
	                XA_STRING,
	                8,
	                PropModeReplace,
	                (uint8_t*)"xev",
	                3);
	XMapWindow(display, window);
	while (1)
	{
		XEvent event;

		if (XNextEvent(display, &event))
			continue;
		if (event.type < 2
		 || (unsigned)event.type >= sizeof(fns) / sizeof(*fns))
		{
			char name[64];
			snprintf(name, sizeof(name), "Unknown (%d)", event.type);
			print_any(&event.xany, name);
		}
		else
		{
			print_any(&event.xany, fns[event.type].name);
			fns[event.type].fn(&event);
		}
		printf("\n");
	}
	return EXIT_SUCCESS;
}
