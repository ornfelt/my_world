#ifndef X11_XLIB_H
#define X11_XLIB_H

#include "X.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define False 0
#define True  1

#define AllPlanes ((unsigned long)-1)

#define BlackPixel(display, screen) XBlackPixel(display, screen)
#define WhitePixel(display, screen) XWhitePixel(display, screen)
#define ConnectionNumber(display) XConnectionNumber(display)
#define DefaultColormap(display, screen) XDefaultColormap(display, screen)
#define DefaultDepth(display, screen) XDefaultDepth(display, screen)
#define DefaultGC(display, screen) XDefaultGC(display, screen)
#define DefaultRootWindow(display) XDefaultRootWindow(display)
#define DefaultScreenOfDisplay(display) XDefaultScreenOfDisplay(display)
#define DefaultScreen(display) XDefaultScreen(display)
#define DefaultVisual(display, screen) XDefaultVisual(display, screen)
#define DisplayCells(display, screen) XDisplayCells(display, screen)
#define DisplayPlanes(display, screen) XDisplayPlanes(display, screen)
#define DisplayString(display) XDisplayString(display)
#define LastKnownRequestProcessed(display) XLastKnownRequestProccessed(display)
#define NextRequest(display) XNextRequest(display)
#define ProtocolVersion(display) XProtocolVersion(display)
#define ProtocolRevision(display) XProtocolRevision(display)
#define QLength(display) XQLength(display)
#define RootWindow(display, screen) XRootWindow(display, screen)
#define ScreenCount(display) XScreenCount(display)
#define ScreenOfDisplay(display, screen) XScreenOfDisplay(display, screen)
#define ServerVendor(display) XServerVendor(display)
#define VendorRelease(display) XVendorRelease(display)
#define ImageByteOrder(display) XImageByteOrder(display)
#define BitmapBitOrder(display) XBitmapBitOrder(display)
#define BitmapPad(display) XBitmapPad(display)
#define BitmapUnit(display) XBitmapUnit(display)
#define DisplayHeight(display, screen) XDisplayHeight(display, screen)
#define DisplayHeightMM(display, screen) XDisplayHeightMM(display, screen)
#define DisplayWidth(display, screen) XDisplayWidth(display, screen)
#define DisplayWidthMM(display, screen) XDisplayWidthMM(display, screen)

#define BlackPixelOfScreen(screen) XBlackPixelOfScreen(screen)
#define WhitePixelOfScreen(screen) XWhitePixelOfScreen(screen)
#define CellsOfScreen(screen) XCellsOfScreen(screen)
#define DefaultColormapOfScreen(screen) XDefaultColormapOfScreen(screen)
#define DefaultDepthOfScreen(screen) XDefaultDepthOfScreen(screen)
#define DefaultGCOfScreen(screen) XDefaultGCOfScreen(screen)
#define DefaultVisualOfScreen(screen) XDefaultVisualOfScreen(screen)
#define DoesBackingStore(screen) XDoesBackingStore(screen)
#define DoesSaveUnders(screen) XDoesSaveUnders(screen)
#define DisplayOfScreen(screen) XDisplayOfScreen(screen)
#define EventMaskOfScreen(screen) XEventMaskOfScreen(screen)
#define HeightOfScreen(screen) XHeightOfScreen(screen)
#define HeightMMOfScreen(screen) XHeightMMOfScreen(screen)
#define MaxCmapsOfScreen(screen) XMaxCmapsOfScreen(screen)
#define MinCmapsOfScreen(screen) XMinCmapsOfScreen(screen)
#define PlanesOfScreen(screen) XPlanesOfScreen(screen)
#define RootWindowOfScreen(screen) XRootWindowOfScreen(screen)
#define WidthOfScreen(screen) XWidthOfScreen(screen)
#define WidthMMOfScreen(screen) XWidthMMOfScreen(screen)

