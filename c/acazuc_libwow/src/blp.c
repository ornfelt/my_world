#include "blp.h"

#include "common.h"
#include "mpq.h"

#include <string.h>
#include <stdlib.h>

struct wow_blp_file *wow_blp_file_new(struct wow_mpq_file *mpq)
{
	struct wow_blp_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	file->mipmaps = NULL;
	file->mipmaps_nb = 0;
	if (wow_mpq_read(mpq, &file->header, sizeof(file->header)) != sizeof(file->header))
		goto err;
	if (file->header.type != 1)
		goto err;
	if (file->header.has_mipmaps)
	{
		file->mipmaps_nb = 0;
		while (file->mipmaps_nb < 16 && file->header.mipmap_offsets[file->mipmaps_nb] && file->header.mipmap_lengths[file->mipmaps_nb] && file->header.width / (1 << file->mipmaps_nb) && file->header.height / (1 << file->mipmaps_nb))
			++file->mipmaps_nb;
	}
	else
	{
		file->mipmaps_nb = 1;
	}
	file->mipmaps = WOW_MALLOC(sizeof(*file->mipmaps) * file->mipmaps_nb);
	if (!file->mipmaps)
		goto err;
	memset(file->mipmaps, 0, sizeof(*file->mipmaps) * file->mipmaps_nb);
	switch (file->header.compression)
	{
		case 1:
		{
			for (uint32_t i = 0; i < file->mipmaps_nb; ++i)
			{
				struct wow_blp_mipmap *mipmap = &file->mipmaps[i];
				mipmap->width = file->header.width / (1 << i);
				mipmap->height = file->header.height / (1 << i);
				mipmap->data_len = file->header.mipmap_lengths[i];
				mipmap->data = WOW_MALLOC(mipmap->data_len);
				if (!mipmap->data)
					goto err;
				if (wow_mpq_seek(mpq, file->header.mipmap_offsets[i], SEEK_SET) == -1)
					goto err;
				if (wow_mpq_read(mpq, mipmap->data, file->header.mipmap_lengths[i]) != file->header.mipmap_lengths[i])
					goto err;
			}
			break;
		}
		case 2:
		{
			uint32_t width = file->header.width;
			uint32_t height = file->header.height;
			uint32_t mult;
			switch (file->header.alpha_type)
			{
				case 0:
					mult = 8;
					break;
				case 1:
					mult = 16;
					break;
				case 7:
					mult = 16;
					break;
				default:
					goto err;
			}
			for (uint32_t i = 0; i < file->mipmaps_nb; ++i)
			{
				struct wow_blp_mipmap *mipmap = &file->mipmaps[i];
				mipmap->width = width;
				mipmap->height = height;
				mipmap->data_len = ((width + 3) / 4) * ((height + 3) / 4) * mult;
				mipmap->data = WOW_MALLOC(mipmap->data_len);
				if (!mipmap->data)
					goto err;
				if (wow_mpq_seek(mpq, file->header.mipmap_offsets[i], SEEK_SET) == -1)
					goto err;
				if (wow_mpq_read(mpq, mipmap->data, file->header.mipmap_lengths[i]) != file->header.mipmap_lengths[i])
					goto err;
				width /= 2;
				height /= 2;
				if (width == 0)
					width = 1;
				if (height == 0)
					height = 1;
			}
			break;
		}
		case 3:
		{
			for (uint32_t i = 0; i < file->mipmaps_nb; ++i)
			{
				struct wow_blp_mipmap *mipmap = &file->mipmaps[i];
				mipmap->width = file->header.width / (1 << i);
				mipmap->height = file->header.height / (1 << i);
				mipmap->data_len = file->header.mipmap_lengths[i];
				mipmap->data = WOW_MALLOC(mipmap->data_len);
				if (!mipmap->data)
					goto err;
				if (wow_mpq_seek(mpq, file->header.mipmap_offsets[i], SEEK_SET) == -1)
					goto err;
				if (wow_mpq_read(mpq, mipmap->data, file->header.mipmap_lengths[i]) != file->header.mipmap_lengths[i])
					goto err;
			}
			break;
		}
	}
	return file;

err:
	wow_blp_file_delete(file);
	return NULL;
}

void wow_blp_file_delete(struct wow_blp_file *file)
{
	if (!file)
		return;
	if (file->mipmaps)
	{
		for (uint32_t i = 0; i < file->mipmaps_nb; ++i)
			WOW_FREE(file->mipmaps[i].data);
		WOW_FREE(file->mipmaps);
	}
	WOW_FREE(file);
}

