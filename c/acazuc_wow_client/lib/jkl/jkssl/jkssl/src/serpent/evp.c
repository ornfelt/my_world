#include "serpent/serpent.h"
#include "evp/cipher.h"

#define EVP_DEF(size, mod_evp_name, mod_name) \
const struct evp_cipher *evp_serpent_##size##_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "SERPENT-" #size "-" mod_name, \
		.init = (evp_cipher_init_t)serpent##size##_init, \
		.update = (evp_cipher_update_t)serpent##size##_update, \
		.final = (evp_cipher_final_t)serpent##size##_final, \
		.mod = &g_evp_mod_##mod_evp_name##128, \
		.block_size = 16, \
		.key_size = size / 8, \
		.ctx_size = sizeof(struct serpent_ctx), \
	}; \
	return &evp; \
}

EVP_DEF(128, ecb, "ECB");
EVP_DEF(128, cbc, "CBC");
EVP_DEF(128, pcbc, "PCBC");
EVP_DEF(128, cfb, "CFB");
EVP_DEF(128, ofb, "OFB");
EVP_DEF(128, ctr, "CTR");
EVP_DEF(128, gcm, "GCM");

EVP_DEF(192, ecb, "ECB");
EVP_DEF(192, cbc, "CBC");
EVP_DEF(192, pcbc, "PCBC");
EVP_DEF(192, cfb, "CFB");
EVP_DEF(192, ofb, "OFB");
EVP_DEF(192, ctr, "CTR");
EVP_DEF(192, gcm, "GCM");

EVP_DEF(256, ecb, "ECB");
EVP_DEF(256, cbc, "CBC");
EVP_DEF(256, pcbc, "PCBC");
EVP_DEF(256, cfb, "CFB");
EVP_DEF(256, ofb, "OFB");
EVP_DEF(256, ctr, "CTR");
EVP_DEF(256, gcm, "GCM");

int serpent128_init(struct serpent_ctx *ctx, const uint8_t *key,
                    const uint8_t *iv)
{
	(void)iv;
	serpent_keyschedule(ctx, key, 16);
	return 1;
}

int serpent128_update(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in,
                      int enc)
{
	if (enc)
		serpent_encrypt(ctx, out, in);
	else
		serpent_decrypt(ctx, out, in);
	return 1;
}

int serpent128_final(struct serpent_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int serpent192_init(struct serpent_ctx *ctx, const uint8_t *key,
                    const uint8_t *iv)
{
	(void)iv;
	serpent_keyschedule(ctx, key, 24);
	return 1;
}

int serpent192_update(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in,
                      int enc)
{
	if (enc)
		serpent_encrypt(ctx, out, in);
	else
		serpent_decrypt(ctx, out, in);
	return 1;
}

int serpent192_final(struct serpent_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int serpent256_init(struct serpent_ctx *ctx, const uint8_t *key,
                    const uint8_t *iv)
{
	(void)iv;
	serpent_keyschedule(ctx, key, 32);
	return 1;
}

int serpent256_update(struct serpent_ctx *ctx, uint8_t *out, const uint8_t *in,
                      int enc)
{
	if (enc)
		serpent_encrypt(ctx, out, in);
	else
		serpent_decrypt(ctx, out, in);
	return 1;
}

int serpent256_final(struct serpent_ctx *ctx)
{
	(void)ctx;
	return 1;
}