typedef struct _Display Display;
typedef uint32_t Atom;
typedef uint32_t Window;
typedef uint32_t Pixmap;
typedef uint32_t Cursor;
typedef uint32_t Colormap;
typedef uint32_t Time;
typedef uint32_t Window;
typedef uint32_t Drawable;
typedef uint32_t XID;
typedef uint32_t VisualID;
typedef uint32_t GContext;
typedef struct _Visual Visual;
typedef char *XPointer;
typedef int Bool;
typedef int Status;
typedef uint32_t Font;
typedef struct _GC *GC;
typedef struct _Screen Screen;
typedef struct _Region *Region;
typedef uint32_t KeySym;
typedef uint8_t KeyCode;

typedef struct
{
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
} XSegment;

typedef struct
{
	int16_t x;
	int16_t y;
} XPoint;

typedef struct
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
} XRectangle;

typedef struct
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	int16_t angle1;
	int16_t angle2;
} XArc;

typedef struct
{
	char *chars;
	int32_t nchars;
	int32_t delta;
	Font font;
} XTextItem;

typedef struct
{
	int16_t lbearing;
	int16_t rbearing;
	int16_t width;
	int16_t ascent;
	int16_t descent;
	uint16_t attributes;
} XCharStruct;

typedef struct
{
	uint32_t pixel;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	char flags;
	char pad;
} XColor;

typedef struct _XImage
{
	int32_t width;
	int32_t height;
	int32_t xoffset;
	int32_t format;
	char *data;
	int32_t byte_order;
	int32_t bitmap_unit;
	int32_t bitmap_bit_order;
	int32_t bitmap_pad;
	int32_t depth;
	int32_t bytes_per_line;
	int32_t bits_per_pixel;
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	XPointer obdata;
	struct funcs
	{
		struct _XImage *(*create_image)();
		int (*destroy_image)();
		unsigned long (*get_pixel)();
		int (*put_pixel)();
		struct _Ximage *(*sub_image)();
		int (*add_pixel)();
	} f;
} XImage;

typedef struct
{
	Pixmap background_pixmap;
	uint32_t background_pixel;
	Pixmap border_pixmap;
	uint32_t border_pixel;
	int32_t bit_gravity;
	int32_t win_gravity;
	int32_t backing_store;
	uint32_t backing_planes;
	uint32_t backing_pixel;
	Bool save_under;
	uint32_t event_mask;
	uint32_t do_not_propagate_mask;
	Bool override_redirect;
	Colormap colormap;
	Cursor cursor;
} XSetWindowAttributes;

typedef struct
{
	int32_t depth;
	int32_t bits_per_pixel;
	int32_t scanline_pad;
} XPixmapFormatValues;

typedef struct
{
	int max_keypermod;
	KeyCode *modifiermap;
} XModifierKeymap;

typedef struct
{
	int x;
	int y;
	unsigned width;
	unsigned height;
	unsigned border_width;
	unsigned depth;
	Visual *visual;
	Window root;
	int class;
	int bit_gravity;
	int win_gravity;
	int backing_store;
	unsigned long backing_planes;
	unsigned long backing_pixel;
	Bool save_under;
	Colormap colormap;
	Bool map_installed;
	int map_state;
	long all_event_masks;
	long your_event_mask;
	long do_not_propagate_mask;
	Bool override_redirect;
	Screen *screen;
} XWindowAttributes;

typedef struct
{
	int key_click_percent;
	int bell_percent;
	int bell_pitch;
	int bell_duration;
	int led;
	int led_mode;
	int key;
	int auto_repeat_mode;
} XKeyboardControl;

typedef struct
{
	int key_click_percent;
	int bell_percent;
	unsigned bell_pitch;
	unsigned bell_duration;
	unsigned long led_mask;
	int global_auto_repeat;
	char auto_repeats[32];
} XKeyboardState;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
} XAnyEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	Window root;
	Window subwindow;
	Time time;
	int x;
	int y;
	int x_root;
	int y_root;
	unsigned state;
	unsigned keycode;
	Bool same_screen;
} XKeyEvent;

typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	Window root;
	Window subwindow;
	Time time;
	int x;
	int y;
	int x_root;
	int y_root;
	unsigned state;
	unsigned button;
	Bool same_screen;
} XButtonEvent;

typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	Window root;
	Window subwindow;
	Time time;
	int x;
	int y;
	int x_root;
	int y_root;
	unsigned state;
	char is_hint;
	Bool same_screen;
} XMotionEvent;

