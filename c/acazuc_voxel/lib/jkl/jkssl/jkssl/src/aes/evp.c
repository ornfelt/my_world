#include "evp/cipher.h"
#include "aes/aes.h"

#define EVP_DEF(size, mod_evp_name, mod_name) \
const struct evp_cipher *evp_aes_##size##_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "AES-" #size "-" mod_name, \
		.init = (evp_cipher_init_t)aes##size##_init, \
		.update = (evp_cipher_update_t)aes##size##_update, \
		.final = (evp_cipher_final_t)aes##size##_final, \
		.mod = &g_evp_mod_##mod_evp_name##128, \
		.block_size = 16, \
		.key_size = size / 8, \
		.ctx_size = sizeof(struct aes_ctx), \
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

int aes128_init(struct aes_ctx *ctx, const uint8_t *key,
                       const uint8_t *iv)
{
	(void)iv;
	aes_keyschedule(ctx, key, 16);
	return 1;
}

int aes128_update(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		aes_encrypt(ctx, out, in);
	else
		aes_decrypt(ctx, out, in);
	return 1;
}

int aes128_final(struct aes_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int aes192_init(struct aes_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	aes_keyschedule(ctx, key, 24);
	return 1;
}

int aes192_update(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		aes_encrypt(ctx, out, in);
	else
		aes_decrypt(ctx, out, in);
	return 1;
}

int aes192_final(struct aes_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int aes256_init(struct aes_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	aes_keyschedule(ctx, key, 32);
	return 1;
}

int aes256_update(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		aes_encrypt(ctx, out, in);
	else
		aes_decrypt(ctx, out, in);
	return 1;
}

int aes256_final(struct aes_ctx *ctx)
{
	(void)ctx;
	return 1;
}
