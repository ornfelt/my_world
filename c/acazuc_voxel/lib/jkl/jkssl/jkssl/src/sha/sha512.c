#include "utils/utils.h"
#include "sha/sha.h"

#include <string.h>

static const uint64_t k[128] =
{
	0x428A2F98D728AE22, 0x7137449123EF65CD,
	0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC,
	0x3956C25BF348B538, 0x59F111F1B605D019,
	0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118,
	0xD807AA98A3030242, 0x12835B0145706FBE,
	0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2,
	0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1,
	0x9BDC06A725C71235, 0xC19BF174CF692694,
	0xE49B69C19EF14AD2, 0xEFBE4786384F25E3,
	0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
	0x2DE92C6F592B0275, 0x4A7484AA6EA6E483,
	0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5,
	0x983E5152EE66DFAB, 0xA831C66D2DB43210,
	0xB00327C898FB213F, 0xBF597FC7BEEF0EE4,
	0xC6E00BF33DA88FC2, 0xD5A79147930AA725,
	0x06CA6351E003826F, 0x142929670A0E6E70,
	0x27B70A8546D22FFC, 0x2E1B21385C26C926,
	0x4D2C6DFC5AC42AED, 0x53380D139D95B3DF,
	0x650A73548BAF63DE, 0x766A0ABB3C77B2A8,
	0x81C2C92E47EDAEE6, 0x92722C851482353B,
	0xA2BFE8A14CF10364, 0xA81A664BBC423001,
	0xC24B8B70D0F89791, 0xC76C51A30654BE30,
	0xD192E819D6EF5218, 0xD69906245565A910,
	0xF40E35855771202A, 0x106AA07032BBD1B8,
	0x19A4C116B8D2D0C8, 0x1E376C085141AB53,
	0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8,
	0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB,
	0x5B9CCA4F7763E373, 0x682E6FF3D6B2B8A3,
	0x748F82EE5DEFB2FC, 0x78A5636F43172F60,
	0x84C87814A1F0AB72, 0x8CC702081A6439EC,
	0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9,
	0xBEF9A3F7B2C67915, 0xC67178F2E372532B,
	0xCA273ECEEA26619C, 0xD186B8C721C0C207,
	0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178,
	0x06F067AA72176FBA, 0x0A637DC5A2C898A6,
	0x113F9804BEF90DAE, 0x1B710B35131C471B,
	0x28DB77F523047D84, 0x32CAAB7B40C72493,
	0x3C9EBE0A15C9BEBC, 0x431D67C49C100D4C,
	0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A,
	0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817,
};

static void loop(int i, uint64_t *tmp, uint64_t *w)
{
	uint64_t tmp1 = ror64(tmp[4], 14)
	              ^ ror64(tmp[4], 18)
	              ^ ror64(tmp[4], 41);
	uint64_t tmp2 = (tmp[4] & tmp[5]) ^ ((~tmp[4]) & tmp[6]);
	uint64_t tmp3 = tmp[7] + tmp1 + tmp2 + k[i] + w[i];
	tmp1 = ror64(tmp[0], 28)
	     ^ ror64(tmp[0], 34)
	     ^ ror64(tmp[0], 39);
	tmp2 = (tmp[0] & tmp[1]) ^ (tmp[0] & tmp[2]) ^ (tmp[1] & tmp[2]);
	uint64_t tmp4 = tmp1 + tmp2;
	tmp[7] = tmp[6];
	tmp[6] = tmp[5];
	tmp[5] = tmp[4];
	tmp[4] = tmp[3] + tmp3;
	tmp[3] = tmp[2];
	tmp[2] = tmp[1];
	tmp[1] = tmp[0];
	tmp[0] = tmp3 + tmp4;
}

static void chunk(struct sha512_ctx *ctx, const uint8_t *data)
{
	uint64_t tmp[8];
	uint64_t w[80];
	int i;

	for (i = 0; i < 16; ++i)
		w[i] = be64dec(&data[i * 8]);
	for (; i < 80; ++i)
		w[i] = w[i - 16] + (ror64(w[i - 15], 1)
		                  ^ ror64(w[i - 15], 8)
		                  ^ (w[i - 15] >> 7))
		                 + w[i - 7]
		                 + (ror64(w[i - 2], 19)
		                  ^ ror64(w[i - 2], 61)
		                  ^ (w[i - 2] >> 6));
	for (i = 0; i < 8; ++i)
		tmp[i] = ctx->h[i];
	for (i = 0; i < 80; ++i)
		loop(i, tmp, w);
	for (i = 0; i < 8; ++i)
		ctx->h[i] += tmp[i];
}

int sha512_init(struct sha512_ctx *ctx)
{
	ctx->total_size = 0;
	ctx->data_size = 0;
	ctx->h[0] = 0x6A09E667F3BCC908;
	ctx->h[1] = 0xBB67AE8584CAA73B;
	ctx->h[2] = 0x3C6EF372FE94F82B;
	ctx->h[3] = 0xA54FF53A5F1D36F1;
	ctx->h[4] = 0x510E527FADE682D1;
	ctx->h[5] = 0x9B05688C2B3E6C1F;
	ctx->h[6] = 0x1F83D9ABFB41BD6B;
	ctx->h[7] = 0x5BE0CD19137E2179;
	return 1;
}

int sha512_update(struct sha512_ctx *ctx, const void *data, size_t size)
{
	HASH_BUFFERIZE(ctx, data, size, 128);
	return 1;
}

int sha512_final(uint8_t *md, struct sha512_ctx *ctx)
{
	MERKLE_DAMGARD_FINALIZE(ctx, 128, 1);
	for (int i = 0; i < 8; ++i)
		be64enc(&md[i * 8], ctx->h[i]);
	return 1;
}

int sha512(const uint8_t *data, size_t len, uint8_t *md)
{
	struct sha512_ctx ctx;
	if (!sha512_init(&ctx)
	 || !sha512_update(&ctx, data, len)
	 || !sha512_final(md, &ctx))
		return 0;
	return 1;
}
