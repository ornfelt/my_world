#if defined(__i386__) || defined(__x86_64__)
#include "arch/x86/asm.h"
#endif

#include <endian.h>
#include <timer.h>
#include <tty.h>
#include <std.h>
#include <sma.h>
#include <fb.h>

#if defined(__i386__) || defined(__x86_64__)

#define CRTC_ADDR 0x3D4
#define CRTC_DATA 0x3D5

#define CRTC_ADDR_HTOTAL     0x0
#define CRTC_ADDR_HDSPLY_END 0x1
#define CRTC_ADDR_HBLANK_BEG 0x2
#define CRTC_ADDR_HBLANK_END 0x3
#define CRTC_ADDR_HRTRC_BEG  0x4
#define CRTC_ADDR_HRTRC_END  0x5
#define CRTC_ADDR_VTOTAL     0x6
#define CRTC_ADDR_OVERFLOW   0x7
#define CRTC_ADDR_PRST_RS    0x8
#define CRTC_ADDR_MAX_SL     0x9
#define CRTC_ADDR_CURSOR_BEG 0xA
#define CRTC_ADDR_CURSOR_END 0xB
#define CRTC_ADDR_START_HGH  0xC
#define CRTC_ADDR_START_LOW  0xD
#define CRTC_ADDR_CURSOR_HGH 0xE
#define CRTC_ADDR_CURSOR_LOW 0xF
#define CRTC_ADDR_VRTRC_BEG  0x10
#define CRTC_ADDR_VRTRC_END  0x11
#define CRTC_ADDR_VDSPLY_END 0x12
#define CRTC_ADDR_OFFSET     0x13
#define CRTC_ADDR_UNDERLINE  0x14
#define CRTC_ADDR_VBLANK_BEG 0x15
#define CRTC_ADDR_VBLANK_END 0x16
#define CRTC_ADDR_CRTC_MODE  0x17
#define CRTC_ADDR_LINE_CMP   0x18

#endif

#define FONT_SCALE_X 1
#define FONT_SCALE_Y 1

extern const uint8_t font8x8[256][8];

enum vga_color
{
	VGA_COLOR_BLACK         = 0x0,
	VGA_COLOR_BLUE          = 0x1,
	VGA_COLOR_GREEN         = 0x2,
	VGA_COLOR_CYAN          = 0x3,
	VGA_COLOR_RED           = 0x4,
	VGA_COLOR_MAGENTA       = 0x5,
	VGA_COLOR_BROWN         = 0x6,
	VGA_COLOR_LIGHT_GREY    = 0x7,
	VGA_COLOR_DARK_GREY     = 0x8,
	VGA_COLOR_LIGHT_BLUE    = 0x9,
	VGA_COLOR_LIGHT_GREEN   = 0xA,
	VGA_COLOR_LIGHT_CYAN    = 0xB,
	VGA_COLOR_LIGHT_RED     = 0xC,
	VGA_COLOR_LIGHT_MAGENTA = 0xD,
	VGA_COLOR_LIGHT_BROWN   = 0xE,
	VGA_COLOR_WHITE         = 0xF,
};

static const uint8_t vga_fg_colors[] =
{
	VGA_COLOR_BLACK,
	VGA_COLOR_RED,
	VGA_COLOR_GREEN,
	VGA_COLOR_BROWN,
	VGA_COLOR_BLUE,
	VGA_COLOR_MAGENTA,
	VGA_COLOR_CYAN,
	VGA_COLOR_LIGHT_GREY,
	VGA_COLOR_LIGHT_GREY,
	VGA_COLOR_LIGHT_GREY,
	VGA_COLOR_DARK_GREY,
	VGA_COLOR_LIGHT_RED,
	VGA_COLOR_LIGHT_GREEN,
	VGA_COLOR_LIGHT_BROWN,
	VGA_COLOR_LIGHT_BLUE,
	VGA_COLOR_LIGHT_MAGENTA,
	VGA_COLOR_LIGHT_CYAN,
	VGA_COLOR_WHITE,
	VGA_COLOR_WHITE,
	VGA_COLOR_WHITE
};

