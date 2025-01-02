#include "evp/cipher.h"
#include "rc2/rc2.h"
#include "oid/oid.h"

static int rc2_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	rc2_key_expand(ctx, key);
	return 1;
}

static int rc2_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		rc2_encrypt(ctx, out, in);
	else
		rc2_decrypt(ctx, out, in);
	return 1;
}

static int rc2_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(mod_evp_name, mod_name, oid_value, oid_size_value) \
const struct evp_cipher *evp_rc2_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "RC2-" mod_name, \
		.init = rc2_init, \
		.update = rc2_update, \
		.final = rc2_final, \
		.mod = &g_evp_mod_##mod_evp_name##64, \
		.block_size = 8, \
		.key_size = 16, \
		.ctx_size = sizeof(struct rc2_ctx), \
		.oid = oid_value, \
		.oid_size = oid_size_value, \
	}; \
	return &evp; \
}

#define EVP_DEF_OID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, oid_rc2_##mod_evp_name, \
	        sizeof(oid_rc2_##mod_evp_name) / sizeof(*oid_rc2_##mod_evp_name))

#define EVP_DEF_NOOID(mod_evp_name, mod_name) \
	EVP_DEF(mod_evp_name, mod_name, NULL, 0)

EVP_DEF_OID(ecb, "ECB");
EVP_DEF_OID(cbc, "CBC");
EVP_DEF_NOOID(pcbc, "PCBC");
EVP_DEF_NOOID(cfb, "CFB");
EVP_DEF_NOOID(ofb, "OFB");
EVP_DEF_NOOID(ctr, "CTR");
