#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <sys/utsname.h>

#include <stdlib.h>
#include <string.h>

XVisualInfo *
XGetVisualInfo(Display *display,
               long vinfo_mask,
               XVisualInfo *vinfo_template,
               int *nitems_return)
{
	xcb_screen_iterator_t screen_it;
	const xcb_setup_t *setup;
	XVisualInfo *vi = NULL;

	*nitems_return = 0;
	setup = xcb_get_setup(display->conn);
	screen_it = xcb_setup_roots_iterator(setup);
	for (size_t i = 0; screen_it.rem; xcb_screen_next(&screen_it), ++i)
	{
		xcb_screen_t *screen = screen_it.data;

		if ((vinfo_mask & VisualScreenMask)
		 && vinfo_template->screen != (int)i)
			continue;
		xcb_depth_iterator_t depth_it = xcb_screen_allowed_depths_iterator(screen);
		for (size_t j = 0; depth_it.rem; xcb_depth_next(&depth_it), ++j)
		{
			xcb_depth_t *depth = depth_it.data;
			if ((vinfo_mask & VisualDepthMask)
			 && depth->depth != vinfo_template->depth)
				continue;
			xcb_visualtype_iterator_t visual_it = xcb_depth_visuals_iterator(depth);
			for (size_t k = 0; visual_it.rem; xcb_visualtype_next(&visual_it), ++k)
			{
				xcb_visualtype_t *visualtype = visual_it.data;
				if ((vinfo_mask & VisualClassMask)
				 && visualtype->_class != vinfo_template->class)
					continue;
				if ((vinfo_mask & VisualRedMaskMask)
				 && visualtype->red_mask != vinfo_template->red_mask)
					continue;
				if ((vinfo_mask & VisualRedMaskMask)
				 && visualtype->green_mask != vinfo_template->green_mask)
					continue;
				if ((vinfo_mask & VisualRedMaskMask)
				 && visualtype->blue_mask != vinfo_template->blue_mask)
					continue;
				if ((vinfo_mask & VisualColormapSizeMask)
				 && visualtype->colormap_entries != vinfo_template->colormap_size)
					continue;
				if ((vinfo_mask & VisualBitsPerRGBMask)
				 && visualtype->bits_per_rgb_value != vinfo_template->bits_per_rgb)
					continue;
				XVisualInfo *tmp = realloc(vi, sizeof(*vi) * (*nitems_return + 1));
				if (!tmp)
				{
					free(vi);
					*nitems_return = 0;
					return NULL;
				}
				vi = tmp;
				tmp = &vi[(*nitems_return)++];
				tmp->visual = (Visual*)visualtype;
				tmp->visualid = visualtype->visual_id;
				tmp->screen = i;
				tmp->depth = depth->depth;
				tmp->class = visualtype->_class;
				tmp->red_mask = visualtype->red_mask;
				tmp->green_mask = visualtype->green_mask;
				tmp->blue_mask = visualtype->blue_mask;
				tmp->colormap_size = visualtype->colormap_entries;
				tmp->bits_per_rgb = visualtype->bits_per_rgb_value;
			}
		}
	}
	return vi;
}

Status
XMatchVisualInfo(Display *display,
                 int screenid,
                 int depthv,
                 int class,
                 XVisualInfo *vinfo)
{
	xcb_screen_iterator_t screen_it;
	const xcb_setup_t *setup;

	setup = xcb_get_setup(display->conn);
	screen_it = xcb_setup_roots_iterator(setup);
	for (size_t i = 0; screen_it.rem; xcb_screen_next(&screen_it), ++i)
	{
		xcb_screen_t *screen = screen_it.data;

		if ((int)i != screenid)
			continue;
		xcb_depth_iterator_t depth_it = xcb_screen_allowed_depths_iterator(screen);
		for (size_t j = 0; depth_it.rem; xcb_depth_next(&depth_it), ++j)
		{
			xcb_depth_t *depth = depth_it.data;
			if (depth->depth != depthv)
				continue;
			xcb_visualtype_iterator_t visual_it = xcb_depth_visuals_iterator(depth);
			for (size_t k = 0; visual_it.rem; xcb_visualtype_next(&visual_it), ++k)
			{
				xcb_visualtype_t *visualtype = visual_it.data;
				if (visualtype->_class != class)
					continue;
				vinfo->visual = (Visual*)visualtype;
				vinfo->visualid = visualtype->visual_id;
				vinfo->screen = i;
				vinfo->depth = depth->depth;
				vinfo->class = visualtype->_class;
				vinfo->red_mask = visualtype->red_mask;
				vinfo->green_mask = visualtype->green_mask;
				vinfo->blue_mask = visualtype->blue_mask;
				vinfo->colormap_size = visualtype->colormap_entries;
				vinfo->bits_per_rgb = visualtype->bits_per_rgb_value;
				return 1;
			}
		}
	}
	return 0;
}