static const uint8_t vga_bg_colors[] =
{
	VGA_COLOR_BLACK,
	VGA_COLOR_RED,
	VGA_COLOR_GREEN,
	VGA_COLOR_BROWN,
	VGA_COLOR_BLUE,
	VGA_COLOR_MAGENTA,
	VGA_COLOR_CYAN,
	VGA_COLOR_LIGHT_GREY,
	VGA_COLOR_BLACK,
	VGA_COLOR_BLACK,
	VGA_COLOR_DARK_GREY,
	VGA_COLOR_LIGHT_RED,
	VGA_COLOR_LIGHT_GREEN,
	VGA_COLOR_LIGHT_BROWN,
	VGA_COLOR_LIGHT_BLUE,
	VGA_COLOR_LIGHT_MAGENTA,
	VGA_COLOR_LIGHT_CYAN,
	VGA_COLOR_WHITE,
	VGA_COLOR_BLACK,
	VGA_COLOR_BLACK
};

struct vtty_color
{
	enum
	{
		VTTY_COLOR_24,
		VTTY_COLOR_STD,
	} type;
	union
	{
		struct
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
		} rgb24;
		uint8_t std;
	};
	uint32_t value;
};

struct vtty
{
	struct tty *tty;
	struct fb *fb;
	int cursor_enabled;
	int bold;
	struct vtty_color fg_color;
	struct vtty_color bg_color;
	uint32_t old_cursor_x;
	uint32_t old_cursor_y;
	size_t font_width;
	size_t font_height;
	size_t bytes_per_char;
	size_t *line_widths;
	uint8_t *buf;
	int dirty;
	uint32_t dirty_minx;
	uint32_t dirty_maxx;
	uint32_t dirty_miny;
	uint32_t dirty_maxy;
	struct timespec last_cursor_time;
	struct timer cursor_timer;
	int cursor_flip_flop;
};

#define MK24(r, g, b) (((uint32_t)(r) << 16) \
                     | ((uint32_t)(g) << 8) \
                     | ((uint32_t)(b) << 0))

#if 0
static const uint32_t rgb_colors[256] = /* vga */
{
	MK24(0  , 0  , 0  ), /* black */
	MK24(170, 0  , 0  ), /* red */
	MK24(0  , 170, 0  ), /* green */
	MK24(170, 85 , 0  ), /* brown */
	MK24(0  , 0  , 170), /* blue */
	MK24(170, 0  , 170), /* magenta */
	MK24(0  , 170, 170), /* cyan */
	MK24(170, 170, 170), /* light grey */
	MK24(170, 170, 170), /* light grey */
	MK24(170, 170, 170), /* light grey */
	MK24(85 , 85 , 85 ), /* dark grey */
	MK24(255, 85 , 85 ), /* light red */
	MK24(85 , 255, 85 ), /* light green */
	MK24(255, 255, 85 ), /* light brown */
	MK24(85 , 85 , 255), /* light blue */
	MK24(255, 85 , 255), /* light magenta */
	MK24(85 , 255, 255), /* light cyan */
	MK24(255, 255, 255), /* white */
	MK24(255, 255, 255), /* white */
	MK24(255, 255, 255), /* white */
};
#else
static const uint32_t rgb_colors[256] = /* xterm */
{
	MK24(0  , 0  , 0  ), /* black */
	MK24(205, 0  , 0  ), /* red */
	MK24(0  , 205, 0  ), /* green */
	MK24(205, 205, 0  ), /* brown */
	MK24(0  , 0  , 238), /* blue */
	MK24(205, 0  , 205), /* magenta */
	MK24(0  , 205, 205), /* cyan */
	MK24(229, 229, 229), /* light grey */
	MK24(229, 229, 229), /* light grey */
	MK24(229, 229, 229), /* light grey */
	MK24(127, 127, 127), /* dark grey */
	MK24(255, 0  , 0  ), /* light red */
	MK24(0  , 255, 0  ), /* light green */
	MK24(255, 255, 0  ), /* light brown */
	MK24(92 , 92 , 255), /* light blue */
	MK24(255, 0  , 255), /* light magenta */
	MK24(0  , 255, 255), /* light cyan */
	MK24(255, 255, 255), /* white */
	MK24(255, 255, 255), /* white */
	MK24(255, 255, 255), /* white */
};
#endif

