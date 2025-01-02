#ifndef X11_EXTENSIONS_XRENDER_H
#define X11_EXTENSIONS_XRENDER_H

#include <X11/Xlib.h>

#define PictFormatID        (1 << 0)
#define PictFormatType      (1 << 1)
#define PictFormatDepth     (1 << 2)
#define PictFormatRed       (1 << 3)
#define PictFormatRedMask   (1 << 4)
#define PictFormatGreen     (1 << 5)
#define PictFormatGreenMask (1 << 6)
#define PictFormatBlue      (1 << 7)
#define PictFormatBlueMask  (1 << 8)
#define PictFormatAlpha     (1 << 9)
#define PictFormatAlphaMask (1 << 10)
#define PictFormatColormap  (1 << 11)

#define PictTypeIndexed 0
#define PictTypeDirect  1

#define PictStandardARGB32 0
#define PictStandardRGB24  1
#define PictStandardA8     2
#define PictStandardA4     3
#define PictStandardA1     4
#define PictStandardNUM    5

#define CPRepeat           (1 << 0)
#define CPAlphaMap         (1 << 1)
#define CPAlphaXOrigin     (1 << 2)
#define CPAlphaYOrigin     (1 << 3)
#define CPClipXOrigin      (1 << 4)
#define CPClipYOrigin      (1 << 5)
#define CPClipMask         (1 << 6)
#define CPGraphicsExposure (1 << 7)
#define CPSubwindowMode    (1 << 8)
#define CPPolyEdge         (1 << 9)
#define CPPolyMode         (1 << 10)
#define CPDither           (1 << 11)
#define CPComponentAlpha   (1 << 12)
#define CPLastBit          12

#define PictOpClear               0x00
#define PictOpSrc                 0x01
#define PictOpDst                 0x02
#define PictOpOver                0x03
#define PictOpOverReverse         0x04
#define PictOpIn                  0x05
#define PictOpInReverse           0x06
#define PictOpOut                 0x07
#define PictOpOutReverse          0x08
#define PictOpAtop                0x09
#define PictOpAtopReverse         0x0A
#define PictOpXor                 0x0B
#define PictOpAdd                 0x0C
#define PictOpSaturate            0x0D
#define PictOpDisjointClear       0x10
#define PictOpDisjointSrc         0x11
#define PictOpDisjointDst         0x12
#define PictOpDisjointOver        0x13
#define PictOpDisjointOverReverse 0x14
#define PictOpDisjointIn          0x15
#define PictOpDisjointInReverse   0x16
#define PictOpDisjointOut         0x17
#define PictOpDisjointOutReverse  0x18
#define PictOpDisjointAtop        0x19
#define PictOpDisjointAtopReverse 0x1A
#define PictOpDisjointXor         0x1B
#define PictOpConjointClear       0x20
#define PictOpConjointSrc         0x21
#define PictOpConjointDst         0x22
#define PictOpConjointOver        0x23
#define PictOpConjointOverReverse 0x24
#define PictOpConjointIn          0x25
#define PictOpConjointInReverse   0x26
#define PictOpConjointOut         0x27
#define PictOpConjointOutReverse  0x28
#define PictOpConjointAtop        0x29
#define PictOpConjointAtopReverse 0x2A
#define PictOpConjointXor         0x2B
#define PictOpMultiply            0x30
#define PictOpScreen              0x31
#define PictOpOverlay             0x32
#define PictOpDarken              0x33
#define PictOpLighten             0x34
#define PictOpColorDodge          0x35
#define PictOpColorBurn           0x36
#define PictOpHardLight           0x37
#define PictOpSoftLight           0x38
#define PictOpDifference          0x39
#define PictOpExclusion           0x3A
#define PictOpHSLHue              0x3B
#define PictOpHSLSaturation       0x3C
#define PictOpHSLColor            0x3D
#define PictOpHSLLuminosity       0x3E