typedef XMotionEvent XPointerMovedEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	Window root;
	Window subwindow;
	Time time;
	int x;
	int y;
	int x_root;
	int y_root;
	int mode;
	int detail;
	Bool same_screen;
	Bool focus;
	unsigned state;
} XCrossingEvent;

typedef XCrossingEvent XEnterWindowEvent;
typedef XCrossingEvent XLeaveWindowEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	int mode;
	int detail;
} XFocusChangeEvent;

typedef XFocusChangeEvent XFocusInEvent;
typedef XFocusChangeEvent XFocusOutEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	char key_vector[32];
} XKeymapEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	int x;
	int y;
	int width;
	int height;
	int count;
} XExposeEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Drawable drawable;
	int x;
	int y;
	int width;
	int height;
	int count;
	int major_code;
	int minor_code;
} XGraphicsExposeEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Drawable drawable;
	int major_code;
	int minor_code;
} XNoExposeEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	int state;
} XVisibilityEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window parent;
	Window window;
	int x;
	int y;
	int width;
	int height;
	int border_width;
	Bool override_redirect;
} XCreateWindowEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window event;
	Window window;
} XDestroyWindowEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window event;
	Window window;
	Bool from_configure;
} XUnmapEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window event;
	Window window;
	Bool override_redirect;
} XMapEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window parent;
	Window window;
} XMapRequestEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window event;
	Window window;
	Window parent;
	int x;
	int y;
	Bool override_redirect;
} XReparentEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window event;
	Window window;
	int x;
	int y;
	int width;
	int height;
	int border_width;
	Window above;
	Bool override_redirect;
} XConfigureEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window event;
	Window window;
	int x;
	int y;
} XGravityEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	int width;
	int height;
} XResizeRequestEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window parent;
	Window window;
	int x;
	int y;
	int width;
	int height;
	int border_width;
	Window above;
	int detail;
	unsigned long value_mask;
} XConfigureRequestEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window event;
	Window window;
	int place;
} XCirculateEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window parent;
	Window window;
	int place;
} XCirculateRequestEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	Atom atom;
	Time time;
	int state;
} XPropertyEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	Atom selection;
	Time time;
} XSelectionClearEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window owner;
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;
	Time time;
} XSelectionRequestEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;
	Time time;
} XSelectionEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool sned_event;
	Display *display;
	Window window;
	Colormap colormap;
	Bool new;
	int state;
} XColormapEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	Atom message_type;
	int format;
	union
	{
		char b[20];
		short s[10];
		long l[5];
	} data;
} XClientMessageEvent;

typedef struct
{
	int type;
	unsigned long serial;
	Bool send_event;
	Display *display;
	Window window;
	int request;
	int first_keycode;
	int count;
} XMappingEvent;

typedef struct
{
	int type;
	Display *display;
	XID resourceid;
	unsigned long serial;
	unsigned char error_code;
	unsigned char request_code;
	unsigned char minor_code;
} XErrorEvent;

typedef union _XEvent
{
	int type;
	XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclientmessage;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
	long pad[24];
} XEvent;

typedef struct
{
	int x;
	int y;
	int width;
	int height;
	int border_width;
	Window sibling;
	int stack_mode;
} XWindowChanges;

typedef struct
{
	int function;
	unsigned long plane_mask;
	unsigned long foreground;
	unsigned long background;
	int line_width;
	int line_style;
	int cap_style;
	int join_style;
	int fill_style;
	int fill_rule;
	int arc_mode;
	Pixmap tile;
	Pixmap stipple;
	int ts_x_origin;
	int ts_y_origin;
	Font font;
	int subwindow_mode;
	Bool graphics_exposures;
	int clip_x_origin;
	int clip_y_origin;
	Pixmap clip_mask;
	int dash_offset;
	char dashes;
} XGCValues;

typedef int (*XErrorHandler)(Display *display, XErrorEvent *event);

Display *XOpenDisplay(const char *display_name);
void XCloseDisplay(Display *display);
void XFree(void *ptr);

