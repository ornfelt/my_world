#include "evp/cipher.h"
#include "rc5/rc5.h"
#include "oid/oid.h"

static int rc5_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	rc5_keyschedule(ctx, key);
	return 1;
}

static int rc5_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		rc5_encrypt(ctx, out, in);
	else
		rc5_decrypt(ctx, out, in);
	return 1;
}

static int rc5_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(mod_evp_name, mod_name, oid_value, oid_size_value) \
const struct evp_cipher *evp_rc5_32_12_16_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "RC5-" mod_name, \
		.init = rc5_init, \
		.update = rc5_update, \
		.final = rc5_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = 16, \
		.ctx_size = sizeof(struct rc5_ctx), \
		.oid = oid_value, \
		.oid_size = oid_size_value, \
	}; \
	return &evp; \
}

#define EVP_DEF_OID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, oid_rc5_##mod_evp_name, \
	        sizeof(oid_rc5_##mod_evp_name) / sizeof(*oid_rc5_##mod_evp_name))

#define EVP_DEF_NOOID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, NULL, 0)

EVP_DEF_NOOID(ecb, "ECB");
EVP_DEF_OID(cbc, "CBC");
EVP_DEF_NOOID(pcbc, "PCBC");
EVP_DEF_NOOID(cfb, "CFB");
EVP_DEF_NOOID(ofb, "OFB");
EVP_DEF_NOOID(ctr, "CTR");
