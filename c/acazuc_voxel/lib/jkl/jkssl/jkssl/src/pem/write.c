#include "utils/utils.h"
#include "pem/pem.h"

#include <jkssl/rand.h>
#include <jkssl/evp.h>
#include <jkssl/bio.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int do_write_salt_iv(const struct evp_cipher *cipher, uint8_t *salt_iv,
                            struct bio *bio)
{
	char *salt_iv_text;

	salt_iv_text = malloc(JKSSL_MAX(17, evp_cipher_get_iv_length(cipher) * 2 + 1));
	if (!salt_iv_text)
		return 0;
	bin2hex(salt_iv_text, salt_iv, evp_cipher_get_block_size(cipher));
	salt_iv_text[JKSSL_MAX(16, evp_cipher_get_block_size(cipher) * 2)] = 0;
	if (bio_printf(bio, "Proc-Type: 4,ENCRYPTED\nDEK-Info: %s, %s\n\n",
	               evp_cipher_get0_name(cipher), salt_iv_text) < 0)
	{
		free(salt_iv_text);
		return 0;
	}
	free(salt_iv_text);
	return 1;
}

static int print_ciphered(struct pem_write_ctx *ctx, struct bio *bio)
{
	struct evp_cipher_ctx *cipher_ctx;
	uint8_t *salt_iv;
	uint8_t *key;
	int ret = 0;
	size_t written = 0;
	uint8_t out[4096];
	size_t outl;
	size_t inl;
	uint8_t *in;

	cipher_ctx = evp_cipher_ctx_new();
	salt_iv = malloc(JKSSL_MAX(8, evp_cipher_get_iv_length(ctx->evp_cipher)));
	key = malloc(evp_cipher_get_key_length(ctx->evp_cipher));
	if (!cipher_ctx
	 || !salt_iv
	 || !key
	 || (!ctx->password && !(ctx->password = ask_password_confirm()))
	 || rand_bytes(salt_iv, JKSSL_MAX(8, evp_cipher_get_iv_length(ctx->evp_cipher)) != 1)
	 || !pem_get_key(ctx->evp_cipher, key, salt_iv, ctx->password)
	 || !do_write_salt_iv(ctx->evp_cipher, salt_iv, ctx->bio)
	 || !evp_cipher_init(cipher_ctx, ctx->evp_cipher, key, salt_iv, 1))
		goto end;
	in = ctx->data;
	while (written < ctx->len)
	{
		inl = ctx->len;
		outl = sizeof(out);
		if (inl > outl)
			inl = outl;
		if (!evp_cipher_update(cipher_ctx, out, &outl, in, inl)
		 || bio_write(bio, out, outl) != (ssize_t)outl)
			goto end;
		in += inl;
		written += inl;
	}
	outl = sizeof(out);
	if (!evp_cipher_final(cipher_ctx, out, &outl)
	 || bio_write(bio, out, outl) != (ssize_t)outl)
		goto end;
	ret = 1;

end:
	evp_cipher_ctx_free(cipher_ctx);
	free(salt_iv);
	free(key);
	return ret;
}

int pem_write(struct pem_write_ctx *ctx)
{
	struct bio *b64;
	int ret = 0;

	b64 = bio_new(bio_f_base64());
	if (!b64)
		return 0;
	bio_push(b64, ctx->bio);
	if (ctx->text)
		bio_printf(ctx->bio, "-----BEGIN %s-----\n", ctx->text);
	if (ctx->evp_cipher)
	{
		if (!print_ciphered(ctx, b64))
			goto end;
	}
	else
	{
		if (bio_write(b64, ctx->data, ctx->len) != (ssize_t)ctx->len)
			goto end;
	}
	if (!bio_flush(b64))
		goto end;
	if (ctx->text)
		bio_printf(ctx->bio, "-----END %s-----\n", ctx->text);
	if (!bio_flush(ctx->bio))
		goto end;
	ret = 1;

end:
	bio_free(b64);
	return ret;
}
