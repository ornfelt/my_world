#include "evp/cipher.h"
#include "cast/cast.h"
#include "oid/oid.h"

static int cast_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	cast_keyschedule(ctx, key, 16);
	return 1;
}

static int cast_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		cast_encrypt(ctx, out, in);
	else
		cast_decrypt(ctx, out, in);
	return 1;
}

static int cast_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(mod_evp_name, mod_name, oid_value, oid_size_value) \
const struct evp_cipher *evp_cast5_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "CAST5-" mod_name, \
		.init = cast_init, \
		.update = cast_update, \
		.final = cast_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = 16, \
		.ctx_size = sizeof(struct cast_ctx), \
		.oid = oid_value, \
		.oid_size = oid_size_value, \
	}; \
	return &evp; \
}

#define EVP_DEF_OID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, oid_cast5_##mod_evp_name, \
	        sizeof(oid_cast5_##mod_evp_name) / sizeof(*oid_cast5_##mod_evp_name))

#define EVP_DEF_NOOID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, NULL, 0)

EVP_DEF_NOOID(ecb, "ECB");
EVP_DEF_OID(cbc, "CBC");
EVP_DEF_NOOID(pcbc, "PCBC");
EVP_DEF_NOOID(cfb, "CFB");
EVP_DEF_NOOID(ofb, "OFB");
EVP_DEF_NOOID(ctr, "CTR");
