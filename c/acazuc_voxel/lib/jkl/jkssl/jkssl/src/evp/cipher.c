#include "utils/utils.h"
#include "evp/cipher.h"

#include <string.h>
#include <stdlib.h>

struct evp_cipher_ctx *evp_cipher_ctx_new(void)
{
	return calloc(sizeof(struct evp_cipher_ctx), 1);
}

void evp_cipher_ctx_free(struct evp_cipher_ctx *ctx)
{
	if (!ctx)
		return;
	free(ctx->buf);
	free(ctx->mod1);
	free(ctx->mod2);
	free(ctx->ctx);
	free(ctx);
}

int evp_cipher_init(struct evp_cipher_ctx *ctx,
                    const struct evp_cipher *evp_cipher,
                    const uint8_t *key, const uint8_t *iv, int enc)
{
	ctx->enc = enc;
	ctx->buf_pos = 0;
	ctx->ended = 0;
	if (ctx->evp_cipher != evp_cipher)
	{
		free(ctx->mod1);
		free(ctx->mod2);
		free(ctx->buf);
		free(ctx->ctx);
		ctx->mod1 = malloc(evp_cipher->block_size);
		ctx->mod2 = malloc(evp_cipher->block_size);
		ctx->buf = malloc(evp_cipher->block_size);
		ctx->ctx = malloc(evp_cipher->ctx_size);
		if (!ctx->mod1
		 || !ctx->mod2
		 || !ctx->buf
		 || !ctx->ctx)
			goto err;
		ctx->evp_cipher = evp_cipher;
	}
	if (!ctx->evp_cipher->init(ctx->ctx, key, iv))
		goto err;
	if (evp_cipher->mod->flags & EVP_CIPHER_MOD_FLAG_IV)
		memcpy(ctx->mod1, iv, evp_cipher->block_size);
	return 1;

err:
	free(ctx->mod1);
	free(ctx->mod2);
	free(ctx->buf);
	free(ctx->ctx);
	ctx->mod1 = NULL;
	ctx->mod2 = NULL;
	ctx->buf = NULL;
	ctx->ctx = NULL;
	return 0;
}

int evp_cipher_update(struct evp_cipher_ctx *ctx, uint8_t *out,
                      size_t *outl, const uint8_t *in, size_t inl)
{
	size_t org = *outl;
	while (*outl >= ctx->evp_cipher->block_size
	    && inl + ctx->buf_pos >= ctx->evp_cipher->block_size + (ctx->enc ? 0 : 1))
	{
		const uint8_t *buf;
		size_t tmp = ctx->evp_cipher->block_size;
		if (ctx->buf_pos)
		{
			tmp -= ctx->buf_pos;
			buf = ctx->buf;
			memcpy(&ctx->buf[ctx->buf_pos], in, tmp);
			ctx->buf_pos = 0;
		}
		else
		{
			buf = in;
		}
		if (!ctx->evp_cipher->mod->update(ctx, out, buf))
			return 0;
		out += ctx->evp_cipher->block_size;
		*outl -= ctx->evp_cipher->block_size;
		in += tmp;
		inl -= tmp;
	}
	if (inl)
	{
		if (inl > ctx->evp_cipher->block_size - ctx->buf_pos)
			return 0;
		memcpy(ctx->buf + ctx->buf_pos, in, inl);
		ctx->buf_pos += inl;
	}
	*outl = org - *outl;
	return 1;
}

int evp_cipher_final(struct evp_cipher_ctx *ctx, uint8_t *out,
                     size_t *outl)
{
	if (ctx->ended)
	{
		*outl = 0;
		return 1;
	}
	if (ctx->enc && !(ctx->evp_cipher->mod->flags & EVP_CIPHER_MOD_FLAG_NOPAD))
	{
		uint8_t off = ctx->evp_cipher->block_size - ctx->buf_pos;
		memset(ctx->buf + ctx->buf_pos, off, off);
		ctx->buf_pos = ctx->evp_cipher->block_size;
	}
	ctx->ended = 1;
	int ret = ctx->evp_cipher->mod->update(ctx, ctx->buf, ctx->buf);
	if (!ctx->enc && !(ctx->evp_cipher->mod->flags & EVP_CIPHER_MOD_FLAG_NOPAD))
	{
		if (!ctx->buf_pos)
			return 0;
		if (ctx->buf[ctx->buf_pos - 1] > ctx->evp_cipher->block_size)
			ret = 0;
		ctx->buf_pos -= ctx->buf[ctx->buf_pos - 1];
	}
	if (ret)
	{
		if (*outl < ctx->buf_pos)
			return 0;
		memcpy(out, ctx->buf, ctx->buf_pos);
		*outl = ctx->buf_pos;
	}
	else
	{
		*outl = 0;
	}
	return 1;
}