unsigned long XBlackPixel(Display *display, int screen);
unsigned long XWhitePixel(Display *display, int screen);
int XConnectionNumber(Display *display);
Colormap XDefaultColormap(Display *display, int screen);
int XDefaultDepth(Display *display, int screen);
int *XListDepths(Display *display, int screen, int *count);
GC XDefaultGC(Display *display, int screen);
Window XDefaultRootWindow(Display *display);
Screen *XDefaultScreenOfDisplay(Display *display);
int XDefaultScreen(Display *display);
Visual *XDefaultVisual(Display *display, int screen);
int XDisplayCells(Display *display, int screen);
int XDisplayPlanes(Display *display, int screen);
char *XDisplayString(Display *display);
long XMaxRequestSize(Display *display);
long XExtendedMaxRequestSize(Display *display);
unsigned long XLastKnownRequestProcessed(Display *display);
unsigned long XNextRequest(Display *display);
int XProtocolVersion(Display *display);
int XProtocolRevision(Display *display);
int XQLength(Display *display);
Window XRootWindow(Display *display, int screen);
int XScreenCount(Display *display);
Screen *XScreenOfDisplay(Display *display, int screen);
char *XServerVendor(Display *display);
int XVendorRelease(Display *display);
int XImageByteOrder(Display *display);
int XBitmapBitOrder(Display *display);
int XBitmapPad(Display *display);
int XBitmapUnit(Display *display);
int XDisplayHeight(Display *display, int screen);
int XDisplayHeightMM(Display *display, int screen);
int XDisplayWidth(Display *display, int screen);
int XDisplayWidthMM(Display *display, int screen);
int XDisplayKeycodes(Display *display, int *min_keycodes, int *max_keycodes);
VisualID XVisualIDFromVisual(Visual *visual);
XPixmapFormatValues *XListPixmapFormats(Display *display, int *count);
unsigned long XDisplayMotionBufferSize(Display *display);

unsigned long XBlackPixelOfScreen(Screen *screen);
unsigned long XWhitePixelOfScreen(Screen *screen);
int XCellsOfScreen(Screen *screen);
Colormap XDefaultColormapOfScreen(Screen *screen);
int XDefaultDepthOfScreen(Screen *screen);
GC XDefaultGCOfScreen(Screen *screen);
Visual *XDefaultVisualOfScreen(Screen *screen);
int XDoesBackingStore(Screen *screen);
Bool XDoesSaveUnders(Screen *screen);
Display *XDisplayOfScreen(Screen *screen);
int XScreenNumberOfScreen(Screen *screen);
long XEventMaskOfScreen(Screen *screen);
int XHeightOfScreen(Screen *screen);
int XHeightMMOfScreen(Screen *screen);
int XMaxCmapsOfScreen(Screen *screen);
int XMinCmapsOfScreen(Screen *screen);
int XPlanesOfScreen(Screen *screen);
Window XRootWindowOfScreen(Screen *screen);
int XWidthOfScreen(Screen *screen);
int XWidthMMOfScreen(Screen *screen);

int XFlush(Display *display);
int XPending(Display *display);
int XSync(Display *display, Bool discard);
int XSynchronize(Display *display, int onoff);
XErrorHandler XSetErrorHandler(Display *display, XErrorHandler handler);

char **XListExtensions(Display *display, int *nextensions);
Bool XQueryExtension(Display *display, char *name, int *major_opcode,
                     int *first_event, int *first_error);

int XIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(),
             XPointer arg);
Bool XCheckIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(),
                   XPointer arg);
Bool XPeekIfEvent(Display *display, XEvent *event_return, Bool (*predicate)(),
                  XPointer arg);
int XNextEvent(Display *display, XEvent *event_return);
int XPeekEvent(Display *display, XEvent *event_return);
int XPutBackEvent(Display *display, XEvent *event);
Status XSendEvent(Display *display, Window *window, Bool propagate,
                  long event_mask, XEvent *event_send);

Window XCreateWindow(Display *display, Window parent, int x, int y,
                     unsigned width, unsigned height, unsigned border_width,
                     int depth, unsigned _class, Visual *visual,
                     unsigned valuemask, XSetWindowAttributes *attributes);
Window XCreateSimpleWindow(Display *display, Window parent, int x, int y,
                           unsigned width, unsigned height,
                           unsigned border_width, unsigned long border,
                           unsigned long background);
