#include "evp/cipher.h"
#include "cast/cast.h"

#define EVP_DEF(mod_evp_name, mod_name) \
const struct evp_cipher *evp_cast5_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "CAST5-" mod_name, \
		.init = (evp_cipher_init_t)cast_init, \
		.update = (evp_cipher_update_t)cast_update, \
		.final = (evp_cipher_final_t)cast_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = 16, \
		.ctx_size = sizeof(struct cast_ctx), \
	}; \
	return &evp; \
}

EVP_DEF(ecb, "ECB");
EVP_DEF(cbc, "CBC");
EVP_DEF(pcbc, "PCBC");
EVP_DEF(cfb, "CFB");
EVP_DEF(ofb, "OFB");
EVP_DEF(ctr, "CTR");

int cast_init(struct cast_ctx *ctx, const uint8_t *key,
                     const uint8_t *iv)
{
	(void)iv;
	cast_keyschedule(ctx, key, 16);
	return 1;
}

int cast_update(struct cast_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		cast_encrypt(ctx, out, in);
	else
		cast_decrypt(ctx, out, in);
	return 1;
}

int cast_final(struct cast_ctx *ctx)
{
	(void)ctx;
	return 1;
}
