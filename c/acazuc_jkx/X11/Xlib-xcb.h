#ifndef X11_XLIB_XCB_H
#define X11_XLIB_XCB_H

#include <X11/Xlib.h>

#include <xcb/xcb.h>

xcb_connection_t *XGetXCBConnection(Display *display);

#endif
