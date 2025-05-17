#include <X11/Xlibint.h>

#include <stdlib.h>
#include <stdio.h>

void
xlib_handle_error(Display *display, xcb_generic_error_t *error)
{
	if (display->error_handler)
	{
		XErrorEvent event;
		event.type = error->response_type;
		event.display = display;
		event.resourceid = error->resource_id;
		event.serial = error->sequence;
		event.error_code = error->error_code;
		event.request_code = error->major_code;
		event.minor_code = error->minor_code;
		display->error_handler(display, &event);
		return;
	}
	xlib_print_error(display, error);
}

Status
xlib_handle_request_check(Display *display,
                          xcb_void_cookie_t cookie)
{
	xcb_generic_error_t *error;

	xcb_flush(display->conn);
	error = xcb_request_check(display->conn, cookie);
	if (error)
	{
		if (display->error_handler)
		{
			XErrorEvent event;
			event.type = error->response_type;
			event.display = display;
			event.resourceid = error->resource_id;
			event.serial = error->sequence;
			event.error_code = error->error_code;
			event.request_code = error->major_code;
			event.minor_code = error->minor_code;
			display->error_handler(display, &event);
			return Success;
		}
		xlib_print_error(display, error);
		Status err = error->error_code;
		free(error);
		return err;
	}
	return Success;
}

Status
xlib_handle_request_async(Display *display,
                          xcb_void_cookie_t cookie)
{
	(void)display;
	(void)cookie;
	return Success;
}

XID
xlib_handle_request_check_xid(Display *display,
                              XID xid,
                              xcb_void_cookie_t cookie)
{
	xcb_generic_error_t *error;

	xcb_flush(display->conn);
	error = xcb_request_check(display->conn, cookie);
	if (error)
	{
		if (display->error_handler)
		{
			XErrorEvent event;
			event.type = error->response_type;
			event.display = display;
			event.resourceid = error->resource_id;
			event.serial = error->sequence;
			event.error_code = error->error_code;
			event.request_code = error->major_code;
			event.minor_code = error->minor_code;
			display->error_handler(display, &event);
			return None;
		}
		xlib_print_error(display, error);
		free(error);
		return None;
	}
	return xid;
}

XID
xlib_handle_request_async_xid(Display *display,
                              XID xid,
                              xcb_void_cookie_t cookie)
{
	(void)display;
	(void)cookie;
	return xid;
}

void
xlib_print_error(Display *display, xcb_generic_error_t *error)
{
	static const char *error_str[] =
	{
		[Success]           = "Success",
		[BadRequest]        = "BadRequest",
		[BadValue]          = "BadValue",
		[BadWindow]         = "BadWindow",
		[BadPixmap]         = "BadPixmap",
		[BadAtom]           = "BadAtom",
		[BadCursor]         = "BadCursor",
		[BadFont]           = "BadFont",
		[BadMatch]          = "BadMatch",
		[BadDrawable]       = "BadDrawable",
		[BadAccess]         = "BadAccess",
		[BadAlloc]          = "BadAlloc",
		[BadColor]          = "BadColor",
		[BadGC]             = "BadGC",
		[BadIDChoice]       = "BadIDChoice",
		[BadName]           = "BadName",
		[BadLength]         = "BadLength",
		[BadImplementation] = "BadImplementation",
	};
	if (error->error_code >= sizeof(error_str) / sizeof(*error_str))
		fprintf(stderr, "X Error of failed request: %u\n", error->error_code);
	else
		fprintf(stderr, "X Error of failed request: %s\n", error_str[error->error_code]);
	fprintf(stderr, "  Major opcode of failed request: %u\n", error->major_code);
	fprintf(stderr, "  Serial number of failed request: %u\n", error->sequence);
	fprintf(stderr, "  Current serial number in output stream: %lu\n", XNextRequest(display));
}

