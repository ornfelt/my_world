#include "evp/cipher.h"
#include "aes/aes.h"
#include "oid/oid.h"

#include <sys/auxv.h>

#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#endif

static void (*evp_aes_encrypt)(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
static void (*evp_aes_decrypt)(struct aes_ctx *ctx, uint8_t *out, const uint8_t *in);
static void (*evp_aes_keyschedule)(struct aes_ctx *ctx, const uint8_t *key, uint8_t len);

static void init_aes(void)
{
#if defined(__i386__) || defined(__x86_64__)
#ifdef ENABLE_AESNI
	uint32_t eax, ebx, ecx, edx;
	if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
	{
		if (ecx & (1 << 25))
		{
			evp_aes_keyschedule = aesni_keyschedule;
			evp_aes_encrypt = aesni_encrypt;
			evp_aes_decrypt = aesni_decrypt;
			return;
		}
	}
#endif
#endif
#ifdef ENABLE_AES_NEON
#ifdef AT_HWCAP
	unsigned long hwcap = getauxval(AT_HWCAP);
	if (0
#ifdef HWCAP_AES
	 || (hwcap & HWCAP_AES)
#endif
#ifdef HWCAP_PMULL
	 || (hwcap & HWCAP_PMULL)
#endif
	)
	{
		evp_aes_keyschedule = aes_neon_keyschedule;
		evp_aes_encrypt = aes_neon_encrypt;
		evp_aes_decrypt = aes_neon_decrypt;
		return;
	}
#endif
#endif
	evp_aes_keyschedule = aes_keyschedule;
	evp_aes_encrypt = aes_encrypt;
	evp_aes_decrypt = aes_decrypt;
}

static int aes128_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	if (!evp_aes_keyschedule)
		init_aes();
	(void)iv;
	evp_aes_keyschedule(ctx, key, 16);
	return 1;
}

static int aes128_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		evp_aes_encrypt(ctx, out, in);
	else
		evp_aes_decrypt(ctx, out, in);
	return 1;
}

static int aes128_final(void *ctx)
{
	(void)ctx;
	return 1;
}

static int aes192_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	if (!evp_aes_keyschedule)
		init_aes();
	(void)iv;
	evp_aes_keyschedule(ctx, key, 24);
	return 1;
}

static int aes192_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		evp_aes_encrypt(ctx, out, in);
	else
		evp_aes_decrypt(ctx, out, in);
	return 1;
}

static int aes192_final(void *ctx)
{
	(void)ctx;
	return 1;
}

static int aes256_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	if (!evp_aes_keyschedule)
		init_aes();
	(void)iv;
	evp_aes_keyschedule(ctx, key, 32);
	return 1;
}

static int aes256_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	if (enc)
		evp_aes_encrypt(ctx, out, in);
	else
		evp_aes_decrypt(ctx, out, in);
	return 1;
}

static int aes256_final(void *ctx)
{
	(void)ctx;
	return 1;
}

#define EVP_DEF(size, mod_evp_name, mod_name, oid_value, oid_size_value) \
const struct evp_cipher *evp_aes_##size##_##mod_evp_name(void) \
{ \
	static const struct evp_cipher evp = \
	{ \
		.name = "AES-" #size "-" mod_name, \
		.init = aes##size##_init, \
		.update = aes##size##_update, \
		.final = aes##size##_final, \
		.mod = &g_evp_mod_##mod_evp_name##128, \
		.block_size = 16, \
		.key_size = size / 8, \
		.ctx_size = sizeof(struct aes_ctx), \
		.oid = oid_value, \
		.oid_size = oid_size_value, \
	}; \
	return &evp; \
}

#define EVP_DEF_OID(size, mod_evp_name, mod_name) \
	EVP_DEF(size, mod_evp_name, mod_name, oid_aes##size##_##mod_evp_name, \
	        sizeof(oid_aes##size##_##mod_evp_name) / sizeof(*oid_aes##size##_##mod_evp_name))

#define EVP_DEF_NOOID(size, mod_evp_name, mod_name) \
	EVP_DEF(size, mod_evp_name, mod_name, NULL, 0)

EVP_DEF_OID(128, ecb, "ECB");
EVP_DEF_OID(128, cbc, "CBC");
EVP_DEF_NOOID(128, pcbc, "PCBC");
EVP_DEF_OID(128, cfb, "CFB");
EVP_DEF_OID(128, ofb, "OFB");
EVP_DEF_NOOID(128, ctr, "CTR");
EVP_DEF_OID(128, gcm, "GCM");

EVP_DEF_OID(192, ecb, "ECB");
EVP_DEF_OID(192, cbc, "CBC");
EVP_DEF_NOOID(192, pcbc, "PCBC");
EVP_DEF_OID(192, cfb, "CFB");
EVP_DEF_OID(192, ofb, "OFB");
EVP_DEF_NOOID(192, ctr, "CTR");
EVP_DEF_OID(192, gcm, "GCM");

EVP_DEF_OID(256, ecb, "ECB");
EVP_DEF_OID(256, cbc, "CBC");
EVP_DEF_NOOID(256, pcbc, "PCBC");
EVP_DEF_OID(256, cfb, "CFB");
EVP_DEF_OID(256, ofb, "OFB");
EVP_DEF_NOOID(256, ctr, "CTR");
EVP_DEF_OID(256, gcm, "GCM");