int XDestroyWindow(Display *display, Window window);
int XDestroySubwindows(Display *display, Window window);
int XMapWindow(Display *display, Window window);
int XUnmapWindow(Display *display, Window window);
int XMapSubwindows(Display *display, Window window);
int XUnmapSubwindows(Display *display, Window window);
int XMapRaised(Display *display, Window window);
Atom XInternAtom(Display *display, char *atom_name, Bool only_if_exists);
char *XGetAtomName(Display *display, Atom atom);
Status XGetAtomNames(Display *display, Atom *atoms, int count, char **names);
int XChangeProperty(Display *display, Window window, Atom property, Atom type,
                    int format, int mode, const uint8_t *data, int nelements);
int XGetWindowProperty(Display *display, Window window, Atom property,
                       long long_offset, long long_length, Bool delete,
                       Atom req_type, Atom *actual_type, int *actual_format,
                       unsigned long *nitems, unsigned long *bytes_after,
                       unsigned char **prop);
Atom *XListProperties(Display *display, Window window, int *nprops);
int XDeleteProperty(Display *display, Window window, Atom property);
int XRotateWindowProperties(Display *display, Window window, Atom *properties,
                            int nprops, int npositions);
int XConfigureWindow(Display *display, Window window, unsigned value_mask,
                     XWindowChanges *values);
int XMoveWindow(Display *display, Window window, int x, int y);
int XResizeWindow(Display *display, Window window, unsigned width,
                  unsigned height);
int XMoveResizeWindow(Display *display, Window window, int x, int y,
                      unsigned width, unsigned height);
int XSetWindowBorderWidth(Display *display, Window window, unsigned width);
int XRaiseWindow(Display *display, Window window);
int XLowerWindow(Display *display, Window window);
int XCirculateSubwindows(Display *display, Window window, int direction);
int XCirculateSubwindowsUp(Display *display, Window window);
int XCirculateSubwindowsDown(Display *display, Window window);
Status XGetGeometry(Display *display, Drawable drawable, Window *root,
                    int *x, int *y, unsigned *width, unsigned *height,
                    unsigned *border_width, unsigned *depth);
Status XGetWindowAttributes(Display *display, Window window,
                            XWindowAttributes *attributes);
int XChangeWindowAttributes(Display *display, Window window,
                            unsigned long valuemask,
                            XSetWindowAttributes *attributes);
int XSetWindowBackground(Display *display, Window window,
                         unsigned long background_pixel);
int XSetWindowBackgroundPixmap(Display *display, Window window,
                               Pixmap background_pixmap);
int XSetWindowBorder(Display *display, Window window,
                     unsigned long border_pixel);
int XSetWindowBorderPixmap(Display *display, Window window,
                           Pixmap border_pixmap);
int XSetWindowColormap(Display *display, Window window, Colormap colormap);
int XDefineCursor(Display *display, Window window, Cursor cursor);
int XUndefineCursor(Display *display, Window window);
Status XSelectInput(Display *display, Window window, uint32_t mask);
int XReparentWindow(Display *display, Window window, Window parent,
                    int x, int y);

GC XCreateGC(Display *display, Drawable drawable, unsigned long valuemask,
             XGCValues *values);
int XCopyGC(Display *display, GC src, GC dst, unsigned long valuemask);
int XFreeGC(Display *display, GC gc);

int XDrawPoints(Display *display, Drawable drawable, GC gc, XPoint *points,
                int npoints, int mode);
int XDrawPoint(Display *display, Drawable drawable, GC gc, int x, int y);
int XDrawLines(Display *display, Drawable drawable, GC gc, XPoint *points,
               int npoints, int mode);
int XDrawLine(Display *display, Drawable drawable, GC gc, int x1, int y1,
              int x2, int y2);
int XDrawSegments(Display *display, Drawable drawable, GC gc,
                  XSegment *segments, int nsegments);
int XDrawRectangles(Display *display, Drawable drawable, GC gc,
                    XRectangle *rectangles, int nrectangles);
int XDrawRectangle(Display *display, Drawable drawable, GC gc, int x, int y,
                   unsigned width, unsigned height);
