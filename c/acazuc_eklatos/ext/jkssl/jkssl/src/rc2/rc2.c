#include "utils/utils.h"
#include "rc2/rc2.h"

#include <string.h>

static const uint8_t pitable[256] =
{
	0xD9, 0x78, 0xF9, 0xC4, 0x19, 0xDD, 0xB5, 0xED,
	0x28, 0xE9, 0xFD, 0x79, 0x4A, 0xA0, 0xD8, 0x9D,
	0xC6, 0x7E, 0x37, 0x83, 0x2B, 0x76, 0x53, 0x8E,
	0x62, 0x4C, 0x64, 0x88, 0x44, 0x8B, 0xFB, 0xA2,
	0x17, 0x9A, 0x59, 0xF5, 0x87, 0xB3, 0x4F, 0x13,
	0x61, 0x45, 0x6D, 0x8D, 0x09, 0x81, 0x7D, 0x32,
	0xBD, 0x8F, 0x40, 0xEB, 0x86, 0xB7, 0x7B, 0x0B,
	0xF0, 0x95, 0x21, 0x22, 0x5C, 0x6B, 0x4E, 0x82,
	0x54, 0xD6, 0x65, 0x93, 0xCE, 0x60, 0xB2, 0x1C,
	0x73, 0x56, 0xC0, 0x14, 0xA7, 0x8C, 0xF1, 0xDC,
	0x12, 0x75, 0xCA, 0x1F, 0x3B, 0xBE, 0xE4, 0xD1,
	0x42, 0x3D, 0xD4, 0x30, 0xA3, 0x3C, 0xB6, 0x26,
	0x6F, 0xBF, 0x0E, 0xDA, 0x46, 0x69, 0x07, 0x57,
	0x27, 0xF2, 0x1D, 0x9B, 0xBC, 0x94, 0x43, 0x03,
	0xF8, 0x11, 0xC7, 0xF6, 0x90, 0xEF, 0x3E, 0xE7,
	0x06, 0xC3, 0xD5, 0x2F, 0xC8, 0x66, 0x1E, 0xD7,
	0x08, 0xE8, 0xEA, 0xDE, 0x80, 0x52, 0xEE, 0xF7,
	0x84, 0xAA, 0x72, 0xAC, 0x35, 0x4D, 0x6A, 0x2A,
	0x96, 0x1A, 0xD2, 0x71, 0x5A, 0x15, 0x49, 0x74,
	0x4B, 0x9F, 0xD0, 0x5E, 0x04, 0x18, 0xA4, 0xEC,
	0xC2, 0xE0, 0x41, 0x6E, 0x0F, 0x51, 0xCB, 0xCC,
	0x24, 0x91, 0xAF, 0x50, 0xA1, 0xF4, 0x70, 0x39,
	0x99, 0x7C, 0x3A, 0x85, 0x23, 0xB8, 0xB4, 0x7A,
	0xFC, 0x02, 0x36, 0x5B, 0x25, 0x55, 0x97, 0x31,
	0x2D, 0x5D, 0xFA, 0x98, 0xE3, 0x8A, 0x92, 0xAE,
	0x05, 0xDF, 0x29, 0x10, 0x67, 0x6C, 0xBA, 0xC9,
	0xD3, 0x00, 0xE6, 0xCF, 0xE1, 0x9E, 0xA8, 0x2C,
	0x63, 0x16, 0x01, 0x3F, 0x58, 0xE2, 0x89, 0xA9,
	0x0D, 0x38, 0x34, 0x1B, 0xAB, 0x33, 0xFF, 0xB0,
	0xBB, 0x48, 0x0C, 0x5F, 0xB9, 0xB1, 0xCD, 0x2E,
	0xC5, 0xF3, 0xDB, 0x47, 0xE5, 0xA5, 0x9C, 0x77,
	0x0A, 0xA6, 0x20, 0x68, 0xFE, 0x7F, 0xC1, 0xAD,
};

void rc2_key_expand(struct rc2_ctx *ctx, const uint8_t *key)
{
	uint8_t l[128];
	memcpy(l, key, 16);
	for (size_t i = 16; i < 128; ++i)
		l[i] = pitable[(l[i - 1] + l[i - 16]) & 0xFF];
	l[128 - 16] = pitable[l[128 - 16]];
	for (size_t i = 127; i >= 16; --i)
	{
		size_t tmp = i - 16;
		l[tmp] = pitable[l[tmp + 1] ^ l[i]];
	}
	for (size_t i = 0; i < 64; ++i)
		ctx->k[i] = l[i * 2] | (l[i * 2 + 1] << 8);
}