#define SubPixelUnknown       0
#define SubPixelHorizontalRGB 1
#define SubPixelHorizontalBGR 2
#define SubPixelVerticalRGB   3
#define SubPixelVerticalBGR   4
#define SubPixelNone          5

typedef uint32_t Glyph;
typedef uint32_t GlyphSet;
typedef uint32_t Picture;
typedef uint32_t PictFormat;
typedef int32_t XFixed;

typedef struct
{
	int16_t red;
	int16_t redMask;
	int16_t green;
	int16_t greenMask;
	int16_t blue;
	int16_t blueMask;
	int16_t alpha;
	int16_t alphaMask;
} XRenderDirectFormat;

typedef struct
{
	PictFormat id;
	int32_t type;
	int32_t depth;
	XRenderDirectFormat direct;
	Colormap colormap;
} XRenderPictFormat;

typedef struct
{
	Bool repeat;
	Picture alpha_map;
	int32_t alpha_x_origin;
	int32_t alpha_y_origin;
	int32_t clip_x_origin;
	int32_t clip_y_origin;
	Pixmap clip_mask;
	Bool graphics_exposure;
	int32_t subwindow_mode;
	int32_t poly_edge;
	int32_t poly_mode;
	Atom dither;
	Bool component_alpha;
} XRenderPictureAttributes;

typedef struct
{
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	uint16_t alpha;
} XRenderColor;

typedef struct
{
	uint16_t width;
	uint16_t height;
	int16_t x;
	int16_t y;
	int16_t xOff;
	int16_t yOff;
} XGlyphInfo;

typedef struct
{
	GlyphSet glyphset;
	const char *chars;
	int32_t nchars;
	int32_t xOff;
	int32_t yOff;
} XGlyphElt8;

typedef struct
{
	GlyphSet glyphset;
	const uint16_t *chars;
	int32_t nchars;
	int32_t xOff;
	int32_t yOff;
} XGlyphElt16;

typedef struct
{
	GlyphSet glyphset;
	const uint32_t *chars;
	int32_t nchars;
	int32_t xOff;
	int32_t yOff;
} XGlyphElt32;

typedef struct
{
	XFixed x;
	XFixed y;
} XPointFixed;

typedef struct
{
	XFixed p1;
	XFixed p2;
} XLineFixed;

typedef struct
{
	XFixed p1;
	XFixed p2;
	XFixed p3;
} XTriangle;

typedef struct
{
	XFixed top;
	XFixed bottom;
	XFixed left;
	XFixed right;
} XTrapezoid;

typedef struct
{
	XFixed matrix[3][3];
} XTransform;

typedef struct
{
	int32_t nfilter;
	char **filter;
	int32_t nalias;
	int16_t *alias;
} XFilters;

typedef struct
{
	uint32_t pixel;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	uint16_t alpha;
} XIndexValue;

Bool XRenderQueryExtension(Display *display,
                           int *first_event,
                           int *first_error);
Status XRenderQueryVersion(Display *display,
                           int *major,
                           int *minor);
Status XRenderQueryFormats(Display *display);

XRenderPictFormat *XRenderFindFormat(Display *display,
                                     unsigned long mask,
                                     const XRenderPictFormat *templ,
                                     int count);
XRenderPictFormat *XRenderFindVisualFormat(Display *display,
                                           const Visual *visual);
XRenderPictFormat *XRenderFindStandardFormat(Display *display,
                                             int format);

Picture XRenderCreatePicture(Display *display,
                             Drawable drawable,
                             const XRenderPictFormat *format,
                             unsigned long valuemask,
                             const XRenderPictureAttributes *attributes);

void XRenderFillRectangle(Display *display,
                          int op,
                          Picture dst,
                          const XRenderColor *color,
                          int x,
                          int y,
                          unsigned int width,
                          unsigned int height);
void XRenderFillRectangles(Display *display,
                           int op,
                           Picture dst,
                           const XRenderColor *color,
                           const XRectangle *rectangles,
                           int nrects);

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
                      unsigned int height);

#endif