int XDrawArcs(Display *display, Drawable drawable, GC gc, XArc *arcs,
              int narcs);
int XDrawArc(Display *display, Drawable drawable, GC gc, int x, int y,
             unsigned width, unsigned height, int angle1, int angle2);
int XFillPolygon(Display *display, Drawable drawable, GC gc, XPoint *points,
                 int npoints, int shape, int mode);
int XFillRectangles(Display *display, Drawable drawable, GC gc,
                    XRectangle *rectangles, int nrectangles);
int XFillRectangle(Display *display, Drawable drawable, GC gc, int x, int y,
                   unsigned width, unsigned height);
int XFillArcs(Display *display, Drawable drawable, GC gc, XArc *arcs, int narcs);
int XFillArc(Display *display, Drawable drawable, GC gc, int x, int y,
             unsigned width, unsigned height, int angle1, int angle2);

Font XLoadFont(Display *display, char *name);
int XUnloadFont(Display *display, Font font);
int XDrawString(Display *display, Drawable drawable, GC gc, int x, int y,
                char *string, int length);
int XQueryTextExtents(Display *display, XID font, char *string, int nchars,
                      int *direction, int *ascent, int *descent,
                      XCharStruct *overall);
Cursor XCreateGlyphCursor(Display *display, Font source_font, Font mask_font,
                          unsigned source_char, unsigned mask_char,
                          XColor *foreground_color, XColor *background_color);
Cursor XCreatePixmapCursor(Display *display, Pixmap source, Pixmap mask,
                           XColor *foreground_color, XColor *background_color,
                           unsigned x, unsigned y);
int XRecolorCursor(Display *display, Cursor cursor, XColor *foreground_color,
                   XColor *background_color);
int XFreeCursor(Display *display, Cursor cursor);
Pixmap XCreatePixmap(Display *display, Drawable drawable, unsigned width,
                     unsigned height, unsigned depth);
int XFreePixmap(Display *display, Pixmap pixmap);
XImage *XCreateImage(Display *display, Visual *visual, unsigned int depth,
                     int format, int offset, char *data, unsigned int width,
                     unsigned int height, int bitmap_pad, int bytes_per_line);
void XDestroyImage(XImage *image);
int XPutImage(Display *display, Drawable drawable, GC gc, XImage *image,
              int src_x, int src_y, int dst_x, int dst_y, unsigned width,
              unsigned height);
XImage *XGetImage(Display *display, Drawable drawable, int x, int y,
                  unsigned width, unsigned height, unsigned long plane_mask,
                  int format);
int XCopyArea(Display *display, Drawable src, Drawable dst, GC gc,
              int src_x, int src_y, unsigned width, unsigned height,
              int dst_x, int dst_y);
int XClearArea(Display *display, Window window, int x, int y, unsigned width,
               unsigned height, Bool exposures);
int XClearWindow(Display *display, Window window);

char **XListFonts(Display *display, char *pattern, int maxnames, int *count);

KeySym *XGetKeyboardMapping(Display *display, KeyCode first, int count,
                            int *keysyms_per_keycode);
int XChangeKeyboardMapping(Display *display, int first_keycode,
                           int keysyms_per_keycode, KeySym *keysyms,
                           int num_codes);
void XRefreshKeyboardMapping(XMappingEvent *event);
KeySym XLookupKeysym(XKeyEvent *event, int index);
XModifierKeymap *XGetModifierMapping(Display *display);
int XSetModifierMapping(Display *display, XModifierKeymap *modmap);
XModifierKeymap *XNewModifiermap(int max_keys_per_mod);
void XFreeModifiermap(XModifierKeymap *modmap);
XModifierKeymap *XInsertModifiermapEntry(XModifierKeymap *modmap,
                                         KeyCode keycode, int modifier);
XModifierKeymap *XDeleteModifiermapEntry(XModifierKeymap *modmap,
                                         KeyCode keycode, int modifier);

Pixmap XCreateBitmapFromData(Display *display, Drawable drawable, char *data,
                             unsigned width, unsigned height);

void XFreeStringList(char **list);

