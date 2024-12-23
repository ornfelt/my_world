#include "serpent/serpent.h"
#include "evp/cipher.h"
#include "oid/oid.h"

static int serpent128_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	serpent_keyschedule(ctx, key, 16);
	return 1;
}

static int serpent128_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		serpent_encrypt(ctx, out, in);
	else
		serpent_decrypt(ctx, out, in);
	return 1;
}

static int serpent128_final(void *ctx)
{
	(void)ctx;
	return 1;
}

static int serpent192_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	serpent_keyschedule(ctx, key, 24);
	return 1;
}

static int serpent192_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		serpent_encrypt(ctx, out, in);
	else
		serpent_decrypt(ctx, out, in);
	return 1;
}

static int serpent192_final(void *ctx)
{
	(void)ctx;
	return 1;
}

static int serpent256_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	serpent_keyschedule(ctx, key, 32);
	return 1;
}

static int serpent256_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		serpent_encrypt(ctx, out, in);
	else
		serpent_decrypt(ctx, out, in);
	return 1;
}

static int serpent256_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(size, mod_evp_name, mod_name, oid_value, oid_size_value) \
const struct evp_cipher *evp_serpent_##size##_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "SERPENT-" #size "-" mod_name, \
		.init = serpent##size##_init, \
		.update = serpent##size##_update, \
		.final = serpent##size##_final, \
		.mod = &g_evp_mod_##mod_evp_name##128, \
		.block_size = 16, \
		.key_size = size / 8, \
		.ctx_size = sizeof(struct serpent_ctx), \
		.oid = oid_value, \
		.oid_size = oid_size_value, \
	}; \
	return &evp; \
}

#define EVP_DEF_OID(size, mod_evp_name, mod_name) \
	EVP_DEF(size, mod_evp_name, mod_name, oid_serpent##size##_##mod_evp_name, \
	        sizeof(oid_serpent##size##_##mod_evp_name) / sizeof(*oid_serpent##size##_##mod_evp_name))

#define EVP_DEF_NOOID(size, mod_evp_name, mod_name) \
	EVP_DEF(size, mod_evp_name, mod_name, NULL, 0)

EVP_DEF_OID(128, ecb, "ECB");
EVP_DEF_OID(128, cbc, "CBC");
EVP_DEF_NOOID(128, pcbc, "PCBC");
EVP_DEF_OID(128, cfb, "CFB");
EVP_DEF_OID(128, ofb, "OFB");
EVP_DEF_NOOID(128, ctr, "CTR");
EVP_DEF_NOOID(128, gcm, "GCM");

EVP_DEF_OID(192, ecb, "ECB");
EVP_DEF_OID(192, cbc, "CBC");
EVP_DEF_NOOID(192, pcbc, "PCBC");
EVP_DEF_OID(192, cfb, "CFB");
EVP_DEF_OID(192, ofb, "OFB");
EVP_DEF_NOOID(192, ctr, "CTR");
EVP_DEF_NOOID(192, gcm, "GCM");

EVP_DEF_OID(256, ecb, "ECB");
EVP_DEF_OID(256, cbc, "CBC");
EVP_DEF_NOOID(256, pcbc, "PCBC");
EVP_DEF_OID(256, cfb, "CFB");
EVP_DEF_OID(256, ofb, "OFB");
EVP_DEF_NOOID(256, ctr, "CTR");
EVP_DEF_NOOID(256, gcm, "GCM");