static struct sma vtty_sma;

void vtty_init(void)
{
	sma_init(&vtty_sma, sizeof(struct vtty), NULL, NULL, "vtty");
}

static void set_pixel(struct vtty *vtty, uint32_t x, uint32_t y, uint32_t v)
{
	/* XXX handle bpp != 32 */
	size_t off = y * vtty->fb->pitch + x * vtty->fb->bpp / 8;
	*(uint32_t*)&vtty->fb->data[off] = htole32(v);
	*(uint32_t*)&vtty->buf[off] = htole32(v);
}

static void set_direct_pixel(struct vtty *vtty, uint32_t x, uint32_t y,
                             uint32_t v)
{
	/* XXX handle bpp != 32 */
	size_t off = y * vtty->fb->pitch + x * vtty->fb->bpp / 8;
	*(uint32_t*)&vtty->fb->data[off] = htole32(v);
}

static void vtty_dirty(struct vtty *vtty, uint32_t minx, uint32_t miny,
                       uint32_t maxx, uint32_t maxy)
{
	vtty->dirty = 1;
	if (minx < vtty->dirty_minx)
		vtty->dirty_minx = minx;
	if (miny < vtty->dirty_miny)
		vtty->dirty_miny = miny;
	if (maxx > vtty->dirty_maxx)
		vtty->dirty_maxx = maxx;
	if (maxy > vtty->dirty_maxy)
		vtty->dirty_maxy = maxy;
}

static void set_char_bgra8(struct vtty *vtty, size_t cx, size_t cy, uint8_t c,
                           uint32_t fg, uint32_t bg)
{
	size_t x = cx * vtty->font_width;
	size_t y = cy * vtty->font_height;
	for (size_t i = 0; i < vtty->font_width; ++i)
	{
		for (size_t j = 0; j < vtty->font_height; ++j)
			set_pixel(vtty, x + i, y + j,
			          (font8x8[c][j / FONT_SCALE_Y] & (1 << (i / FONT_SCALE_X))) ? fg : bg);
	}
	if (cx >= vtty->line_widths[cy])
		vtty->line_widths[cy] = cx + 1;
	vtty_dirty(vtty, x, y, x + vtty->font_width - 1, y + vtty->font_height - 1);
}

static void set_char_vga(struct vtty *vtty, size_t cx, size_t cy, uint8_t c,
                         uint32_t fg, uint32_t bg)
{
	size_t off = cy * vtty->fb->pitch + cx * vtty->fb->bpp / 8;
	*(uint16_t*)&vtty->fb->data[off] = fg | bg | c;
	*(uint16_t*)&vtty->buf[off] = fg | bg | c;
	if (cx >= vtty->line_widths[cy])
		vtty->line_widths[cy] = cx + 1;
	vtty_dirty(vtty, cx, cy, cx, cy);
}

static void set_char(struct vtty *vtty, size_t cx, size_t cy, uint8_t c,
                     uint32_t fg, uint32_t bg)
{
	switch (vtty->fb->format)
	{
		case FB_FMT_B8G8R8A8:
			set_char_bgra8(vtty, cx, cy, c, fg, bg);
			break;
		case FB_FMT_VGA:
			set_char_vga(vtty, cx, cy, c, fg, bg);
			break;
	}
}

static void scroll_up(struct vtty *vtty)
{
	uint32_t dst = 0;
	uint32_t src = dst + vtty->font_height * vtty->fb->pitch;
	uint32_t rows = vtty->tty->winsize.ws_row;
	for (size_t y = 0; y < rows - 1; ++y)
	{
		size_t line_width = vtty->line_widths[y];
		size_t next_width = vtty->line_widths[y + 1];
		if (next_width > line_width)
			line_width = next_width;
		line_width *= vtty->bytes_per_char;
		for (size_t n = 0; n < vtty->font_height; ++n)
		{
			memcpy(&vtty->fb->data[dst], &vtty->buf[src], line_width);
			memcpy(&vtty->buf[dst], &vtty->buf[src], line_width);
			dst += vtty->fb->pitch;
			src += vtty->fb->pitch;
		}
	}
	size_t line_width = vtty->line_widths[rows - 1];
	line_width *= vtty->bytes_per_char;
	for (size_t n = 0; n < vtty->font_height; ++n)
	{
		memset(&vtty->fb->data[dst], 0, line_width);
		memset(&vtty->buf[dst], 0, line_width);
		dst += vtty->fb->pitch;
	}
	memmove(&vtty->line_widths[0], &vtty->line_widths[1],
	        sizeof(*vtty->line_widths) * (rows - 1));
	vtty->line_widths[rows - 1] = 0;
	/* XXX width & height could be better computed
	 * by using min of line_widths
	 */
	vtty_dirty(vtty, 0, 0, vtty->fb->width - 1, vtty->fb->height - 1);
}