void evp_foreach_cipher(int (*cb)(const struct evp_cipher *cipher,
                                  void *data),
                        void *data)
{
#define CIPHER_FOREACH(c) \
do \
{ \
	if (!cb(evp_##c(), data)) \
		return; \
} while (0)

	CIPHER_FOREACH(des_ecb);
	CIPHER_FOREACH(des_cbc);
	CIPHER_FOREACH(des_pcbc);
	CIPHER_FOREACH(des_cfb);
	CIPHER_FOREACH(des_ofb);
	CIPHER_FOREACH(des_ctr);
	CIPHER_FOREACH(des_ede_ecb);
	CIPHER_FOREACH(des_ede_cbc);
	CIPHER_FOREACH(des_ede_pcbc);
	CIPHER_FOREACH(des_ede_cfb);
	CIPHER_FOREACH(des_ede_ofb);
	CIPHER_FOREACH(des_ede_ctr);
	CIPHER_FOREACH(des_ede3_ecb);
	CIPHER_FOREACH(des_ede3_cbc);
	CIPHER_FOREACH(des_ede3_pcbc);
	CIPHER_FOREACH(des_ede3_cfb);
	CIPHER_FOREACH(des_ede3_ofb);
	CIPHER_FOREACH(des_ede3_ctr);
	CIPHER_FOREACH(aes_128_ecb);
	CIPHER_FOREACH(aes_128_cbc);
	CIPHER_FOREACH(aes_128_pcbc);
	CIPHER_FOREACH(aes_128_cfb);
	CIPHER_FOREACH(aes_128_ofb);
	CIPHER_FOREACH(aes_128_ctr);
	CIPHER_FOREACH(aes_128_gcm);
	CIPHER_FOREACH(aes_192_ecb);
	CIPHER_FOREACH(aes_192_cbc);
	CIPHER_FOREACH(aes_192_pcbc);
	CIPHER_FOREACH(aes_192_cfb);
	CIPHER_FOREACH(aes_192_ofb);
	CIPHER_FOREACH(aes_192_ctr);
	CIPHER_FOREACH(aes_192_gcm);
	CIPHER_FOREACH(aes_256_ecb);
	CIPHER_FOREACH(aes_256_cbc);
	CIPHER_FOREACH(aes_256_pcbc);
	CIPHER_FOREACH(aes_256_cfb);
	CIPHER_FOREACH(aes_256_ofb);
	CIPHER_FOREACH(aes_256_ctr);
	CIPHER_FOREACH(aes_256_gcm);
	CIPHER_FOREACH(camellia_128_ecb);
	CIPHER_FOREACH(camellia_128_cbc);
	CIPHER_FOREACH(camellia_128_pcbc);
	CIPHER_FOREACH(camellia_128_cfb);
	CIPHER_FOREACH(camellia_128_ofb);
	CIPHER_FOREACH(camellia_128_ctr);
	CIPHER_FOREACH(camellia_128_gcm);
	CIPHER_FOREACH(camellia_192_ecb);
	CIPHER_FOREACH(camellia_192_cbc);
	CIPHER_FOREACH(camellia_192_pcbc);
	CIPHER_FOREACH(camellia_192_cfb);
	CIPHER_FOREACH(camellia_192_ofb);
	CIPHER_FOREACH(camellia_192_ctr);
	CIPHER_FOREACH(camellia_192_gcm);
	CIPHER_FOREACH(camellia_256_ecb);
	CIPHER_FOREACH(camellia_256_cbc);
	CIPHER_FOREACH(camellia_256_pcbc);
	CIPHER_FOREACH(camellia_256_cfb);
	CIPHER_FOREACH(camellia_256_ofb);
	CIPHER_FOREACH(camellia_256_ctr);
	CIPHER_FOREACH(camellia_256_gcm);
	CIPHER_FOREACH(aria_128_ecb);
	CIPHER_FOREACH(aria_128_cbc);
	CIPHER_FOREACH(aria_128_pcbc);
	CIPHER_FOREACH(aria_128_cfb);
	CIPHER_FOREACH(aria_128_ofb);
	CIPHER_FOREACH(aria_128_ctr);
	CIPHER_FOREACH(aria_128_gcm);
	CIPHER_FOREACH(aria_192_ecb);
	CIPHER_FOREACH(aria_192_cbc);
	CIPHER_FOREACH(aria_192_pcbc);
	CIPHER_FOREACH(aria_192_cfb);
	CIPHER_FOREACH(aria_192_ofb);
	CIPHER_FOREACH(aria_192_ctr);
	CIPHER_FOREACH(aria_192_gcm);
	CIPHER_FOREACH(aria_256_ecb);
	CIPHER_FOREACH(aria_256_cbc);
	CIPHER_FOREACH(aria_256_pcbc);
	CIPHER_FOREACH(aria_256_cfb);
	CIPHER_FOREACH(aria_256_ofb);
	CIPHER_FOREACH(aria_256_ctr);
	CIPHER_FOREACH(aria_256_gcm);
	CIPHER_FOREACH(chacha20);
	CIPHER_FOREACH(rc2_ecb);
	CIPHER_FOREACH(rc2_cbc);
	CIPHER_FOREACH(rc2_pcbc);
	CIPHER_FOREACH(rc2_cfb);
	CIPHER_FOREACH(rc2_ofb);
	CIPHER_FOREACH(rc4);
	CIPHER_FOREACH(seed_ecb);
	CIPHER_FOREACH(seed_cbc);
	CIPHER_FOREACH(seed_pcbc);
	CIPHER_FOREACH(seed_cfb);
	CIPHER_FOREACH(seed_ofb);
	CIPHER_FOREACH(seed_ctr);
	CIPHER_FOREACH(seed_gcm);
	CIPHER_FOREACH(bf_ecb);
	CIPHER_FOREACH(bf_cbc);
	CIPHER_FOREACH(bf_pcbc);
	CIPHER_FOREACH(bf_cfb);
	CIPHER_FOREACH(bf_ofb);
	CIPHER_FOREACH(cast5_ecb);
	CIPHER_FOREACH(cast5_cbc);
	CIPHER_FOREACH(cast5_pcbc);
	CIPHER_FOREACH(cast5_cfb);
	CIPHER_FOREACH(cast5_ofb);
	CIPHER_FOREACH(serpent_128_ecb);
	CIPHER_FOREACH(serpent_128_cbc);
	CIPHER_FOREACH(serpent_128_pcbc);
	CIPHER_FOREACH(serpent_128_cfb);
	CIPHER_FOREACH(serpent_128_ofb);
	CIPHER_FOREACH(serpent_128_ctr);
	CIPHER_FOREACH(serpent_128_gcm);
	CIPHER_FOREACH(serpent_192_ecb);
	CIPHER_FOREACH(serpent_192_cbc);
	CIPHER_FOREACH(serpent_192_pcbc);
	CIPHER_FOREACH(serpent_192_cfb);
	CIPHER_FOREACH(serpent_192_ofb);
	CIPHER_FOREACH(serpent_192_ctr);
	CIPHER_FOREACH(serpent_192_gcm);
	CIPHER_FOREACH(serpent_256_ecb);
	CIPHER_FOREACH(serpent_256_cbc);
	CIPHER_FOREACH(serpent_256_pcbc);
	CIPHER_FOREACH(serpent_256_cfb);
	CIPHER_FOREACH(serpent_256_ofb);
	CIPHER_FOREACH(serpent_256_ctr);
	CIPHER_FOREACH(serpent_256_gcm);
	CIPHER_FOREACH(sm4_ecb);
	CIPHER_FOREACH(sm4_cbc);
	CIPHER_FOREACH(sm4_pcbc);
	CIPHER_FOREACH(sm4_cfb);
	CIPHER_FOREACH(sm4_ofb);
	CIPHER_FOREACH(sm4_ctr);
	CIPHER_FOREACH(sm4_gcm);

#undef CIPHER_FOREACH
}

struct cipher_get_state
{
	const char *name;
	const struct evp_cipher *cipher;
};

static int cipher_get_foreach(const struct evp_cipher *cipher, void *data)
{
	struct cipher_get_state *state = data;
	if (!strcasecmp(cipher->name, state->name))
	{
		state->cipher = cipher;
		return 0;
	}
	return 1;
}

const struct evp_cipher *evp_get_cipherbyname(const char *name)
{
	struct cipher_get_state state;
	state.name = name;
	state.cipher = NULL;
	evp_foreach_cipher(cipher_get_foreach, &state);
	return state.cipher;
}

size_t evp_cipher_get_block_size(const struct evp_cipher *evp_cipher)
{
	return evp_cipher->block_size;
}

size_t evp_cipher_get_key_length(const struct evp_cipher *evp_cipher)
{
	return evp_cipher->key_size;
}

size_t evp_cipher_get_iv_length(const struct evp_cipher *evp_cipher)
{
	return evp_cipher->block_size; /* XXX */
}

const char *evp_cipher_get0_name(const struct evp_cipher *evp_cipher)
{
	return evp_cipher->name;
}

#define MOD_CBC(size) \
static int cbc##size##_update(struct evp_cipher_ctx *ctx, uint8_t *out, \
                              const uint8_t *in) \
{ \
	if (ctx->enc) \
	{ \
		memxor(out, in, ctx->mod1, size / 8); \
		if (!ctx->evp_cipher->update(ctx->ctx, out, out, ctx->enc)) \
			return 0; \
		memcpy(ctx->mod1, out, size / 8); \
	} \
	else \
	{ \
		memcpy(ctx->mod2, in, size / 8); \
		if (!ctx->evp_cipher->update(ctx->ctx, out, in, ctx->enc)) \
			return 0; \
		memxor(out, out, ctx->mod1, size / 8); \
		memcpy(ctx->mod1, ctx->mod2, size / 8); \
	} \
	return 1; \
} \
const struct evp_cipher_mod g_evp_mod_cbc##size = \
{ \
	.update = cbc##size##_update, \
	.flags = EVP_CIPHER_MOD_FLAG_IV, \
}

MOD_CBC(64);
MOD_CBC(128);

#define MOD_CFB(size) \
static int cfb##size##_update(struct evp_cipher_ctx *ctx, uint8_t *out, \
                              const uint8_t *in) \
{ \
	memcpy(ctx->mod2, in, size / 8); \
	memcpy(out, ctx->mod1, size / 8); \
	if (!ctx->evp_cipher->update(ctx->ctx, out, out, 1)) \
		return 0; \
	memxor(out, out, ctx->mod2, size / 8); \
	if (ctx->enc) \
		memcpy(ctx->mod1, out, size / 8); \
	else \
		memcpy(ctx->mod1, ctx->mod2, size / 8); \
	return 1; \
} \
const struct evp_cipher_mod g_evp_mod_cfb##size = \
{ \
	.update = cfb##size##_update, \
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD | EVP_CIPHER_MOD_FLAG_IV, \
}

