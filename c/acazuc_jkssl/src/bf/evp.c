#include "evp/cipher.h"
#include "oid/oid.h"
#include "bf/bf.h"

static int bf_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	bf_keyschedule(ctx, key, 16);
	return 1;
}

static int bf_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		bf_encrypt(ctx, out, in);
	else
		bf_decrypt(ctx, out, in);
	return 1;
}

static int bf_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(mod_evp_name, mod_name, oid_value, oid_size_value) \
const struct evp_cipher *evp_bf_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "BF-" mod_name, \
		.init = bf_init, \
		.update = bf_update, \
		.final = bf_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = 16, \
		.ctx_size = sizeof(struct bf_ctx), \
		.oid = oid_value, \
		.oid_size = oid_size_value, \
	}; \
	return &evp; \
}

#define EVP_DEF_OID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, oid_bf_##mod_evp_name, \
	        sizeof(oid_bf_##mod_evp_name) / sizeof(*oid_bf_##mod_evp_name))

#define EVP_DEF_NOOID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, NULL, 0)

EVP_DEF_OID(ecb, "ECB");
EVP_DEF_OID(cbc, "CBC");
EVP_DEF_NOOID(pcbc, "PCBC");
EVP_DEF_OID(cfb, "CFB");
EVP_DEF_OID(ofb, "OFB");
EVP_DEF_NOOID(ctr, "CTR");