static void hide_old_cursor(struct vtty *vtty)
{
	if (vtty->fb->format == FB_FMT_VGA)
		return;
	uint32_t min_y = (vtty->old_cursor_y * 8 + 6) * FONT_SCALE_Y;
	uint32_t max_y = min_y + 2 * FONT_SCALE_Y;
	uint32_t min_x = (vtty->old_cursor_x * 8) * FONT_SCALE_X;
	uint32_t max_x = min_x + 8 * FONT_SCALE_X;
	for (uint32_t y = min_y; y < max_y; ++y)
	{
		for (uint32_t x = min_x; x < max_x; ++x)
		{
			size_t off = y * vtty->fb->pitch + x * vtty->fb->bpp / 8;
			uint32_t *dst = (uint32_t*)&vtty->fb->data[off];
			uint32_t *src = (uint32_t*)&vtty->buf[off];
			*dst = *src;
		}
	}
	vtty_dirty(vtty, min_x, min_y, max_x - 1, max_y - 1);
}

static void draw_cursor(struct vtty *vtty)
{
	struct tty *tty = vtty->tty;
	if (vtty->fb->format == FB_FMT_VGA)
	{
#if defined(__i386__) || defined(__x86_64__)
		uint16_t p = vtty->tty->cursor_y * vtty->fb->width + vtty->tty->cursor_x;
		outb(CRTC_ADDR, CRTC_ADDR_CURSOR_HGH);
		outb(CRTC_DATA, (p >> 8) & 0xFF);
		outb(CRTC_ADDR, CRTC_ADDR_CURSOR_LOW);
		outb(CRTC_DATA, p & 0xFF);
#endif
		return;
	}
	uint32_t min_y = (tty->cursor_y * 8 + 6) * FONT_SCALE_Y;
	uint32_t max_y = min_y + 2 * FONT_SCALE_Y;
	uint32_t min_x = (tty->cursor_x * 8) * FONT_SCALE_X;
	uint32_t max_x = min_x + 8 * FONT_SCALE_X;
	for (uint32_t y = min_y; y < max_y; ++y)
	{
		for (uint32_t x = min_x; x < max_x; ++x)
			set_direct_pixel(vtty, x, y, 0xFFFFFFFF);
	}
	vtty_dirty(vtty, min_x, min_y, max_x - 1, max_y - 1);
}

static void update_cursor(struct vtty *vtty)
{
	struct tty *tty = vtty->tty;
	if (vtty->cursor_enabled)
	{
		hide_old_cursor(vtty);
		draw_cursor(vtty);
	}
	vtty->old_cursor_x = tty->cursor_x;
	vtty->old_cursor_y = tty->cursor_y;
}

static void update_color(struct vtty *vtty, struct vtty_color *color, int fg)
{
	switch (vtty->fb->format)
	{
		case FB_FMT_B8G8R8A8:
			switch (color->type)
			{
				case VTTY_COLOR_24:
					color->value = MK24(color->rgb24.r,
					                    color->rgb24.g,
					                    color->rgb24.b);
					break;
				case VTTY_COLOR_STD:
					if (fg)
						color->value = rgb_colors[color->std + vtty->bold * 10];
					else
						color->value = rgb_colors[color->std];
			}
			break;
		case FB_FMT_VGA:
			switch (color->type)
			{
				case VTTY_COLOR_24:
					if (fg)
						color->value = VGA_COLOR_WHITE << 8;
					else
						color->value = VGA_COLOR_WHITE << 12;
					break;
				case VTTY_COLOR_STD:
					if (fg)
						color->value = vga_fg_colors[color->std + vtty->bold * 10] << 8;
					else
						color->value = vga_bg_colors[color->std] << 12;
					break;
			}
			break;
	}
}