#define RGB5TO8(v) ((((v) * 527) + 23) >> 6)
#define RGB6TO8(v) ((((v) * 259) + 33) >> 6)

static void unpack_bc1_block(uint32_t bx, uint32_t by,
                             uint32_t width, uint32_t height,
                             const uint8_t *in, uint8_t *out)
{
	(void)height;
	uint32_t idx = (by * width + bx) * 4;
	uint32_t idx_inc = width * 4 - 16;
	uint32_t color_bits = in[4] | (in[5] << 8) | (in[6] << 16) | ((uint32_t)in[7] << 24);
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
			switch (color_bits & 3)
			{
				case 0:
					out[idx++] = r1;
					out[idx++] = g1;
					out[idx++] = b1;
					out[idx++] = 0xFF;
					break;
				case 1:
					out[idx++] = r2;
					out[idx++] = g2;
					out[idx++] = b2;
					out[idx++] = 0xFF;
					break;
				case 2:
					if (color1 > color2)
					{
						out[idx++] = (2 * r1 + r2) / 3;
						out[idx++] = (2 * g1 + g2) / 3;
						out[idx++] = (2 * b1 + b2) / 3;
						out[idx++] = 0xFF;
					}
					else
					{
						out[idx++] = (r1 + r2) / 2;
						out[idx++] = (g1 + g2) / 2;
						out[idx++] = (b1 + b2) / 2;
						out[idx++] = 0xFF;
					}
					break;
				case 3:
					if (color1 > color2)
					{
						out[idx++] = (2 * r2 + r1) / 3;
						out[idx++] = (2 * g2 + g1) / 3;
						out[idx++] = (2 * b2 + b1) / 3;
						out[idx++] = 0xFF;
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
			color_bits >>= 2;
		}
		idx += idx_inc;
	}
}

static void unpack_bc1(uint32_t width, uint32_t height,
                       const uint8_t *in, uint8_t *out)
{
	uint32_t bx = (width + 3) & (~3);
	uint32_t by = (height + 3) & (~3);
	for (uint32_t y = 0; y < by; y += 4)
	{
		for (uint32_t x = 0; x < bx; x += 4)
		{
			unpack_bc1_block(x, y, width, height, in, out);
			in += 8;
		}
	}
}

static void unpack_bc2_block(uint32_t bx, uint32_t by,
                             uint32_t width, uint32_t height,
                             const uint8_t *in, uint8_t *out)
{
	(void)height;
	uint32_t idx = (by * width + bx) * 4;
	uint32_t idx_inc = width * 4 - 16;
	uint32_t color_bits = in[12] | (in[13] << 8) | (in[14] << 16) | ((uint32_t)in[15] << 24);
	uint16_t color1 = (in[9] << 8) | in[8];
	uint8_t r1 = RGB5TO8(in[9] >> 3);
	uint8_t g1 = RGB6TO8((color1 >> 5) & 0x3F);
	uint8_t b1 = RGB5TO8(in[8] & 0x1F);
	uint16_t color2 = (in[11] << 8) | in[10];
	uint8_t r2 = RGB5TO8(in[11] >> 3);
	uint8_t g2 = RGB6TO8((color2 >> 5) & 0x3F);
	uint8_t b2 = RGB5TO8(in[10] & 0x1F);
	uint32_t alpha_idx = 0;
	for (uint32_t y = 0; y < 4; ++y)
	{
		for (uint32_t x = 0; x < 4; ++x)
		{
			switch (color_bits & 3)
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
			color_bits >>= 2;
			if (x & 1)
			{
				out[idx] = in[alpha_idx++] & 0xF0;
				out[idx] |= out[idx] >> 4;
			}
			else
			{
				out[idx] = in[alpha_idx] & 0x0F;
				out[idx] |= out[idx] << 4;
			}
			idx++;
		}
		idx += idx_inc;
	}
}

static void unpack_bc2(uint32_t width, uint32_t height,
                       const uint8_t *in, uint8_t *out)
{
	uint32_t bx = (width + 3) & (~3);
	uint32_t by = (height + 3) & (~3);
	for (uint32_t y = 0; y < by; y += 4)
	{
		for (uint32_t x = 0; x < bx; x += 4)
		{
			unpack_bc2_block(x, y, width, height, in, out);
			in += 16;
		}
	}
}

