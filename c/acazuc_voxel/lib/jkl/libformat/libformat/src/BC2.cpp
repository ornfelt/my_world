#include "BC2.h"

#define RGB5TO8(v) ((((v) * 527) + 23) >> 6)
#define RGB6TO8(v) ((((v) * 259) + 33) >> 6)

namespace libformat
{

	void BC2::unpackBlock(uint32_t bx, uint32_t by, uint32_t width, uint32_t height, uint8_t *in, uint8_t *out)
	{
		uint32_t idx = (by * width + bx) * 4;
		uint32_t idxInc = width * 4 - 16;
		uint32_t colorBits = in[12] | (in[13] << 8) | (in[14] << 16) | (in[15] << 24);
		uint16_t color1 = (in[9] << 8) | in[8];
		uint8_t r1 = RGB5TO8(in[9] >> 3);
		uint8_t g1 = RGB6TO8((color1 >> 5) & 0x3F);
		uint8_t b1 = RGB5TO8(in[8] & 0x1F);
		uint16_t color2 = (in[11] << 8) | in[10];
		uint8_t r2 = RGB5TO8(in[11] >> 3);
		uint8_t g2 = RGB6TO8((color2 >> 5) & 0x3F);
		uint8_t b2 = RGB5TO8(in[10] & 0x1F);
		uint32_t alphaIdx = 0;
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
						break;
					case 1:
						out[idx++] = r2;
						out[idx++] = g2;
						out[idx++] = b2;
						break;
					case 2:
						out[idx++] = (2 * r1 + r2) / 3;
						out[idx++] = (2 * g1 + g2) / 3;
						out[idx++] = (2 * b1 + b2) / 3;
						break;
					case 3:
						out[idx++] = (2 * r2 + r1) / 3;
						out[idx++] = (2 * g2 + g1) / 3;
						out[idx++] = (2 * b2 + b1) / 3;
						break;
				}
				colorBits >>= 2;
				if (x & 1)
				{
					out[idx] = in[alphaIdx++] & 0xF0;
					out[idx] |= out[idx] >> 4;
				}
				else
				{
					out[idx] = in[alphaIdx] & 0x0F;
					out[idx] |= out[idx] << 4;
				}
				idx++;
			}
			idx += idxInc;
		}
	}

	void BC2::read(uint32_t width, uint32_t height, uint8_t *in, uint8_t *out)
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