void
XSetStandardProperties(Display *display,
                       Window window,
                       char *window_name,
                       char *icon_name,
                       Pixmap icon_pixmap,
                       char **argv,
                       int argc,
                       XSizeHints *hints)
{
	if (window_name)
	{
		XTextProperty prop;
		prop.value = (uint8_t*)window_name;
		prop.encoding = XA_STRING;
		prop.format = 8;
		prop.nitems = 0;
		XSetWMName(display, window, &prop);
	}
	if (icon_name)
	{
		XTextProperty prop;
		prop.value = (uint8_t*)icon_name;
		prop.encoding = XA_STRING;
		prop.format = 8;
		prop.nitems = 0;
		XSetWMIconName(display, window, &prop);
	}
	if (icon_pixmap)
	{
		XWMHints wmhints;
		wmhints.flags = IconPixmapHint;
		wmhints.icon_pixmap = icon_pixmap;
		XSetWMHints(display, window, &wmhints);
	}
	if (argv)
		XSetCommand(display, window, argv, argc);
	if (hints)
		XSetWMNormalHints(display, window, hints);
}

void
XSetWMProperties(Display *display,
                 Window window,
                 XTextProperty *window_name,
                 XTextProperty *icon_name,
                 char **argv,
                 int argc,
                 XSizeHints *normal_hints,
                 XWMHints *wm_hints,
                 XClassHint *class_hints)
{
	if (window_name)
		XSetWMName(display, window, window_name);
	if (icon_name)
		XSetWMIconName(display, window, icon_name);
	if (argv)
		XSetCommand(display, window, argv, argc);
	if (normal_hints)
		XSetWMNormalHints(display, window, normal_hints);
	if (wm_hints)
		XSetWMHints(display, window, wm_hints);
	if (class_hints)
	{
		if (!class_hints->res_name)
		{
			XClassHint tmp;
			tmp.res_name = getenv("RESOURCE_NAME");
			if (!tmp.res_name && argv)
				tmp.res_name = argv[0];
			tmp.res_class = class_hints->res_class;
			XSetClassHint(display, window, &tmp);
		}
		else
		{
			XSetClassHint(display, window, class_hints);
		}
	}
	{
		struct utsname uts;
		if (!uname(&uts))
		{
			XTextProperty prop;
			prop.value = (uint8_t*)uts.machine;
			prop.encoding = XA_STRING;
			prop.format = 8;
			prop.nitems = 0;
			XSetWMClientMachine(display, window, &prop);
		}
	}
}

void
XSetTextProperty(Display *display,
                 Window window,
                 XTextProperty *prop,
                 Atom property)
{
	XChangeProperty(display,
	                window,
	                property,
	                prop->encoding,
	                prop->format,
	                PropModeReplace,
	                prop->value,
	                prop->nitems);
}

Status
XGetTextProperty(Display *display,
                 Window window,
                 XTextProperty **prop,
                 Atom property)
{
	Atom actual_type;
	int actual_format;
	unsigned long nitems;
	unsigned long bytes_after;
	unsigned char *propv = NULL;

	if (XGetWindowProperty(display,
	                       window,
	                       property,
	                       0,
	                       0,
	                       False,
	                       AnyPropertyType,
	                       &actual_type,
	                       &actual_format,
	                       &nitems,
	                       &bytes_after,
	                       &propv) != Success)
		return False;
	free(propv);
	if (XGetWindowProperty(display,
	                       window,
	                       property,
	                       0,
	                       (bytes_after + 3) / 4,
	                       False,
	                       actual_type,
	                       &actual_type,
	                       &actual_format,
	                       &nitems,
	                       &bytes_after,
	                       &propv) != Success)
		return False;
	*prop = malloc(sizeof(**prop));
	if (!*prop)
	{
		free(propv);
		return False;
	}
	(*prop)->value = propv;
	(*prop)->encoding = actual_type;
	(*prop)->format = actual_format;
	(*prop)->nitems = nitems;
	return True;
}

