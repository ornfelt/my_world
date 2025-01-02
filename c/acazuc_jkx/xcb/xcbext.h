#ifndef XCB_XCBEXT_H
#define XCB_XCBEXT_H

typedef struct xcb_extension_t
{
	const char *name;
	int global_id;
} xcb_extension_t;

#endif
