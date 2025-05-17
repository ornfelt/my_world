#include "jkg.h"

#define U4TOF(v) ((v) * (1.0 / 0x0F))
#define U5TOF(v) ((v) * (1.0 / 0x1F))
#define U6TOF(v) ((v) * (1.0 / 0x3F))
#define U8TOF(v) ((v) * (1.0 / 0xFF))
#define S8TOF(v) ((v) * (1.0 / 0x7F))

static void
get_comp_rgb(float * restrict color,
             const uint8_t *in,
             uint32_t px,
             uint32_t py)
{
	uint32_t color_bits = in[4] | (in[5] << 8) | (in[6] << 16) | ((uint32_t)in[7] << 24);
	uint16_t color1 = (in[1] << 8) | in[0];
	float c1[3] =
	{
		U5TOF(in[1] >> 3),
		U6TOF((color1 >> 5) & 0x3F),
		U5TOF(in[0] & 0x1F),
	};
	uint16_t color2 = (in[3] << 8) | in[2];
	float c2[3] =
	{
		U5TOF(in[3] >> 3),
		U6TOF((color2 >> 5) & 0x3F),
		U5TOF(in[2] & 0x1F),
	};

	switch ((color_bits >> (2 * px + 8 * py)) & 3)
	{
		case 0:
			for (size_t i = 0; i < 3; ++i)
				color[i] = c1[i];
			break;
		case 1:
			for (size_t i = 0; i < 3; ++i)
				color[i] = c2[i];
			break;
		case 2:
			for (size_t i = 0; i < 3; ++i)
				color[i] = (2 * c1[i] + c2[i]) / 3;
			break;
		case 3:
			for (size_t i = 0; i < 3; ++i)
				color[i] = (2 * c2[i] + c1[i]) / 3;
			break;
	}
}

static float
get_comp_chan_unorm(const uint8_t *in,
                    uint32_t by,
                    uint32_t px,
                    uint32_t py)
{
	uint32_t value_bits1 = (in[4] << 16) | (in[3] << 8) | in[2];
	uint32_t value_bits2 = (in[7] << 16) | (in[6] << 8) | in[5];
	float values[8];

	values[0] = U8TOF(in[0]);
	values[1] = U8TOF(in[1]);
	if (values[0] > values[1])
	{
		values[2] = (6 * values[0] + 1 * values[1]) / 7;
		values[3] = (5 * values[0] + 2 * values[1]) / 7;
		values[4] = (4 * values[0] + 3 * values[1]) / 7;
		values[5] = (3 * values[0] + 4 * values[1]) / 7;
		values[6] = (2 * values[0] + 5 * values[1]) / 7;
		values[7] = (1 * values[0] + 6 * values[1]) / 7;
	}
	else
	{
		values[2] = (4 * values[0] + 1 * values[1]) / 5;
		values[3] = (3 * values[0] + 2 * values[1]) / 5;
		values[4] = (2 * values[0] + 3 * values[1]) / 5;
		values[5] = (1 * values[0] + 4 * values[1]) / 5;
		values[6] = 0;
		values[7] = 1;
	}
	if (by < 2)
		return values[(value_bits1 >> (3 * (px + py * 4))) & 7];
	return values[(value_bits2 >> (3 * (px + (py - 2) * 4))) & 7];
}

static float
get_comp_chan_snorm(const uint8_t *in,
                    uint32_t by,
                    uint32_t px,
                    uint32_t py)
{
	uint32_t value_bits1 = (in[4] << 16) | (in[3] << 8) | in[2];
	uint32_t value_bits2 = (in[7] << 16) | (in[6] << 8) | in[5];
	float values[8];

	values[0] = S8TOF(in[0]);
	values[1] = S8TOF(in[1]);
	if (values[0] > values[1])
	{
		values[2] = (6 * values[0] + 1 * values[1]) / 7;
		values[3] = (5 * values[0] + 2 * values[1]) / 7;
		values[4] = (4 * values[0] + 3 * values[1]) / 7;
		values[5] = (3 * values[0] + 4 * values[1]) / 7;
		values[6] = (2 * values[0] + 5 * values[1]) / 7;
		values[7] = (1 * values[0] + 6 * values[1]) / 7;
	}
	else
	{
		values[2] = (4 * values[0] + 1 * values[1]) / 5;
		values[3] = (3 * values[0] + 2 * values[1]) / 5;
		values[4] = (2 * values[0] + 3 * values[1]) / 5;
		values[5] = (1 * values[0] + 4 * values[1]) / 5;
		values[6] = 0;
		values[7] = 1;
	}
	if (by < 2)
		return values[(value_bits1 >> (3 * (px + py * 4))) & 7];
	return values[(value_bits2 >> (3 * (px + (py - 2) * 4))) & 7];
}

