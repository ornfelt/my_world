#ifndef X11_XUTIL_H
#define X11_XUTIL_H

#include <X11/Xlib.h>

#define VisualNoMask            0
#define VisualIDMask           (1 << 0)
#define VisualScreenMask       (1 << 1)
#define VisualDepthMask        (1 << 2)
#define VisualClassMask        (1 << 3)
#define VisualRedMaskMask      (1 << 4)
#define VisualGreenMaskMask    (1 << 5)
#define VisualBlueMaskMask     (1 << 6)
#define VisualColormapSizeMask (1 << 7)
#define VisualBitsPerRGBMask   (1 << 8)
#define VisualAllMask           0x1FF

typedef struct
{
	Visual *visual;
	VisualID visualid;
	int screen;
	unsigned depth;
	int class;
	unsigned long red_mask;
	unsigned long green_mask;
	unsigned long blue_mask;
	int colormap_size;
	int bits_per_rgb;
} XVisualInfo;

typedef struct
{
	char *res_name;
	char *res_class;
} XClassHint;

typedef struct
{
	uint32_t flags;
	Bool input;
	int32_t initial_state;
	Pixmap icon_pixmap;
	Window icon_window;
	int32_t icon_x;
	int32_t icon_y;
	Pixmap icon_mask;
	XID window_group;
} XWMHints;

typedef struct
{
	uint32_t flags;
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
	int32_t min_width;
	int32_t min_height;
	int32_t max_width;
	int32_t max_height;
	int32_t width_inc;
	int32_t height_inc;
	struct
	{
		int32_t x;
		int32_t y;
	} min_aspect, max_aspect;
	int32_t base_width;
	int32_t base_height;
	int32_t win_gravity;
} XSizeHints;

typedef struct
{
	uint8_t *value;
	Atom encoding;
	int32_t format;
	uint32_t nitems;
} XTextProperty;

XVisualInfo *XGetVisualInfo(Display *display, long vinfo_mask,
                            XVisualInfo *vinfo_template, int *nitems_return);

Status XMatchVisualInfo(Display *display, int screen, int depth, int class,
                        XVisualInfo *vinfo);

void XSetStandardProperties(Display *display, Window window, char *window_name,
                            char *icon_name, Pixmap icon_pixmap, char **argv,
                            int argc, XSizeHints *hints);
void XSetWMProperties(Display *display, Window window,
                      XTextProperty *window_name, XTextProperty *icon_name,
                      char **argv, int argc, XSizeHints *normal_hints,
                      XWMHints *wm_hints, XClassHint *class_hints);
void XSetTextProperty(Display *display, Window window, XTextProperty *prop,
                      Atom property);
Status XGetTextProperty(Display *display, Window window, XTextProperty **prop,
                        Atom property);
void XSetWMName(Display *display, Window window, XTextProperty *text);
Status XGetWMName(Display *display, Window window, XTextProperty **text);
void XSetWMIconName(Display *display, Window window, XTextProperty *text);
Status XGetWMIconName(Display *display, Window window, XTextProperty **text);
void XSetWMClientMachine(Display *display, Window window, XTextProperty *text);
Status XGetWMClientMachine(Display *display, Window window, XTextProperty **text);
void XSetCommand(Display *display, Window window, char **argv, int argc);
void XSetWMNormalHints(Display *display, Window window, XSizeHints *hints);
void XSetWMHints(Display *display, Window window, XWMHints *hints);
void XSetClassHint(Display *display, Window window, XClassHint *hints);
void XStoreName(Display *display, Window window, char *window_name);
XClassHint *XAllocClassHint(void);
XWMHints *XAllocWMHints(void);
XSizeHints *XAllocSizeHints(void);
Status XStringListToTextProperty(char **list, int count, XTextProperty **prop);

#endif