MOD_CFB(64);
MOD_CFB(128);

static int ctr128_update(struct evp_cipher_ctx *ctx, uint8_t *out,
                         const uint8_t *ion)
{
	memcpy(ctx->mod2, out, 16);
	memcpy(out, ctx->mod1, 16);
	if (!ctx->evp_cipher->update(ctx->ctx, out, out, 1))
		return 0;
	memcpy(ctx->mod1, out, 16);
	be64enc(&ctx->mod1[8], be64dec(&ctx->mod1[8]) + 1);
	memxor(out, out, ctx->mod2, 16);
	return 1;
}

const struct evp_cipher_mod g_evp_mod_ctr128 =
{
	.update = ctr128_update,
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD | EVP_CIPHER_MOD_FLAG_IV,
};

static int ctr64_update(struct evp_cipher_ctx *ctx, uint8_t *out,
                        const uint8_t *in)
{
	memcpy(ctx->mod2, in, 8);
	memcpy(out, ctx->mod1, 8);
	if (!ctx->evp_cipher->update(ctx->ctx, out, out, 1))
		return 0;
	memcpy(ctx->mod1, out, 8);
	be64enc(&ctx->mod1[8], be64dec(&ctx->mod1[8]) + 1);
	memxor(out, out, ctx->mod2, 8);
	return 1;
}