void
xlib_copy_event(Display *display, XEvent *xe, xcb_generic_event_t *e)
{
	xe->xany.type = e->response_type & 0x7F;
	if (!xe->xany.type)
	{
		xcb_generic_error_t *error = (xcb_generic_error_t*)e;
		xe->xerror.display = display;
		xe->xerror.resourceid = error->resource_id;
		xe->xerror.serial = error->sequence;
		xe->xerror.error_code = error->error_code;
		xe->xerror.request_code = error->major_code;
		xe->xerror.minor_code = 0; /* XXX */
		return;
	}
	xe->xany.serial = e->sequence;
	xe->xany.send_event = e->response_type >> 7;
	xe->xany.display = display;
	switch (xe->xany.type)
	{
		case XCB_KEY_PRESS:
		case XCB_KEY_RELEASE:
		{
			xcb_key_press_event_t *key = (xcb_key_press_event_t*)e;
			xe->xkey.window = key->event;
			xe->xkey.root = key->root;
			xe->xkey.subwindow = key->child;
			xe->xkey.time = key->time;
			xe->xkey.x = key->event_x;
			xe->xkey.y = key->event_y;
			xe->xkey.x_root = key->root_x;
			xe->xkey.y_root = key->root_y;
			xe->xkey.state = key->state;
			xe->xkey.keycode = key->detail;
			xe->xkey.same_screen = key->same_screen;
			break;
		}
		case XCB_BUTTON_PRESS:
		case XCB_BUTTON_RELEASE:
		{
			xcb_button_press_event_t *button = (xcb_button_press_event_t*)e;
			xe->xbutton.window = button->event;
			xe->xbutton.root = button->root;
			xe->xbutton.subwindow = button->child;
			xe->xbutton.time = button->time;
			xe->xbutton.x = button->event_x;
			xe->xbutton.y = button->event_y;
			xe->xbutton.x_root = button->root_x;
			xe->xbutton.y_root = button->root_y;
			xe->xbutton.state = button->state;
			xe->xbutton.button = button->detail;
			xe->xbutton.same_screen = button->same_screen;
			break;
		}
		case XCB_MOTION_NOTIFY:
		{
			xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t*)e;
			xe->xmotion.window = motion->event;
			xe->xmotion.root = motion->root;
			xe->xmotion.subwindow = motion->child;
			xe->xmotion.time = motion->time;
			xe->xmotion.x = motion->event_x;
			xe->xmotion.y = motion->event_y;
			xe->xmotion.x_root = motion->root_x;
			xe->xmotion.y_root = motion->root_y;
			xe->xmotion.state = motion->state;
			xe->xmotion.is_hint = motion->detail;
			xe->xmotion.same_screen = motion->same_screen;
			break;
		}
		case XCB_ENTER_NOTIFY:
		case XCB_LEAVE_NOTIFY:
		{
			xcb_enter_notify_event_t *crossing = (xcb_enter_notify_event_t*)e;
			xe->xcrossing.window = crossing->event;
			xe->xcrossing.root = crossing->root;
			xe->xcrossing.subwindow = crossing->child;
			xe->xcrossing.time = crossing->time;
			xe->xcrossing.x = crossing->event_x;
			xe->xcrossing.y = crossing->event_y;
			xe->xcrossing.x_root = crossing->root_x;
			xe->xcrossing.y_root = crossing->root_y;
			xe->xcrossing.mode = crossing->mode;
			xe->xcrossing.detail = crossing->detail;
			xe->xcrossing.same_screen = (crossing->same_screen_focus >> 1) & 1;
			xe->xcrossing.focus = crossing->same_screen_focus & 1;
			xe->xcrossing.state = crossing->state;
			break;
		}
		case XCB_FOCUS_IN:
		case XCB_FOCUS_OUT:
		{
			xcb_focus_in_event_t *focus = (xcb_focus_in_event_t*)e;
			xe->xfocus.window = focus->event;
			xe->xfocus.mode = focus->mode;
			xe->xfocus.detail = focus->detail;
			break;
		}
		case XCB_KEYMAP_NOTIFY:
		{
			xcb_keymap_notify_event_t *keymap = (xcb_keymap_notify_event_t*)e;
			xe->xkeymap.window = None;
			for (int i = 0; i < 31; ++i)
				xe->xkeymap.key_vector[i] = keymap->keys[i];
			xe->xkeymap.key_vector[31] = 0;
			break;
		}
		case XCB_EXPOSE:
		{
			xcb_expose_event_t *expose = (xcb_expose_event_t*)e;
			xe->xexpose.window = expose->window;
			xe->xexpose.x = expose->x;
			xe->xexpose.y = expose->y;
			xe->xexpose.width = expose->width;
			xe->xexpose.height = expose->height;
			xe->xexpose.count = expose->count;
			break;
		}
		case XCB_GRAPHICS_EXPOSURE:
		{
			xcb_graphics_exposure_event_t *gexpose = (xcb_graphics_exposure_event_t*)e;
			xe->xgraphicsexpose.drawable = gexpose->drawable;
			xe->xgraphicsexpose.x = gexpose->x;
			xe->xgraphicsexpose.y = gexpose->y;
			xe->xgraphicsexpose.width = gexpose->width;
			xe->xgraphicsexpose.height = gexpose->height;
			xe->xgraphicsexpose.count = gexpose->count;
			xe->xgraphicsexpose.major_code = gexpose->major_opcode;
			xe->xgraphicsexpose.minor_code = gexpose->minor_opcode;
			break;
		}
		case XCB_NO_EXPOSURE:
		{
			xcb_no_exposure_event_t *noexposure = (xcb_no_exposure_event_t*)e;
			xe->xnoexpose.drawable = noexposure->drawable;
			xe->xnoexpose.major_code = noexposure->major_opcode;
			xe->xnoexpose.minor_code = noexposure->minor_opcode;
			break;
		}
		case XCB_VISIBILITY_NOTIFY:
		{
			xcb_visibility_notify_event_t *visibility = (xcb_visibility_notify_event_t*)e;
			xe->xvisibility.window = visibility->window;
			xe->xvisibility.state = visibility->state;
			break;
		}
		case XCB_CREATE_NOTIFY:
		{
			xcb_create_notify_event_t *create = (xcb_create_notify_event_t*)e;
			xe->xcreatewindow.parent = create->parent;
			xe->xcreatewindow.window = create->window;
			xe->xcreatewindow.x = create->x;
			xe->xcreatewindow.y = create->y;
			xe->xcreatewindow.width = create->width;
			xe->xcreatewindow.height = create->height;
			xe->xcreatewindow.border_width = create->border_width;
			xe->xcreatewindow.override_redirect = create->override_redirect;
			break;
		}
		case XCB_DESTROY_NOTIFY:
		{
			xcb_destroy_notify_event_t *destroy = (xcb_destroy_notify_event_t*)e;
			xe->xdestroywindow.event = destroy->event;
			xe->xdestroywindow.window = destroy->window;
			break;
		}
		case XCB_UNMAP_NOTIFY:
		{
			xcb_unmap_notify_event_t *unmap = (xcb_unmap_notify_event_t*)e;
			xe->xunmap.event = unmap->event;
			xe->xunmap.window = unmap->window;
			xe->xunmap.from_configure = unmap->from_configure;
			break;
		}
		case XCB_MAP_NOTIFY:
		{
			xcb_map_notify_event_t *map = (xcb_map_notify_event_t*)e;
			xe->xmap.event = map->event;
			xe->xmap.window = map->window;
			xe->xmap.override_redirect =- map->override_redirect;
			break;
		}
		case XCB_MAP_REQUEST:
		{
			xcb_map_request_event_t *mapreq = (xcb_map_request_event_t*)e;
			xe->xmaprequest.parent = mapreq->parent;
			xe->xmaprequest.window = mapreq->window;
			break;
		}
		case XCB_REPARENT_NOTIFY:
		{
			xcb_reparent_notify_event_t *reparent = (xcb_reparent_notify_event_t*)e;
			xe->xreparent.event = reparent->event;
			xe->xreparent.window = reparent->window;
			xe->xreparent.parent = reparent->parent;
			xe->xreparent.x = reparent->x;
			xe->xreparent.y = reparent->y;
			xe->xreparent.override_redirect = reparent->override_redirect;
			break;
		}
		case XCB_CONFIGURE_NOTIFY:
		{
			xcb_configure_notify_event_t *configure = (xcb_configure_notify_event_t*)e;
			xe->xconfigure.event = configure->event;
			xe->xconfigure.window = configure->window;
			xe->xconfigure.x = configure->x;
			xe->xconfigure.y = configure->y;
			xe->xconfigure.width = configure->width;
			xe->xconfigure.height = configure->height;
			xe->xconfigure.border_width = configure->border_width;
			xe->xconfigure.override_redirect = configure->override_redirect;
			break;
		}
		case XCB_CONFIGURE_REQUEST:
		{
			xcb_configure_request_event_t *configure = (xcb_configure_request_event_t*)e;
			xe->xconfigurerequest.parent = configure->parent;
			xe->xconfigurerequest.window = configure->window;
			xe->xconfigurerequest.x = configure->x;
			xe->xconfigurerequest.y = configure->y;
			xe->xconfigurerequest.width = configure->width;
			xe->xconfigurerequest.height = configure->height;
			xe->xconfigurerequest.border_width = configure->border_width;
			xe->xconfigurerequest.above = configure->sibling;
			xe->xconfigurerequest.detail = configure->stack_mode;
			xe->xconfigurerequest.value_mask = configure->value_mask;
			break;
		}
		case XCB_GRAVITY_NOTIFY:
		{
			xcb_gravity_notify_event_t *gravity = (xcb_gravity_notify_event_t*)e;
			xe->xgravity.event = gravity->event;
			xe->xgravity.window = gravity->window;
			xe->xgravity.x = gravity->x;
			xe->xgravity.y = gravity->y;
			break;
		}
		case XCB_RESIZE_REQUEST:
		{
			xcb_resize_request_event_t *resize = (xcb_resize_request_event_t*)e;
			xe->xresizerequest.window = resize->window;
			xe->xresizerequest.width = resize->width;
			xe->xresizerequest.height = resize->height;
			break;
		}
		case XCB_CIRCULATE_NOTIFY:
		case XCB_CIRCULATE_REQUEST:
		{
			xcb_circulate_notify_event_t *circulate = (xcb_circulate_notify_event_t*)e;
			xe->xcirculate.event = circulate->event;
			xe->xcirculate.window = circulate->window;
			xe->xcirculate.place = circulate->place;
			break;
		}
		case XCB_PROPERTY_NOTIFY:
		{
			xcb_property_notify_event_t *property = (xcb_property_notify_event_t*)e;
			xe->xproperty.window = property->window;
			xe->xproperty.atom = property->atom;
			xe->xproperty.time = property->time;
			xe->xproperty.state = property->state;
			break;
		}
		case XCB_SELECTION_CLEAR:
		{
			xcb_selection_clear_event_t *selection = (xcb_selection_clear_event_t*)e;
			xe->xselectionclear.window = selection->owner;
			xe->xselectionclear.selection = selection->selection;
			xe->xselectionclear.time = selection->time;
			break;
		}
		case XCB_SELECTION_REQUEST:
		{
			xcb_selection_request_event_t *selection = (xcb_selection_request_event_t*)e;
			xe->xselectionrequest.owner = selection->owner;
			xe->xselectionrequest.requestor = selection->requestor;
			xe->xselectionrequest.selection = selection->selection;
			xe->xselectionrequest.target = selection->target;
			xe->xselectionrequest.property = selection->property;
			xe->xselectionrequest.time = selection->time;
			break;
		}
		case XCB_SELECTION_NOTIFY:
		{
			xcb_selection_notify_event_t *selection = (xcb_selection_notify_event_t*)e;
			xe->xselection.requestor = selection->requestor;
			xe->xselection.selection = selection->selection;
			xe->xselection.target = selection->target;
			xe->xselection.property = selection->property;
			xe->xselection.time = selection->time;
			break;
		}
		case XCB_COLORMAP_NOTIFY:
		{
			xcb_colormap_notify_event_t *colormap = (xcb_colormap_notify_event_t*)e;
			xe->xcolormap.window = colormap->window;
			xe->xcolormap.colormap = colormap->colormap;
			xe->xcolormap.new = colormap->_new;
			xe->xcolormap.state = colormap->state;
			break;
		}
		case XCB_CLIENT_MESSAGE:
		{
			xcb_client_message_event_t *clientm = (xcb_client_message_event_t*)e;
			xe->xclientmessage.window = clientm->window;
			xe->xclientmessage.message_type = clientm->type;
			xe->xclientmessage.format = clientm->format;
			xe->xclientmessage.data.l[0] = clientm->data.data32[0];
			xe->xclientmessage.data.l[1] = clientm->data.data32[1];
			xe->xclientmessage.data.l[2] = clientm->data.data32[2];
			xe->xclientmessage.data.l[3] = clientm->data.data32[3];
			xe->xclientmessage.data.l[4] = clientm->data.data32[4];
			break;
		}
		case XCB_MAPPING_NOTIFY:
		{
			xcb_mapping_notify_event_t *mapping = (xcb_mapping_notify_event_t*)e;
			xe->xmapping.window = None;
			xe->xmapping.request = mapping->request;
			xe->xmapping.first_keycode = mapping->first_keycode;
			xe->xmapping.count = mapping->count;
			break;
		}
	}
}