static inline void mash_round(struct rc2_ctx *ctx, uint16_t *r)
{
	r[0] += ctx->k[r[3] & 63];
	r[1] += ctx->k[r[0] & 63];
	r[2] += ctx->k[r[1] & 63];
	r[3] += ctx->k[r[2] & 63];
}

static inline void mix_round(const uint16_t *k, uint16_t *r)
{
	r[0] += k[0] + (r[3] & r[2]) + (~r[3] & r[1]);
	r[0] = rol16(r[0], 1);
	r[1] += k[1] + (r[0] & r[3]) + (~r[0] & r[2]);
	r[1] = rol16(r[1], 2);
	r[2] += k[2] + (r[1] & r[0]) + (~r[1] & r[3]);
	r[2] = rol16(r[2], 3);
	r[3] += k[3] + (r[2] & r[1]) + (~r[2] & r[0]);
	r[3] = rol16(r[3], 5);
}

void rc2_encrypt(struct rc2_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	uint16_t r[4];
	r[0] = le16dec(&in[0x0]);
	r[1] = le16dec(&in[0x2]);
	r[2] = le16dec(&in[0x4]);
	r[3] = le16dec(&in[0x6]);
	mix_round(&ctx->k[0x00], r);
	mix_round(&ctx->k[0x04], r);
	mix_round(&ctx->k[0x08], r);
	mix_round(&ctx->k[0x0C], r);
	mix_round(&ctx->k[0x10], r);
	mash_round(ctx, r);
	mix_round(&ctx->k[0x14], r);
	mix_round(&ctx->k[0x18], r);
	mix_round(&ctx->k[0x1C], r);
	mix_round(&ctx->k[0x20], r);
	mix_round(&ctx->k[0x24], r);
	mix_round(&ctx->k[0x28], r);
	mash_round(ctx, r);
	mix_round(&ctx->k[0x2C], r);
	mix_round(&ctx->k[0x30], r);
	mix_round(&ctx->k[0x34], r);
	mix_round(&ctx->k[0x38], r);
	mix_round(&ctx->k[0x3C], r);
	le16enc(&out[0x0], r[0]);
	le16enc(&out[0x2], r[1]);
	le16enc(&out[0x4], r[2]);
	le16enc(&out[0x6], r[3]);
}

static inline void rmix_round(const uint16_t *k, uint16_t *r)
{
	r[3] = ror16(r[3], 5);
	r[3] -= k[3] + (r[2] & r[1]) + (~r[2] & r[0]);
	r[2] = ror16(r[2], 3);
	r[2] -= k[2] + (r[1] & r[0]) + (~r[1] & r[3]);
	r[1] = ror16(r[1], 2);
	r[1] -= k[1] + (r[0] & r[3]) + (~r[0] & r[2]);
	r[0] = ror16(r[0], 1);
	r[0] -= k[0] + (r[3] & r[2]) + (~r[3] & r[1]);
}

static inline void rmash_round(struct rc2_ctx *ctx, uint16_t *r)
{
	r[3] -= ctx->k[r[2] & 63];
	r[2] -= ctx->k[r[1] & 63];
	r[1] -= ctx->k[r[0] & 63];
	r[0] -= ctx->k[r[3] & 63];
}

void rc2_decrypt(struct rc2_ctx *ctx, uint8_t *out, const uint8_t *in)
{
	uint16_t r[4];
	r[0] = le16dec(&in[0x0]);
	r[1] = le16dec(&in[0x2]);
	r[2] = le16dec(&in[0x4]);
	r[3] = le16dec(&in[0x6]);
	rmix_round(&ctx->k[0x3C], r);
	rmix_round(&ctx->k[0x38], r);
	rmix_round(&ctx->k[0x34], r);
	rmix_round(&ctx->k[0x30], r);
	rmix_round(&ctx->k[0x2C], r);
	rmash_round(ctx, r);
	rmix_round(&ctx->k[0x28], r);
	rmix_round(&ctx->k[0x24], r);
	rmix_round(&ctx->k[0x20], r);
	rmix_round(&ctx->k[0x1C], r);
	rmix_round(&ctx->k[0x18], r);
	rmix_round(&ctx->k[0x14], r);
	rmash_round(ctx, r);
	rmix_round(&ctx->k[0x10], r);
	rmix_round(&ctx->k[0x0C], r);
	rmix_round(&ctx->k[0x08], r);
	rmix_round(&ctx->k[0x04], r);
	rmix_round(&ctx->k[0x00], r);
	le16enc(&out[0x0], r[0]);
	le16enc(&out[0x2], r[1]);
	le16enc(&out[0x4], r[2]);
	le16enc(&out[0x6], r[3]);
}
