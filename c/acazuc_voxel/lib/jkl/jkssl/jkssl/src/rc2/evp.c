#include "evp/cipher.h"
#include "rc2/rc2.h"

#define EVP_DEF(mod_evp_name, mod_name) \
const struct evp_cipher *evp_rc2_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "RC2-" mod_name, \
		.init = (evp_cipher_init_t)rc2_init, \
		.update = (evp_cipher_update_t)rc2_update, \
		.final = (evp_cipher_final_t)rc2_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = 16, \
		.ctx_size = sizeof(struct rc2_ctx), \
	}; \
	return &evp; \
}

EVP_DEF(ecb, "ECB");
EVP_DEF(cbc, "CBC");
EVP_DEF(pcbc, "PCBC");
EVP_DEF(cfb, "CFB");
EVP_DEF(ofb, "OFB");
EVP_DEF(ctr, "CTR");

int rc2_init(struct rc2_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	rc2_key_expand(ctx, key);
	return 1;
}

int rc2_update(struct rc2_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		rc2_encrypt(ctx, out, in);
	else
		rc2_decrypt(ctx, out, in);
	return 1;
}

int rc2_final(struct rc2_ctx *ctx)
{
	(void)ctx;
	return 1;
}