const struct evp_cipher_mod g_evp_mod_ctr64 =
{
	.update = ctr64_update,
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD | EVP_CIPHER_MOD_FLAG_IV,
};

static int ecb_update(struct evp_cipher_ctx *ctx, uint8_t *out,
                      const uint8_t *in)
{
	return ctx->evp_cipher->update(ctx->ctx, out, in, ctx->enc);
}

const struct evp_cipher_mod g_evp_mod_ecb64 =
{
	.update = ecb_update,
	.flags = 0,
};

const struct evp_cipher_mod g_evp_mod_ecb128 =
{
	.update = ecb_update,
	.flags = 0,
};

const struct evp_cipher_mod g_evp_mod_ecb_iv =
{
	.update = ecb_update,
	.flags = EVP_CIPHER_MOD_FLAG_IV,
};

const struct evp_cipher_mod g_evp_mod_ecb_nopad =
{
	.update = ecb_update,
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD,
};

const struct evp_cipher_mod g_evp_mod_ecb_nopad_iv =
{
	.update = ecb_update,
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD | EVP_CIPHER_MOD_FLAG_IV,
};

static int gcm128_update(struct evp_cipher_ctx *ctx, uint8_t *out,
                         const uint8_t *in)
{
	memcpy(ctx->mod2, in, 16);
	memcpy(out, ctx->mod1, 16);
	if (!ctx->evp_cipher->update(ctx->ctx, out, out, 1))
		return 0;
	memcpy(ctx->mod1, out, 16);
	be64enc(ctx->mod1 + 8, be64dec(ctx->mod1 + 8) + 1);
	memxor(out, out, ctx->mod2, 16);
	return 1;
}