static void
get_bc1(float * restrict color,
        uint32_t x,
        uint32_t y,
        uint32_t width,
        uint32_t height,
        const void *data,
        float alpha)
{
	uint32_t bx = x / 4;
	uint32_t by = y / 4;
	uint32_t px = x % 4;
	uint32_t py = y % 4;
	const uint8_t *in = &((uint8_t*)data)[8 * (bx + width / 4 * by)];
	uint32_t color_bits = in[4] | (in[5] << 8) | (in[6] << 16) | ((uint32_t)in[7] << 24);
	uint16_t color1 = (in[1] << 8) | in[0];
	float c1[3] =
	{
		U5TOF(in[1] >> 3),
		U6TOF((color1 >> 5) & 0x3F),
		U5TOF(in[0] & 0x1F),
	};
	uint16_t color2 = (in[3] << 8) | in[2];
	float c2[3] =
	{
		U5TOF(in[3] >> 3),
		U6TOF((color2 >> 5) & 0x3F),
		U5TOF(in[2] & 0x1F),
	};

	(void)height;
	switch ((color_bits >> (2 * px + 8 * py)) & 3)
	{
		case 0:
			for (size_t i = 0; i < 3; ++i)
				color[i] = c1[i];
			color[3] = 1;
			break;
		case 1:
			for (size_t i = 0; i < 3; ++i)
				color[i] = c2[i];
			color[3] = 1;
			break;
		case 2:
			if (color1 > color2)
			{
				for (size_t i = 0; i < 3; ++i)
					color[i] = (2 * c1[i] + c2[i]) / 3;
				color[3] = 1;
			}
			else
			{
				for (size_t i = 0; i < 3; ++i)
					color[i] = (c1[i] + c2[i]) / 2;
				color[3] = 1;
			}
			break;
		case 3:
			if (color1 > color2)
			{
				for (size_t i = 0; i < 3; ++i)
					color[i] = (2 * c2[i] + c1[i]) / 3;
				color[3] = 1;
			}
			else
			{
				for (size_t i = 0; i < 3; ++i)
					color[i] = 0;
				color[3] = alpha;
			}
			break;
	}
}

void
jkg_get_bc1_rgb(float * restrict color,
                uint32_t x,
                uint32_t y,
                uint32_t width,
                uint32_t height,
                const void *data)
{
	get_bc1(color, x, y, width, height, data, 0);
}

void
jkg_get_bc1_rgba(float * restrict color,
                 uint32_t x,
                 uint32_t y,
                 uint32_t width,
                 uint32_t height,
                 const void *data)
{
	get_bc1(color, x, y, width, height, data, 1);
}

void
jkg_get_bc2(float * restrict color,
            uint32_t x,
            uint32_t y,
            uint32_t width,
            uint32_t height,
            const void *data)
{
	uint32_t bx = x / 4;
	uint32_t by = y / 4;
	uint32_t px = x % 4;
	uint32_t py = y % 4;
	const uint8_t *in = &((uint8_t*)data)[16 * (bx + width / 4 * by)];
	uint32_t alpha_idx = px / 2 + py * 2;

	(void)height;
	get_comp_rgb(color, &in[8], px, py);
	if (x & 1)
		color[3] = U4TOF((in[alpha_idx] & 0xF0) >> 4);
	else
		color[3] = U4TOF((in[alpha_idx] & 0x0F) >> 0);
}

void
jkg_get_bc3(float * restrict color,
            uint32_t x,
            uint32_t y,
            uint32_t width,
            uint32_t height,
            const void *data)
{
	uint32_t bx = x / 4;
	uint32_t by = y / 4;
	uint32_t px = x % 4;
	uint32_t py = y % 4;
	const uint8_t *in = &((uint8_t*)data)[16 * (bx + width / 4 * by)];

	(void)height;
	get_comp_rgb(color, &in[8], px, py);
	color[3] = get_comp_chan_unorm(&in[0], by, px, py);
}

void
jkg_get_bc4_unorm(float * restrict color,
                  uint32_t x,
                  uint32_t y,
                  uint32_t width,
                  uint32_t height,
                  const void *data)
{
	uint32_t bx = x / 4;
	uint32_t by = y / 4;
	uint32_t px = x % 4;
	uint32_t py = y % 4;
	const uint8_t *in = &((uint8_t*)data)[8 * (bx + width / 4 * by)];

	(void)height;
	color[0] = get_comp_chan_unorm(&in[0], by, px, py);
}

void
jkg_get_bc4_snorm(float * restrict color,
                  uint32_t x,
                  uint32_t y,
                  uint32_t width,
                  uint32_t height,
                  const void *data)
{
	uint32_t bx = x / 4;
	uint32_t by = y / 4;
	uint32_t px = x % 4;
	uint32_t py = y % 4;
	const uint8_t *in = &((uint8_t*)data)[8 * (bx + width / 4 * by)];

	(void)height;
	color[0] = get_comp_chan_snorm(&in[0], by, px, py);
}

void
jkg_get_bc5_unorm(float * restrict color,
                  uint32_t x,
                  uint32_t y,
                  uint32_t width,
                  uint32_t height,
                  const void *data)
{
	uint32_t bx = x / 4;
	uint32_t by = y / 4;
	uint32_t px = x % 4;
	uint32_t py = y % 4;
	const uint8_t *in = &((uint8_t*)data)[16 * (bx + width / 4 * by)];

	(void)height;
	color[0] = get_comp_chan_unorm(&in[0], by, px, py);
	color[1] = get_comp_chan_unorm(&in[8], by, px, py);
}

void
jkg_get_bc5_snorm(float * restrict color,
                  uint32_t x,
                  uint32_t y,
                  uint32_t width,
                  uint32_t height,
                  const void *data)
{
	uint32_t bx = x / 4;
	uint32_t by = y / 4;
	uint32_t px = x % 4;
	uint32_t py = y % 4;
	const uint8_t *in = &((uint8_t*)data)[16 * (bx + width / 4 * by)];

	(void)height;
	color[0] = get_comp_chan_snorm(&in[0], by, px, py);
	color[1] = get_comp_chan_snorm(&in[8], by, px, py);
}