void XFlushGC(GC gc);
GContext XGContextFromGC(GC gc);
int XChangeGC(Display *display, GC gc, unsigned long mask, XGCValues *values);
int XSetFunction(Display *display, GC gc, int function);
int XSetPlaneMask(Display *display, GC gc, unsigned long plane_mask);
int XSetForeground(Display *display, GC gc, unsigned long foreground);
int XSetBackground(Display *display, GC gc, unsigned long background);
int XSetLineAttributes(Display *display, GC gc, unsigned int line_width,
                        int line_style, int cap_style, int join_style);
int XSetFillStyle(Display *display, GC gc, int fill_style);
int XSetFillRule(Display *display, GC gc, int fill_rule);
int XSetArcMode(Display *display, GC gc, int arc_mode);
int XSetTile(Display *display, GC gc, Pixmap tile);
int XSetStipple(Display *display, GC gc, Pixmap stipple);
int XSetTSOrigin(Display *display, GC gc, int ts_x_origin, int ts_y_origin);
int XSetFont(Display *display, GC gc, Font font);
int XSetClipOrigin(Display *display, GC gc, int clip_x_origin,
                   int clip_y_origin);
int XSetClipMask(Display *display, GC gc, Pixmap pixmap);
int XSetClipRectangles(Display *display, GC gc, int clip_x_origin,
                       int clip_y_origin, XRectangle *rectangles, int n,
                       int ordering);
int XSetDashes(Display *display, GC gc, int dash_offset, char *dash_list,
               int n);
int XSetRegion(Display *display, GC gc, Region r);

int XBell(Display *display, int percent);

Bool XQueryPointer(Display *display, Window win, Window *root, Window *window,
                   int *root_x, int *root_y, int *win_x, int *win_y,
                   unsigned *mask);
int XWarpPointer(Display *display, Window src_w, Window dst_w, int src_x,
                 int src_y, unsigned src_width, unsigned src_height,
                 int dst_x, int dst_y);

Bool XGetInputFocus(Display *display, Window *window, int *revert_to);
int XSetInputFocus(Display *display, Window window, int revert_to, Time time);

int XGrabServer(Display *display);
int XUngrabServer(Display *display);

Status XQueryTree(Display *display, Window window, Window *root,
                  Window *parent, Window **children, unsigned *nchildren);

int XGrabButton(Display *display, unsigned button, unsigned modifiers,
                Window grab_window, Bool owner_events, unsigned event_mask,
                int pointer_mode, int keyboard_mode, Window confine_to,
                Cursor cursor);
int XUngrabButton(Display *display, unsigned button, unsigned modifiers,
                  Window grab_window);

int XGrabPointer(Display *display, Window grab_window, Bool owner_events,
                 unsigned event_mask, int pointer_mode, int keyboard_mode,
                 Window confine_to, Cursor cursor, Time time);
int XUngrabPointer(Display *display, Time time);

int XSetPointerMapping(Display *display, uint8_t *map, int nmap);
int XGetPointerMapping(Display *display, uint8_t *map, int nmap);

int XChangePointerControl(Display *display, Bool do_accel, Bool do_threshold,
                          int accel_numerator, int accel_denominator,
                          int threshold);
int XGetPointerControl(Display *display, int *accel_numerator,
                       int *accel_denominator, int *threshold);

Status XQueryBestSize(Display *display, int class, Drawable drawable,
                      unsigned width, unsigned height,
                      unsigned *best_width, unsigned *best_height);
Status XQueryBestTile(Display *display, Drawable drawable,
                      unsigned width, unsigned height,
                      unsigned *best_width, unsigned *best_height);
Status XQueryBestStipple(Display *display, Drawable drawable,
                         unsigned width, unsigned height,
                         unsigned *best_width, unsigned *best_height);
Status XQueryBestCursor(Display *display, Drawable drawable,
                        unsigned width, unsigned height,
                        unsigned *best_width, unsigned *best_height);

int XChangeKeyboardControl(Display *display, unsigned long value_mask,
                           XKeyboardControl *values);
int XGetKeyboardControl(Display *display, XKeyboardState *values);
int XAutoRepeatOn(Display *display);
int XAutoRepeatOff(Display *display);

#ifdef __cplusplus
}
#endif

#endif