const struct evp_cipher_mod g_evp_mod_gcm128 =
{
	.update = gcm128_update,
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD | EVP_CIPHER_MOD_FLAG_IV,
};

#define MOD_OFB(size) \
static int ofb##size##_update(struct evp_cipher_ctx *ctx, uint8_t *out, \
                              const uint8_t *in) \
{ \
	memcpy(ctx->mod2, in, size / 8); \
	memcpy(out, ctx->mod1, size / 8); \
	if (!ctx->evp_cipher->update(ctx->ctx, out, out, 1)) \
		return 0; \
	memcpy(ctx->mod1, out, size / 8); \
	memxor(out, out, ctx->mod2, size / 8); \
	return 1; \
} \
const struct evp_cipher_mod g_evp_mod_ofb##size = \
{ \
	.update = ofb##size##_update, \
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD | EVP_CIPHER_MOD_FLAG_IV, \
}

MOD_OFB(64);
MOD_OFB(128);

#define MOD_PCBC(size) \
static int pcbc##size##_update(struct evp_cipher_ctx *ctx, uint8_t *out, \
                               const uint8_t *in) \
{ \
	memcpy(ctx->mod2, in, size / 8); \
	if (ctx->enc) \
		memxor(out, in, ctx->mod1, size / 8); \
	if (!ctx->evp_cipher->update(ctx->ctx, out, out, ctx->enc)) \
		return 0; \
	if (!ctx->enc) \
		memxor(out, out, ctx->mod1, size / 8); \
	memxor(ctx->mod1, ctx->mod2, out, size / 8); \
	return 1; \
} \
const struct evp_cipher_mod g_evp_mod_pcbc##size = \
{ \
	.update = pcbc##size##_update, \
	.flags = EVP_CIPHER_MOD_FLAG_NOPAD | EVP_CIPHER_MOD_FLAG_IV, \
}

MOD_PCBC(64);
MOD_PCBC(128);