static void unpack_bc3_block(uint32_t bx, uint32_t by,
                             uint32_t width, uint32_t height,
                             const uint8_t *in, uint8_t *out)
{
	(void)height;
	uint32_t idx = (by * width + bx) * 4;
	uint32_t idx_inc = width * 4 - 16;
	uint32_t color_bits = in[12] | (in[13] << 8) | (in[14] << 16) | ((uint32_t)in[15] << 24);
	uint16_t color1 = (in[9] << 8) | in[8];
	uint8_t r1 = RGB5TO8(in[9] >> 3);
	uint8_t g1 = RGB6TO8((color1 >> 5) & 0x3F);
	uint8_t b1 = RGB5TO8(in[8] & 0x1F);
	uint16_t color2 = (in[11] << 8) | in[10];
	uint8_t r2 = RGB5TO8(in[11] >> 3);
	uint8_t g2 = RGB6TO8((color2 >> 5) & 0x3F);
	uint8_t b2 = RGB5TO8(in[10] & 0x1F);
	uint32_t alpha_bits1 = (in[4] << 16) | (in[3] << 8) | in[2];
	uint32_t alpha_bits2 = (in[7] << 16) | (in[6] << 8) | in[5];
	uint8_t alphas[8];
	alphas[0] = in[0];
	alphas[1] = in[1];
	if (alphas[0] > alphas[1])
	{
		alphas[2] = (6 * alphas[0] + 1 * alphas[1]) / 7;
		alphas[3] = (5 * alphas[0] + 2 * alphas[1]) / 7;
		alphas[4] = (4 * alphas[0] + 3 * alphas[1]) / 7;
		alphas[5] = (3 * alphas[0] + 4 * alphas[1]) / 7;
		alphas[6] = (2 * alphas[0] + 5 * alphas[1]) / 7;
		alphas[7] = (1 * alphas[0] + 6 * alphas[1]) / 7;
	}
	else
	{
		alphas[2] = (4 * alphas[0] + 1 * alphas[1]) / 5;
		alphas[3] = (3 * alphas[0] + 2 * alphas[1]) / 5;
		alphas[4] = (2 * alphas[0] + 3 * alphas[1]) / 5;
		alphas[5] = (1 * alphas[0] + 4 * alphas[1]) / 5;
		alphas[6] = 0;
		alphas[7] = 0xFF;
	}
	for (uint32_t y = 0; y < 4; ++y)
	{
		for (uint32_t x = 0; x < 4; ++x)
		{
			switch (color_bits & 3)
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
			color_bits >>= 2;
			if (y < 2)
			{
				out[idx++] = alphas[alpha_bits1 & 7];
				alpha_bits1 >>= 3;
			}
			else
			{
				out[idx++] = alphas[alpha_bits2 & 7];
				alpha_bits2 >>= 3;
			}
		}
		idx += idx_inc;
	}
}

static void unpack_bc3(uint32_t width, uint32_t height,
                       const uint8_t *in, uint8_t *out)
{
	uint32_t bx = (width + 3) & (~3);
	uint32_t by = (height + 3) & (~3);
	for (uint32_t y = 0; y < by; y += 4)
	{
		for (uint32_t x = 0; x < bx; x += 4)
		{
			unpack_bc3_block(x, y, width, height, in, out);
			in += 16;
		}
	}
}

static void unpack_bc4_block(uint32_t bx, uint32_t by,
                             uint32_t width, uint32_t height,
                             const uint8_t *in, uint8_t *out)
{
	(void)height;
	uint32_t idx = by * width + bx;
	uint32_t idx_inc = width - 4;
	uint32_t red_bits1 = (in[4] << 16) | (in[3] << 8) | in[2];
	uint32_t red_bits2 = (in[7] << 16) | (in[6] << 8) | in[5];
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
	for (uint32_t y = 0; y < 4; ++y)
	{
		for (uint32_t x = 0; x < 4; ++x)
		{
			if (y < 2)
			{
				out[idx++] = reds[red_bits1 & 7];
				red_bits1 >>= 3;
			}
			else
			{
				out[idx++] = reds[red_bits2 & 7];
				red_bits2 >>= 3;
			}
		}
		idx += idx_inc;
	}
}

static void unpack_bc4(uint32_t width, uint32_t height,
                       const uint8_t *in, uint8_t *out)
{
	uint32_t bx = (width + 3) & (~3);
	uint32_t by = (height + 3) & (~3);
	for (uint32_t y = 0; y < by; y += 4)
	{
		for (uint32_t x = 0; x < bx; x += 4)
		{
			unpack_bc4_block(x, y, width, height, in, out);
			in += 8;
		}
	}
}

