#include "evp/cipher.h"
#include "bf/bf.h"

#define EVP_DEF(mod_evp_name, mod_name) \
const struct evp_cipher *evp_bf_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "BF-" mod_name, \
		.init = (evp_cipher_init_t)bf_init, \
		.update = (evp_cipher_update_t)bf_update, \
		.final = (evp_cipher_final_t)bf_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = 16, \
		.ctx_size = sizeof(struct bf_ctx), \
	}; \
	return &evp; \
}

EVP_DEF(ecb, "ECB");
EVP_DEF(cbc, "CBC");
EVP_DEF(pcbc, "PCBC");
EVP_DEF(cfb, "CFB");
EVP_DEF(ofb, "OFB");
EVP_DEF(ctr, "CTR");

int bf_init(struct bf_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	bf_keyschedule(ctx, key, 16);
	return 1;
}

int bf_update(struct bf_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		bf_encrypt(ctx, out, in);
	else
		bf_decrypt(ctx, out, in);
	return 1;
}

int bf_final(struct bf_ctx *ctx)
{
	(void)ctx;
	return 1;
}