static void enable_cursor(struct vtty *vtty)
{
	if (vtty->cursor_enabled)
		return;
	vtty->cursor_enabled = 1;
#if defined(__i386__) || defined(__x86_64__)
	if (vtty->fb->format == FB_FMT_VGA)
	{
		outb(CRTC_ADDR, CRTC_ADDR_CURSOR_BEG);
		outb(CRTC_DATA, (inb(CRTC_DATA) & 0xC0) | 13);
		outb(CRTC_ADDR, CRTC_ADDR_CURSOR_END);
		outb(CRTC_DATA, (inb(CRTC_DATA) & 0xE0) | 15);
	}
#endif
	draw_cursor(vtty);
}

static void disable_cursor(struct vtty *vtty)
{
	if (!vtty->cursor_enabled)
		return;
	hide_old_cursor(vtty);
	vtty->cursor_enabled = 0;
#if defined(__i386__) || defined(__x86_64__)
	if (vtty->fb->format == FB_FMT_VGA)
	{
		outb(CRTC_ADDR, CRTC_ADDR_CURSOR_BEG);
		outb(CRTC_DATA, 0x20);
	}
#endif
}

static void vtty_flush(struct tty *tty)
{
	struct vtty *vtty = tty->userdata;
	if (!vtty->dirty)
		return;
	fb_flush(vtty->fb, vtty->dirty_minx, vtty->dirty_miny,
	         vtty->dirty_maxx - vtty->dirty_minx + 1,
	         vtty->dirty_maxy - vtty->dirty_miny + 1);
	vtty->dirty = 0;
	vtty->dirty_minx = UINT32_MAX;
	vtty->dirty_maxx = 0;
	vtty->dirty_miny = UINT32_MAX;
	vtty->dirty_maxy = 0;
}

static void putchar(struct vtty *vtty, uint8_t c)
{
	struct tty *tty = vtty->tty;
	if (!c)
		return;
	switch (c)
	{
		case 0x7F:
			if (tty->cursor_x == 0)
			{
				tty->cursor_x = tty->winsize.ws_col - 1u;
				if (tty->cursor_y)
					tty->cursor_y--;
			}
			else
			{
				tty->cursor_x--;
			}
			update_cursor(vtty);
			set_char(vtty, tty->cursor_x, tty->cursor_y, ' ',
			         rgb_colors[7], rgb_colors[0]);
			return;
		case '\b':
			if (tty->cursor_x == 0)
			{
				tty->cursor_x = tty->winsize.ws_col - 1u;
				if (tty->cursor_y)
					tty->cursor_y--;
			}
			else
			{
				tty->cursor_x--;
			}
			update_cursor(vtty);
			return;
		case '\n':
			if (tty->cursor_y >= tty->winsize.ws_row - 1u)
				scroll_up(vtty);
			else
				tty->cursor_y++;
			tty->cursor_x = 0;
			update_cursor(vtty);
			/* set_char(tty->cursor_x, tty->cursor_y, ' ', entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)); */
			return;
		case '\t':
		{
			uint8_t to = (tty->cursor_x & ~7) + 8;
			for (size_t i = tty->cursor_x; i < to; ++i)
				set_char(vtty, i, tty->cursor_y, ' ',
				         vtty->fg_color.value, vtty->bg_color.value);
			tty->cursor_x = to;
			break;
		}
		default:
			set_char(vtty, tty->cursor_x, tty->cursor_y, c,
			         vtty->fg_color.value, vtty->bg_color.value);
			tty->cursor_x++;
			break;
	}
	if (tty->cursor_x >= tty->winsize.ws_col)
	{
		tty->cursor_x = 0;
		if (tty->cursor_y >= tty->winsize.ws_row - 1u)
			scroll_up(vtty);
		else
			tty->cursor_y++;
	}
	update_cursor(vtty);
}

static ssize_t vtty_write(struct tty *tty, const void *data, size_t len)
{
	struct vtty *vtty = tty->userdata;
	for (size_t i = 0; i < len; ++i)
		putchar(vtty, ((uint8_t*)data)[i]);
	return len;
}

