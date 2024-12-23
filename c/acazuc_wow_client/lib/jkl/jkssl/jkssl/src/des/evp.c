#include "evp/cipher.h"
#include "des.h"

#define EVP_DEF(size, evp, evp_name, mod_evp_name, mod_name) \
const struct evp_cipher *evp_##evp##_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = evp_name "-" mod_name, \
		.init = (evp_cipher_init_t)evp##_init, \
		.update = (evp_cipher_update_t)evp##_update, \
		.final = (evp_cipher_final_t)evp##_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = size, \
		.ctx_size = sizeof(struct evp##_ctx), \
	}; \
	return &evp; \
}

EVP_DEF(8, des, "DES", ecb, "ECB");
EVP_DEF(8, des, "DES", cbc, "CBC");
EVP_DEF(8, des, "DES", pcbc, "PCBC");
EVP_DEF(8, des, "DES", cfb, "CFB");
EVP_DEF(8, des, "DES", ofb, "OFB");
EVP_DEF(8, des, "DES", ctr, "CTR");

EVP_DEF(16, des_ede, "DES-EDE", ecb, "ECB");
EVP_DEF(16, des_ede, "DES-EDE", cbc, "CBC");
EVP_DEF(16, des_ede, "DES-EDE", pcbc, "PCBC");
EVP_DEF(16, des_ede, "DES-EDE", cfb, "CFB");
EVP_DEF(16, des_ede, "DES-EDE", ofb, "OFB");
EVP_DEF(16, des_ede, "DES-EDE", ctr, "CTR");

EVP_DEF(24, des_ede3, "DES-EDE3", ecb, "ECB");
EVP_DEF(24, des_ede3, "DES-EDE3", cbc, "CBC");
EVP_DEF(24, des_ede3, "DES-EDE3", pcbc, "PCBC");
EVP_DEF(24, des_ede3, "DES-EDE3", cfb, "CFB");
EVP_DEF(24, des_ede3, "DES-EDE3", ofb, "OFB");
EVP_DEF(24, des_ede3, "DES-EDE3", ctr, "CTR");

int des_init(struct des_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	des_generate_keys(ctx, key);
	return 1;
}

int des_update(struct des_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	des_operate_block(ctx, out, in, enc);
	return 1;
}

int des_final(struct des_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int des_ede_init(struct des_ede_ctx *ctx, const uint8_t *key,
                        const uint8_t *iv)
{
	(void)iv;
	des_generate_keys(&ctx->ctx1, key);
	des_generate_keys(&ctx->ctx2, key + 8);
	return 1;
}

int des_ede_update(struct des_ede_ctx *ctx, uint8_t *out, const uint8_t *in,
                   int enc)
{
	uint8_t tmp1[8];
	uint8_t tmp2[8];
	des_operate_block(&ctx->ctx1, tmp1, in, enc);
	des_operate_block(&ctx->ctx2, tmp2, tmp1, !enc);
	des_operate_block(&ctx->ctx1, out, tmp2, enc);
	return 1;
}

int des_ede_final(struct des_ede_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int des_ede3_init(struct des_ede3_ctx *ctx, const uint8_t *key,
                  const uint8_t *iv)
{
	(void)iv;
	des_generate_keys(&ctx->ctx1, key);
	des_generate_keys(&ctx->ctx2, key + 8);
	des_generate_keys(&ctx->ctx3, key + 16);
	return 1;
}

int des_ede3_update(struct des_ede3_ctx *ctx, uint8_t *out, const uint8_t *in,
                    int enc)
{
	uint8_t tmp1[8];
	uint8_t tmp2[8];
	if (enc)
	{
		des_operate_block(&ctx->ctx1, tmp1, in, 1);
		des_operate_block(&ctx->ctx2, tmp2, tmp1, 0);
		des_operate_block(&ctx->ctx3, out, tmp2, 1);
	}
	else
	{
		des_operate_block(&ctx->ctx3, tmp1, in, 0);
		des_operate_block(&ctx->ctx2, tmp2, tmp1, 1);
		des_operate_block(&ctx->ctx1, out, tmp2, 0);
	}
	return 1;
}

int des_ede3_final(struct des_ede3_ctx *ctx)
{
	(void)ctx;
	return 1;
}