void
XSetWMName(Display *display, Window window, XTextProperty *text)
{
	XSetTextProperty(display, window, text, XA_WM_NAME);
}

Status
XGetWMName(Display *display, Window window, XTextProperty **text)
{
	return XGetTextProperty(display, window, text, XA_WM_NAME);
}

void
XSetWMIconName(Display *display, Window window, XTextProperty *text)
{
	XSetTextProperty(display, window, text, XA_WM_ICON_NAME);
}

Status
XGetWMIconName(Display *display, Window window, XTextProperty **text)
{
	return XGetTextProperty(display, window, text, XA_WM_ICON_NAME);
}

void
XSetWMClientMachine(Display *display, Window window, XTextProperty *text)
{
	XSetTextProperty(display, window, text, XA_WM_CLIENT_MACHINE);
}

Status
XGetWMClientMachine(Display *display, Window window, XTextProperty **text)
{
	return XGetTextProperty(display, window, text, XA_WM_CLIENT_MACHINE);
}

void
XSetCommand(Display *display, Window window, char **argv, int argc)
{
	XTextProperty *property;
	Status status;

	status = XStringListToTextProperty(argv, argc, &property);
	if (status)
		return;
	XSetTextProperty(display, window, property, XA_WM_COMMAND);
	free(property->value);
	free(property);
}

void
XSetWMNormalHints(Display *display, Window window, XSizeHints *hints)
{
	XChangeProperty(display,
	                window,
	                XA_WM_NORMAL_HINTS,
	                XA_WM_SIZE_HINTS,
	                32,
	                PropModeReplace,
	                (uint8_t*)hints,
	                18);
}

void
XSetWMHints(Display *display, Window window, XWMHints *hints)
{
	XChangeProperty(display,
	                window,
	                XA_WM_HINTS,
	                XA_WM_HINTS,
	                32,
	                PropModeReplace,
	                (uint8_t*)hints,
	                9);
}

void
XSetClassHint(Display *display, Window window, XClassHint *hints)
{
	char *data;
	size_t name_len = hints->res_name ? strlen(hints->res_name) : 0;
	size_t class_len = hints->res_class ? strlen(hints->res_class) : 0;
	size_t len = name_len + class_len + 2;

	data = malloc(len);
	if (!data)
		return;
	if (hints->res_name)
		memcpy(data, hints->res_name, name_len + 1);
	else
		data[0] = '\0';
	if (hints->res_class)
		memcpy(data + name_len + 1, hints->res_class, class_len + 1);
	else
		data[name_len + 1] = '\0';
	XChangeProperty(display,
	                window,
	                XA_WM_CLASS,
	                XA_STRING,
	                8,
	                PropModeReplace,
	                (uint8_t*)data,
	                len);
	free(data);
}

void
XStoreName(Display *display, Window window, char *window_name)
{
	XTextProperty prop;

	prop.value = (uint8_t*)window_name;
	prop.encoding = XA_STRING;
	prop.format = 8;
	prop.nitems = 0;
	XSetWMName(display, window, &prop);
}

XClassHint *
XAllocClassHint(void)
{
	return calloc(sizeof(XClassHint), 1);
}

XWMHints *
XAllocWMHints(void)
{
	return calloc(sizeof(XWMHints), 1);
}

XSizeHints *
XAllocSizeHints(void)
{
	return calloc(sizeof(XSizeHints), 1);
}

Status
XStringListToTextProperty(char **list, int count, XTextProperty **prop)
{
	XTextProperty *p;
	size_t length;
	size_t it;

	p = malloc(sizeof(*p));
	if (!p)
		return BadAlloc;
	length = 0;
	for (int i = 0; i < count; ++i)
		length += strlen(list[i]) + 1;
	p->value = malloc(length);
	if (!p->value)
	{
		free(p);
		return BadAlloc;
	}
	it = 0;
	for (int i = 0; i < count; ++i)
	{
		size_t len = strlen(list[i]) + 1;
		memcpy(p->value + it, list[i], len);
		it += len;
	}
	p->value[length] = '\0';
	p->encoding = XA_STRING;
	p->format = 8;
	p->nitems = length;
	*prop = p;
	return Success;
}
