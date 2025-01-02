#include <X11/extensions/Xrender.h>
#include <X11/Xlibint.h>

#include <xcb/render.h>

#include <stdlib.h>

static XRenderPictFormat *pict_formats;
static uint32_t pict_formats_count;

Bool XRenderQueryExtension(Display *display,
                           int *event_base,
                           int *error_base)
{
	int major_opcode;
	int first_event;
	int first_error;
	Status ret = XQueryExtension(display, "RENDER", &major_opcode, &first_event,
	                             &first_error);
	if (ret != Success)
		return False;
	if (event_base)
		*event_base = first_event;
	if (error_base)
		*error_base = first_error;
	return True;
}

Status XRenderQueryVersion(Display *display,
                           int *major,
                           int *minor)
{
	REPLY_REQ(display, render_query_version,
	          XCB_RENDER_MAJOR_VERSION,
	          XCB_RENDER_MINOR_VERSION);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	if (major)
		*major = reply->major_version;
	if (minor)
		*minor = reply->minor_version;
	free(reply);
	return True;
}

Status XRenderQueryFormats(Display *display)
{
	REPLY_REQ(display, render_query_pict_formats);
	if (error)
	{
		xlib_handle_error(display, error);
		free(error);
		return False;
	}
	if (!reply)
		return False;
	uint32_t formats_count = xcb_render_query_pict_formats_formats_length(reply);
	XRenderPictFormat *formats = malloc(sizeof(*formats) * formats_count);
	if (!formats)
	{
		free(reply);
		return False;
	}
	for (uint32_t i = 0; i < formats_count; ++i)
	{
		XRenderPictFormat *format = &formats[i];
		xcb_render_pictforminfo_t *pfi = &xcb_render_query_pict_formats_formats(reply)[i];
		format->id = pfi->id;
		format->type = pfi->type;
		format->depth = pfi->depth;
		format->direct.red = pfi->direct.red_shift;
		format->direct.redMask = pfi->direct.red_mask;
		format->direct.green = pfi->direct.green_shift;
		format->direct.greenMask = pfi->direct.green_mask;
		format->direct.blue = pfi->direct.blue_shift;
		format->direct.blueMask = pfi->direct.blue_mask;
		format->direct.alpha = pfi->direct.alpha_shift;
		format->direct.alphaMask = pfi->direct.alpha_mask;
		format->colormap = pfi->colormap;
	}
	free(pict_formats);
	pict_formats = formats;
	pict_formats_count = formats_count;
	return True;
}

XRenderPictFormat *XRenderFindFormat(Display *display,
                                     unsigned long mask,
                                     const XRenderPictFormat *templ,
                                     int count)
{
	if (!pict_formats)
	{
		if (!XRenderQueryFormats(display))
			return NULL;
	}
	for (size_t i = 0; i < pict_formats_count; ++i)
	{
		XRenderPictFormat *fmt = &pict_formats[i];
		if ((mask & PictFormatID) && fmt->id != templ->id)
			continue;
		if ((mask & PictFormatType) && fmt->type != templ->type)
			continue;
		if ((mask & PictFormatDepth) && fmt->depth != templ->depth)
			continue;
		if ((mask & PictFormatRed) && fmt->direct.red != templ->direct.red)
			continue;
		if ((mask & PictFormatRedMask) && fmt->direct.redMask != templ->direct.redMask)
			continue;
		if ((mask & PictFormatGreen) && fmt->direct.green != templ->direct.green)
			continue;
		if ((mask & PictFormatGreenMask) && fmt->direct.greenMask != templ->direct.greenMask)
			continue;
		if ((mask & PictFormatBlue) && fmt->direct.blue != templ->direct.blue)
			continue;
		if ((mask & PictFormatBlueMask) && fmt->direct.blueMask != templ->direct.blueMask)
			continue;
		if ((mask & PictFormatAlpha) && fmt->direct.alpha != templ->direct.alpha)
			continue;
		if ((mask & PictFormatAlphaMask) && fmt->direct.alphaMask != templ->direct.alphaMask)
			continue;
		if ((mask & PictFormatColormap) && fmt->colormap != templ->colormap)
			continue;
		if (!count)
			return fmt;
		count--;
	}
}

XRenderPictFormat *XRenderFindVisualFormat(Display *display,
                                           const Visual *visual)
{
	XRenderPictFormat templ;
	unsigned long mask = PictFormatRed
	                   | PictFormatRedMask
	                   | PictFormatGreen
	                   | PictFormatGreenMask
	                   | PictFormatBlue
	                   | PictFormatBlueMask;
	templ.direct.red = visual->visual.bits_per_rgb_value;
	templ.direct.redMask = visual->visual.red_mask;
	templ.direct.green = visual->visual.bits_per_rgb_value;
	templ.direct.greenMask = visual->visual.green_mask;
	templ.direct.blue = visual->visual.bits_per_rgb_value;
	templ.direct.blueMask = visual->visual.blue_mask;
	return XRenderFindFormat(display, mask, &templ, 0);
}

