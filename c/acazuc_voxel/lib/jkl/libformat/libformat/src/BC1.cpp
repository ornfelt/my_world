#include "BC1.h"

#define RGB5TO8(v) ((((v) * 527) + 23) >> 6)
#define RGB6TO8(v) ((((v) * 259) + 33) >> 6)

namespace libformat
{

	void BC1::unpackBlock(uint32_t bx, uint32_t by, uint32_t width, uint32_t height, uint8_t *in, uint8_t *out)
	{
		uint32_t idx = (by * width + bx) * 4;
		uint32_t idxInc = width * 4 - 16;
		uint32_t colorBits = in[4] | (in[5] << 8) | (in[6] << 16) | (in[7] << 24);
		uint16_t color1 = (in[1] << 8) | in[0];
		uint8_t r1 = RGB5TO8(in[1] >> 3);
		uint8_t g1 = RGB6TO8((color1 >> 5) & 0x3F);
		uint8_t b1 = RGB5TO8(in[0] & 0x1F);
		uint16_t color2 = (in[3] << 8) | in[2];
		uint8_t r2 = RGB5TO8(in[3] >> 3);
		uint8_t g2 = RGB6TO8((color2 >> 5) & 0x3F);
		uint8_t b2 = RGB5TO8(in[2] & 0x1F);
		for (uint32_t y = 0; y < 4; ++y)
		{
			for (uint32_t x = 0; x < 4; ++x)
			{
				switch (colorBits & 3)
				{
					case 0:
						out[idx++] = r1;
						out[idx++] = g1;
						out[idx++] = b1;
						out[idx++] = 0xff;
						break;
					case 1:
						out[idx++] = r2;
						out[idx++] = g2;
						out[idx++] = b2;
						out[idx++] = 0xff;
						break;
					case 2:
						if (color1 > color2)
						{
							out[idx++] = (2 * r1 + r2) / 3;
							out[idx++] = (2 * g1 + g2) / 3;
							out[idx++] = (2 * b1 + b2) / 3;
							out[idx++] = 0xff;
						}
						else
						{
							out[idx++] = (r1 + r2) / 2;
							out[idx++] = (g1 + g2) / 2;
							out[idx++] = (b1 + b2) / 2;
							out[idx++] = 0xff;
						}
						break;
					case 3:
						if (color1 > color2)
						{
							out[idx++] = (2 * r2 + r1) / 3;
							out[idx++] = (2 * g2 + g1) / 3;
							out[idx++] = (2 * b2 + b1) / 3;
							out[idx++] = 0xff;
						}
						else
						{
							out[idx++] = 0;
							out[idx++] = 0;
							out[idx++] = 0;
							out[idx++] = 0;
						}
						break;
				}
				colorBits >>= 2;
			}
			idx += idxInc;
		}
	}

	void BC1::read(uint32_t width, uint32_t height, uint8_t *in, uint8_t *out)
	{
		uint32_t bx = (width + 3) / 4;
		uint32_t by = (height + 3) / 4;
		for (uint32_t y = 0; y < by; ++y)
		{
			for (uint32_t x = 0; x < bx; ++x)
			{
				unpackBlock(x * 4, y * 4, width, height, in, out);
				in += 8;
			}
		}
	}

}