static int vtty_ctrl(struct tty *tty, enum tty_ctrl ctrl, uint32_t val)
{
	struct vtty *vtty = tty->userdata;
	switch (ctrl)
	{
		case TTY_CTRL_CUP:
		{
			uint8_t row = val >> 8;
			uint8_t col = val;
			if (row >= tty->winsize.ws_row)
				row = tty->winsize.ws_row - 1;
			if (col >= tty->winsize.ws_col)
				col = tty->winsize.ws_col - 1;
			tty->cursor_x = col;
			tty->cursor_y = row;
			update_cursor(vtty);
			break;
		}
		case TTY_CTRL_CUU:
		{
			if (tty->cursor_y > 0)
			{
				tty->cursor_y--;
				update_cursor(vtty);
			}
			break;
		}
		case TTY_CTRL_CUD:
		{
			if (tty->cursor_y < tty->winsize.ws_row - 1u)
			{
				tty->cursor_y++;
				update_cursor(vtty);
			}
			break;
		}
		case TTY_CTRL_CUB:
		{
			if (tty->cursor_x > 0)
			{
				tty->cursor_x--;
				update_cursor(vtty);
			}
			break;
		}
		case TTY_CTRL_CUF:
		{
			if (tty->cursor_x < tty->winsize.ws_col - 1u)
			{
				tty->cursor_x++;
				update_cursor(vtty);
			}
			break;
		}
		case TTY_CTRL_CNL:
		{
			if (tty->cursor_y < tty->winsize.ws_row - 1u)
				tty->cursor_y++;
			tty->cursor_x = 0;
			update_cursor(vtty);
			break;
		}
		case TTY_CTRL_CPL:
		{
			if (tty->cursor_y > 0)
				tty->cursor_y--;
			tty->cursor_x = 0;
			update_cursor(vtty);
			break;
		}
		case TTY_CTRL_GC:
			vtty->bold = 0;
			vtty->fg_color.type = VTTY_COLOR_STD;
			vtty->fg_color.std = 7;
			update_color(vtty, &vtty->fg_color, 1);
			vtty->bg_color.type = VTTY_COLOR_STD;
			vtty->bg_color.std = 0;
			update_color(vtty, &vtty->bg_color, 0);
			break;
		case TTY_CTRL_GFG:
			vtty->fg_color.type = VTTY_COLOR_STD;
			vtty->fg_color.std = val;
			update_color(vtty, &vtty->fg_color, 1);
			break;
		case TTY_CTRL_GFG24:
			vtty->fg_color.type = VTTY_COLOR_24;
			vtty->fg_color.rgb24.r = val >> 16;
			vtty->fg_color.rgb24.g = val >> 8;
			vtty->fg_color.rgb24.b = val >> 0;
			update_color(vtty, &vtty->fg_color, 1);
			break;
		case TTY_CTRL_GBG:
			vtty->bg_color.type = VTTY_COLOR_STD;
			vtty->bg_color.std = val;
			update_color(vtty, &vtty->bg_color, 0);
			break;
		case TTY_CTRL_GBG24:
			vtty->bg_color.type = VTTY_COLOR_24;
			vtty->bg_color.rgb24.r = val >> 16;
			vtty->bg_color.rgb24.g = val >> 8;
			vtty->bg_color.rgb24.b = val >> 0;
			update_color(vtty, &vtty->bg_color, 0);
			break;
		case TTY_CTRL_GB:
			vtty->bold = 1;
			update_color(vtty, &vtty->fg_color, 1);
			break;
		case TTY_CTRL_GRB:
			vtty->bold = 0;
			update_color(vtty, &vtty->fg_color, 1);
			break;
		case TTY_CTRL_PCD:
			disable_cursor(vtty);
			break;
		case TTY_CTRL_PCE:
			enable_cursor(vtty);
			break;
		case TTY_CTRL_ED:
			switch (val)
			{
				case 2:
					memset(vtty->fb->data, 0,
					       vtty->fb->pitch * vtty->fb->height);
					tty->cursor_x = 0;
					tty->cursor_y = 0;
					update_cursor(vtty);
					break;
			}
			break;
		case TTY_CTRL_ES:
			memset(vtty->fb->data, 0,
			        vtty->fb->pitch * vtty->fb->height);
			tty->cursor_x = 0;
			tty->cursor_y = 0;
			update_cursor(vtty);
			break;
		default:
			break;
	}
	return 0;
}