XRenderPictFormat *XRenderFindStandardFormat(Display *display,
                                             int format)
{
	XRenderPictFormat templ;
	templ.type = PictTypeDirect;
	unsigned long mask = PictFormatType
	                   | PictFormatDepth
	                   | PictFormatRedMask
	                   | PictFormatGreenMask
	                   | PictFormatBlueMask
	                   | PictFormatAlphaMask;
	switch (format)
	{
		case PictStandardARGB32:
			templ.depth = 32;
			templ.direct.red = 16;
			templ.direct.redMask = 0xFF;
			templ.direct.green = 8;
			templ.direct.greenMask = 0xFF;
			templ.direct.blue = 0;
			templ.direct.blueMask = 0xFF;
			templ.direct.alpha = 24;
			templ.direct.alphaMask = 0xFF;
			mask |= PictFormatRed
			      | PictFormatGreen
			      | PictFormatBlue
			      | PictFormatAlpha;
			break;
		case PictStandardRGB24:
			templ.depth = 24;
			templ.direct.red = 16;
			templ.direct.redMask = 0xFF;
			templ.direct.green = 8;
			templ.direct.greenMask = 0xFF;
			templ.direct.blue = 0;
			templ.direct.blueMask = 0xFF;
			templ.direct.alpha = 0;
			templ.direct.alphaMask = 0;
			mask |= PictFormatRed
			      | PictFormatGreen
			      | PictFormatBlue;
			break;
		case PictStandardA8:
			templ.depth = 8;
			templ.direct.red = 0;
			templ.direct.redMask = 0;
			templ.direct.green = 0;
			templ.direct.greenMask = 0;
			templ.direct.blue = 0;
			templ.direct.blueMask = 0;
			templ.direct.alpha = 0;
			templ.direct.alphaMask = 0xFF;
			mask |= PictFormatAlpha;
			break;
		case PictStandardA4:
			templ.depth = 4;
			templ.direct.red = 0;
			templ.direct.redMask = 0;
			templ.direct.green = 0;
			templ.direct.greenMask = 0;
			templ.direct.blue = 0;
			templ.direct.blueMask = 0;
			templ.direct.alpha = 0;
			templ.direct.alphaMask = 0xF;
			mask |= PictFormatAlpha;
			break;
		case PictStandardA1:
			templ.depth = 1;
			templ.direct.red = 0;
			templ.direct.redMask = 0;
			templ.direct.green = 0;
			templ.direct.greenMask = 0;
			templ.direct.blue = 0;
			templ.direct.blueMask = 0;
			templ.direct.alpha = 0;
			templ.direct.alphaMask = 0x1;
			mask |= PictFormatAlpha;
			break;
		default:
			return NULL;
	}
	return XRenderFindFormat(display, mask, &templ, 0);
}

Picture XRenderCreatePicture(Display *display,
                             Drawable drawable,
                             const XRenderPictFormat *format,
                             unsigned long valuemask,
                             const XRenderPictureAttributes *attributes)
{
	uint32_t values[13];
	uint32_t nvalues = 0;
	if (valuemask & CPRepeat)
		values[nvalues++] = attributes->repeat;
	if (valuemask & CPAlphaMap)
		values[nvalues++] = attributes->alpha_map;
	if (valuemask & CPAlphaXOrigin)
		values[nvalues++] = attributes->alpha_x_origin;
	if (valuemask & CPAlphaYOrigin)
		values[nvalues++] = attributes->alpha_y_origin;
	if (valuemask & CPClipXOrigin)
		values[nvalues++] = attributes->clip_x_origin;
	if (valuemask & CPClipYOrigin)
		values[nvalues++] = attributes->clip_y_origin;
	if (valuemask & CPClipMask)
		values[nvalues++] = attributes->clip_mask;
	if (valuemask & CPGraphicsExposure)
		values[nvalues++] = attributes->graphics_exposure;
	if (valuemask & CPSubwindowMode)
		values[nvalues++] = attributes->subwindow_mode;
	if (valuemask & CPPolyEdge)
		values[nvalues++] = attributes->poly_edge;
	if (valuemask & CPPolyMode)
		values[nvalues++] = attributes->poly_mode;
	if (valuemask & CPDither)
		values[nvalues++] = attributes->dither;
	if (valuemask & CPComponentAlpha)
		values[nvalues++] = attributes->component_alpha;
	Picture id = xcb_generate_id(display->conn);
	return XID_REQ(display, id, render_create_picture,
	               id,
	               drawable,
	               format->id,
	               valuemask,
	               values);
}

void XRenderFillRectangle(Display *display,
                          int op,
                          Picture dst,
                          const XRenderColor *color,
                          int x,
                          int y,
                          unsigned int width,
                          unsigned int height)
{
	XRectangle rectangle;
	rectangle.x = x;
	rectangle.y = y;
	rectangle.width = width;
	rectangle.height = height;
	XRenderFillRectangles(display, op, dst, color, &rectangle, 1);
}

void XRenderFillRectangles(Display *display,
                           int op,
                           Picture dst,
                           const XRenderColor *color,
                           const XRectangle *rectangles,
                           int nrects)
{
	REQUEST(display, render_fill_rectangles,
	        op,
	        dst,
	        *(xcb_render_color_t*)color,
	        nrects,
	        (xcb_rectangle_t*)rectangles);
}

void XRenderComposite(Display *display,
                      int op,
                      Picture src,
                      Picture mask,
                      Picture dst,
                      int src_x,
                      int src_y,
                      int mask_x,
                      int mask_y,
                      int dst_x,
                      int dst_y,
                      unsigned int width,
                      unsigned int height)
{
	REQUEST(display, render_composite,
	        op,
	        src,
	        mask,
	        dst,
	        src_x,
	        src_y,
	        mask_x,
	        mask_y,
	        dst_x,
	        dst_y,
	        width,
	        height);
}
