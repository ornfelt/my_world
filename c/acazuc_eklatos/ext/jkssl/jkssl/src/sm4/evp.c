#include "evp/cipher.h"
#include "sm4/sm4.h"

static int sm4_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	sm4_keyschedule(ctx, key);
	return 1;
}

static int sm4_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		sm4_encrypt(ctx, out, in);
	else
		sm4_decrypt(ctx, out, in);
	return 1;
}

static int sm4_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(mod_evp_name, mod_name) \
const struct evp_cipher *evp_sm4_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "SM4-" mod_name, \
		.init = sm4_init, \
		.update = sm4_update, \
		.final = sm4_final, \
		.mod = &g_evp_mod_##mod_evp_name##128, \
		.block_size = 16, \
		.key_size = 128 / 8, \
		.ctx_size = sizeof(struct sm4_ctx), \
	}; \
	return &evp; \
}

EVP_DEF(ecb, "ECB");
EVP_DEF(cbc, "CBC");
EVP_DEF(pcbc, "PCBC");
EVP_DEF(cfb, "CFB");
EVP_DEF(ofb, "OFB");
EVP_DEF(ctr, "CTR");
EVP_DEF(gcm, "GCM");