static void unpack_bc5_block(uint32_t bx, uint32_t by,
                             uint32_t width, uint32_t height,
                             const uint8_t *in, uint8_t *out)
{
	(void)height;
	uint32_t idx = (by * width + bx) * 2;
	uint32_t idx_inc = width * 2 - 8;
	uint32_t red_bits1 = (in[0x4] << 16) | (in[0x3] << 8) | in[0x2];
	uint32_t red_bits2 = (in[0x7] << 16) | (in[0x6] << 8) | in[0x5];
	uint8_t reds[8];
	reds[0] = in[0x0];
	reds[1] = in[0x1];
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
	uint32_t green_bits1 = (in[0xC] << 16) | (in[0xB] << 8) | in[0xA];
	uint32_t green_bits2 = (in[0xF] << 16) | (in[0xE] << 8) | in[0xD];
	uint8_t greens[8];
	greens[0] = in[0x8];
	greens[1] = in[0x9];
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
				out[idx++] = reds[red_bits1 & 7];
				red_bits1 >>= 3;
				out[idx++] = greens[green_bits1 & 7];
				green_bits1 >>= 3;
			}
			else
			{
				out[idx++] = reds[red_bits2 & 7];
				red_bits2 >>= 3;
				out[idx++] = greens[green_bits2 & 7];
				green_bits2 >>= 3;
			}
		}
		idx += idx_inc;
	}
}

static void unpack_bc5(uint32_t width, uint32_t height,
                       const uint8_t *in, uint8_t *out)
{
	uint32_t bx = (width + 3) & (~3);
	uint32_t by = (height + 3) & (~3);
	for (uint32_t y = 0; y < by; y += 4)
	{
		for (uint32_t x = 0; x < bx; x += 4)
		{
			unpack_bc5_block(x, y, width, height, in, out);
			in += 16;
		}
	}
}

bool wow_blp_decode_rgba(const struct wow_blp_file *file, uint8_t mipmap_id,
                         uint32_t *width, uint32_t *height, uint8_t **data)
{
	if (file->header.type != 1)
		return false;
	if (mipmap_id >= file->mipmaps_nb)
		return false;
	const struct wow_blp_mipmap *mipmap = &file->mipmaps[mipmap_id];
	*width = mipmap->width;
	*height = mipmap->height;
	switch (file->header.compression)
	{
		case 1:
		{
			*data = WOW_MALLOC(*width * *height * 4);
			if (!*data)
				return false;
			const uint8_t *indexes = mipmap->data;
			const uint8_t *alphas = indexes + *width * *height;
			uint32_t idx = 0;
			for (uint32_t i = 0; i < *width * *height; ++i)
			{
				uint32_t p = file->header.palette[indexes[i]];
				uint8_t *r = &(*data)[idx++];
				uint8_t *g = &(*data)[idx++];
				uint8_t *b = &(*data)[idx++];
				uint8_t *a = &(*data)[idx++];
				*r = p >> 16;
				*g = p >> 8;
				*b = p >> 0;
				switch (file->header.alpha_depth)
				{
					case 0:
						*a = 0xFF;
						break;
					case 1:
						*a = ((alphas[i / 8] >> (i % 8)) & 1) * 0xFF;
						break;
					case 4:
						*a = ((alphas[i / 2] >> ((i % 2) * 4)) & 0xF);
						*a |= *a << 4;
						break;
					case 8:
						*a = alphas[i];
						break;
					default:
						*a = 0xFF;
						break;
				}
			}
			break;
		}
		case 2:
		{
			size_t size = *width * *height * 4;
			if (*width < 4)
				size += (4 - *width) * *height * 4;
			if (*height < 4)
				size += (4 - *height) * *width * 4;
			switch (file->header.alpha_type)
			{
				case 0:
					*data = WOW_MALLOC(size);
					if (!*data)
						return false;
					unpack_bc1(*width, *height, mipmap->data, *data);
					break;
				case 1:
					*data = WOW_MALLOC(size);
					if (!*data)
						return false;
					unpack_bc2(*width, *height, mipmap->data, *data);
					break;
				case 7:
					*data = WOW_MALLOC(size);
					if (!*data)
						return false;
					unpack_bc3(*width, *height, mipmap->data, *data);
					break;
				default:
					return false;
			}
			break;
		}
		case 3:
		{
			*data = WOW_MALLOC(*width * *height * 4);
			if (!*data)
				return false;
			for (uint32_t i = 0; i < *width * *height * 4; i += 4)
			{
				(*data)[i + 0] = mipmap->data[i + 2];
				(*data)[i + 1] = mipmap->data[i + 1];
				(*data)[i + 2] = mipmap->data[i + 0];
				(*data)[i + 3] = mipmap->data[i + 3];
			}
			break;
		}
		default:
			return false;
	}
	return true;
}
