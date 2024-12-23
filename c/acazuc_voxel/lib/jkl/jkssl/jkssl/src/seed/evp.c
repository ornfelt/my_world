#include "evp/cipher.h"
#include "seed/seed.h"

#define EVP_DEF(mod_evp_name, mod_name) \
const struct evp_cipher *evp_seed_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "SEED-" mod_name, \
		.init = (evp_cipher_init_t)seed_init, \
		.update = (evp_cipher_update_t)seed_update, \
		.final = (evp_cipher_final_t)seed_final, \
		.mod = &g_evp_mod_##mod_evp_name##128, \
		.block_size = 16, \
		.key_size = 16, \
		.ctx_size = sizeof(struct seed_ctx), \
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

int seed_init(struct seed_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	seed_keyschedule(ctx, key);
	return 1;
}

int seed_update(struct seed_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		seed_encrypt(ctx, out, in);
	else
		seed_decrypt(ctx, out, in);
	return 1;
}

int seed_final(struct seed_ctx *ctx)
{
	(void)ctx;
	return 1;
}