static const struct tty_op tty_op =
{
	.write = vtty_write,
	.ctrl = vtty_ctrl,
	.flush = vtty_flush,
};

static void cursor_timer_add(struct vtty *vtty);

static void cursor_timer_cb(struct timer *timer)
{
	struct vtty *vtty = timer->userdata;
	if (vtty->cursor_flip_flop)
	{
		hide_old_cursor(vtty);
		vtty->cursor_flip_flop = 0;
	}
	else
	{
		draw_cursor(vtty);
		vtty->cursor_flip_flop = 1;
	}
	cursor_timer_add(vtty);
}

static void cursor_timer_add(struct vtty *vtty)
{
	struct timespec ts;
	struct timespec add;
	add.tv_sec = 0;
	add.tv_nsec = 500000000;
	ts = vtty->last_cursor_time;
	timespec_add(&ts, &ts, &add);
	timer_add(&vtty->cursor_timer, CLOCK_MONOTONIC, ts, cursor_timer_cb, vtty);
	vtty->last_cursor_time = ts;
}

void vtty_free(struct vtty *vtty)
{
	if (!vtty)
		return;
	if (vtty->cursor_timer.queued)
		timer_remove(&vtty->cursor_timer);
	free(vtty->line_widths);
	free(vtty->buf);
	tty_free(vtty->tty);
	sma_free(&vtty_sma, vtty);
}

int vtty_alloc(const char *name, int id, struct fb *fb, struct tty **ttyp)
{
	struct vtty *vtty = sma_alloc(&vtty_sma, M_ZERO);
	if (!vtty)
		return -ENOMEM;
	int ret = tty_alloc(name, makedev(4, id), &tty_op, &vtty->tty);
	if (ret)
	{
		printf("vtty: tty allocation failed\n");
		goto err;
	}
	vtty->tty->userdata = vtty;
	vtty->fb = fb;
	vtty->fg_color.type = VTTY_COLOR_STD;
	vtty->fg_color.std = 7;
	update_color(vtty, &vtty->fg_color, 1);
	vtty->bg_color.type = VTTY_COLOR_STD;
	vtty->bg_color.std = 0;
	update_color(vtty, &vtty->bg_color, 0);
	vtty->cursor_enabled = 1;
	vtty->dirty = 0;
	vtty->dirty_minx = UINT32_MAX;
	vtty->dirty_maxx = 0;
	vtty->dirty_miny = UINT32_MAX;
	vtty->dirty_maxy = 0;
	switch (vtty->fb->format)
	{
		case FB_FMT_B8G8R8A8:
			vtty->font_width = 8 * FONT_SCALE_X;
			vtty->font_height = 8 * FONT_SCALE_Y;
			break;
		case FB_FMT_VGA:
			vtty->font_width = 1;
			vtty->font_height = 1;
			break;
	}
	vtty->tty->winsize.ws_col = vtty->fb->width / vtty->font_width;
	vtty->tty->winsize.ws_row = vtty->fb->height / vtty->font_height;
	vtty->bytes_per_char = vtty->font_width * vtty->fb->bpp / 8;
	vtty->line_widths = malloc(sizeof(*vtty->line_widths) * vtty->tty->winsize.ws_row, M_ZERO);
	if (!vtty->line_widths)
	{
		printf("vtty: failed to allocate vga line widths\n");
		ret = -ENOMEM;
		goto err;
	}
	vtty->buf = malloc(vtty->fb->pitch * vtty->fb->height, M_ZERO);
	if (!vtty->buf)
	{
		panic("vtty: failed to allocate backbuffer\n");
		ret = -ENOMEM;
		goto err;
	}
	update_cursor(vtty);
	if (vtty->fb->format != FB_FMT_VGA)
	{
		clock_gettime(CLOCK_MONOTONIC, &vtty->last_cursor_time);
		cursor_timer_add(vtty);
	}
	*ttyp = vtty->tty;
	return 0;

err:
	vtty_free(vtty);
	return ret;
}
