#include "evp/cipher.h"
#include "des/des.h"
#include "oid/oid.h"

static int des_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	des_generate_keys(ctx, key);
	return 1;
}

static int des_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	des_operate_block(ctx, out, in, enc);
	return 1;
}

static int des_final(void *ctx)
{
	(void)ctx;
	return 1;
}

static int des_ede_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	struct des_ede_ctx *ede = ctx;
	(void)iv;
	des_generate_keys(&ede->ctx1, key);
	des_generate_keys(&ede->ctx2, key + 8);
	return 1;
}

static int des_ede_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	struct des_ede_ctx *ede = ctx;
	uint8_t tmp1[8];
	uint8_t tmp2[8];
	des_operate_block(&ede->ctx1, tmp1, in, enc);
	des_operate_block(&ede->ctx2, tmp2, tmp1, !enc);
	des_operate_block(&ede->ctx1, out, tmp2, enc);
	return 1;
}

static int des_ede_final(void *ctx)
{
	(void)ctx;
	return 1;
}

static int des_ede3_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	struct des_ede3_ctx *ede3 = ctx;
	(void)iv;
	des_generate_keys(&ede3->ctx1, key);
	des_generate_keys(&ede3->ctx2, key + 8);
	des_generate_keys(&ede3->ctx3, key + 16);
	return 1;
}

static int des_ede3_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	struct des_ede3_ctx *ede3 = ctx;
	uint8_t tmp1[8];
	uint8_t tmp2[8];
	if (enc)
	{
		des_operate_block(&ede3->ctx1, tmp1, in, 1);
		des_operate_block(&ede3->ctx2, tmp2, tmp1, 0);
		des_operate_block(&ede3->ctx3, out, tmp2, 1);
	}
	else
	{
		des_operate_block(&ede3->ctx3, tmp1, in, 0);
		des_operate_block(&ede3->ctx2, tmp2, tmp1, 1);
		des_operate_block(&ede3->ctx1, out, tmp2, 0);
	}
	return 1;
}

static int des_ede3_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(size, evp, evp_name, mod_evp_name, mod_name, oid_value, oid_size_value) \
const struct evp_cipher *evp_##evp##_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = evp_name "-" mod_name, \
		.init = evp##_init, \
		.update = evp##_update, \
		.final = evp##_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = size, \
		.ctx_size = sizeof(struct evp##_ctx), \
		.oid = oid_value, \
		.oid_size = oid_size_value, \
	}; \
	return &evp; \
}

#define EVP_DEF_OID(size, evp, evp_name, mod_evp_name, mod_name) \
	EVP_DEF(size, evp, evp_name, mod_evp_name, mod_name, oid_##evp##_##mod_evp_name, \
	        sizeof(oid_##evp##_##mod_evp_name) / sizeof(*oid_##evp##_##mod_evp_name))

#define EVP_DEF_NOOID(size, evp, evp_name, mod_evp_name, mod_name) \
	EVP_DEF(size, evp, evp_name, mod_evp_name, mod_name, NULL, 0)

EVP_DEF_OID(8, des, "DES", ecb, "ECB");
EVP_DEF_OID(8, des, "DES", cbc, "CBC");
EVP_DEF_NOOID(8, des, "DES", pcbc, "PCBC");
EVP_DEF_OID(8, des, "DES", cfb, "CFB");
EVP_DEF_OID(8, des, "DES", ofb, "OFB");
EVP_DEF_NOOID(8, des, "DES", ctr, "CTR");

EVP_DEF_NOOID(16, des_ede, "DES-EDE", ecb, "ECB");
EVP_DEF_NOOID(16, des_ede, "DES-EDE", cbc, "CBC");
EVP_DEF_NOOID(16, des_ede, "DES-EDE", pcbc, "PCBC");
EVP_DEF_NOOID(16, des_ede, "DES-EDE", cfb, "CFB");
EVP_DEF_NOOID(16, des_ede, "DES-EDE", ofb, "OFB");
EVP_DEF_NOOID(16, des_ede, "DES-EDE", ctr, "CTR");

EVP_DEF_NOOID(24, des_ede3, "DES-EDE3", ecb, "ECB");
EVP_DEF_OID(24, des_ede3, "DES-EDE3", cbc, "CBC");
EVP_DEF_NOOID(24, des_ede3, "DES-EDE3", pcbc, "PCBC");
EVP_DEF_NOOID(24, des_ede3, "DES-EDE3", cfb, "CFB");
EVP_DEF_NOOID(24, des_ede3, "DES-EDE3", ofb, "OFB");
EVP_DEF_NOOID(24, des_ede3, "DES-EDE3", ctr, "CTR");
