#include "BC5.h"

namespace libformat
{

	void BC5::unpackBlock(uint32_t bx, uint32_t by, uint32_t width, uint32_t height, uint8_t *in, uint8_t *out)
	{
		uint32_t idx = (by * width + bx) * 2;
		uint32_t idxInc = width * 2 - 8;
		uint32_t redBits1 = (in[4] << 16) | (in[3] << 8) | in[2];
		uint32_t redBits2 = (in[7] << 16) | (in[6] << 8) | in[5];
		uint8_t reds[8];
		reds[0] = in[0];
		reds[1] = in[1];
		if (reds[0] > reds[1])
		{
			reds[2] = (6 * reds[0] + 1 * reds[1]) / 7;
			reds[3] = (5 * reds[0] + 2 * reds[1]) / 7;
			reds[4] = (4 * reds[0] + 3 * reds[1]) / 7;
			reds[5] = (3 * reds[0] + 4 * reds[1]) / 7;
			reds[6] = (2 * reds[0] + 5 * reds[1]) / 7;
			reds[7] = (1 * reds[0] + 6 * reds[1]) / 7;
		}
		else
		{
			reds[2] = (4 * reds[0] + 1 * reds[1]) / 5;
			reds[3] = (3 * reds[0] + 2 * reds[1]) / 5;
			reds[4] = (2 * reds[0] + 3 * reds[1]) / 5;
			reds[5] = (1 * reds[0] + 4 * reds[1]) / 5;
			reds[6] = 0;
			reds[7] = 0xFF;
		}
		uint32_t greenBits1 = (in[4] << 16) | (in[3] << 8) | in[2];
		uint32_t greenBits2 = (in[7] << 16) | (in[6] << 8) | in[5];
		uint8_t greens[8];
		greens[0] = in[0];
		greens[1] = in[1];
		if (greens[0] > greens[1])
		{
			greens[2] = (6 * greens[0] + 1 * greens[1]) / 7;
			greens[3] = (5 * greens[0] + 2 * greens[1]) / 7;
			greens[4] = (4 * greens[0] + 3 * greens[1]) / 7;
			greens[5] = (3 * greens[0] + 4 * greens[1]) / 7;
			greens[6] = (2 * greens[0] + 5 * greens[1]) / 7;
			greens[7] = (1 * greens[0] + 6 * greens[1]) / 7;
		}
		else
		{
			greens[2] = (4 * greens[0] + 1 * greens[1]) / 5;
			greens[3] = (3 * greens[0] + 2 * greens[1]) / 5;
			greens[4] = (2 * greens[0] + 3 * greens[1]) / 5;
			greens[5] = (1 * greens[0] + 4 * greens[1]) / 5;
			greens[6] = 0;
			greens[7] = 0xFF;
		}
		for (uint32_t y = 0; y < 4; ++y)
		{
			for (uint32_t x = 0; x < 4; ++x)
			{
				if (y < 2)
				{
					out[idx++] = reds[redBits1 & 7];
					redBits1 >>= 3;
					out[idx++] = greens[greenBits1 & 7];
					greenBits1 >>= 3;
				}
				else
				{
					out[idx++] = reds[redBits2 & 7];
					redBits2 >>= 3;
					out[idx++] = greens[greenBits2 & 7];
					greenBits2 >>= 3;
				}
			}
			idx += idxInc;
		}
	}

	void BC5::read(uint32_t width, uint32_t height, uint8_t *in, uint8_t *out)
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
