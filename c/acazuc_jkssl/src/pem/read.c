#include "utils/utils.h"
#include "pem/pem.h"

#include <jkssl/evp.h>
#include <jkssl/bio.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static int output(struct pem_read_ctx *ctx, const uint8_t *data, size_t len)
{
	uint8_t *tmp;

	if (!len)
		return 1;
	tmp = malloc(len + ctx->len);
	if (!tmp)
		return 0;
	memcpy(tmp, ctx->data, ctx->len);
	memcpy(tmp + ctx->len, data, len);
	free(ctx->data);
	ctx->data = tmp;
	ctx->len += len;
	return 1;
}

static int read_line(struct bio *bio, char *data, int in_data)
{
	ssize_t r = bio_gets(bio, data, 67);
	if (r <= 0)
		return -1;
	if (in_data && r > 65)
	{
		if (data[64] != '\r' || data[65] != '\n')
			return -1;
		data[64] = '\n';
	}
	char *tmp = strchr(data, '\n');
	if (tmp)
		*tmp = '\0';
	return strlen(data);
}

static int read_file_data(struct pem_read_ctx *ctx, char *line, int line_len)
{
	struct evp_cipher_ctx *cipher_ctx = NULL;
	struct evp_encode_ctx *b64_ctx = NULL;
	uint8_t b64_out[64];
	size_t b64_outl;
	uint8_t cipher_out[64];
	size_t cipher_outl;
	char end_text[64];
	uint8_t *key = NULL;
	int ret = 0;

	if (ctx->text && snprintf(end_text, sizeof(end_text),
	                          "-----END %s-----",
	                          ctx->text) >= (int)sizeof(end_text))
		return 0;
	b64_ctx = evp_encode_ctx_new();
	if (!b64_ctx)
		goto end;
	evp_decode_init(b64_ctx);
	if (ctx->evp_cipher)
	{
		key = malloc(evp_cipher_get_key_length(ctx->evp_cipher));
		if (!key)
			goto end;
		if (ctx->salt_iv_len != (int)JKSSL_MAX(8, evp_cipher_get_iv_length(ctx->evp_cipher)))
			goto end;
		char buf[512];
		char *password;
		if (ctx->password_cb)
		{
			if (ctx->password_cb(buf, sizeof(buf), 0, ctx->password_userdata) < 0)
				goto end;
			password = buf;
		}
		else if (ctx->password_userdata)
		{
			password = ctx->password_userdata;
		}
		else
		{
			goto end;
		}
		if (!pem_get_key(ctx->evp_cipher, key, ctx->salt_iv, password))
			goto end;
		cipher_ctx = evp_cipher_ctx_new();
		if (!cipher_ctx)
			goto end;
		if (!evp_cipher_init(cipher_ctx, ctx->evp_cipher, key, ctx->salt_iv, 0))
			goto end;
	}
	while (line_len > 0)
	{
		if (ctx->text)
		{
			if (!strcmp(line, end_text))
				break;
		}
		else if (!strncmp(line, "-----END ", 9))
		{
			break;
		}
		b64_outl = sizeof(b64_out);
		if (!evp_decode_update(b64_ctx, b64_out, &b64_outl,
		                       (uint8_t*)line, line_len))
			goto end;
		if (ctx->evp_cipher)
		{
			cipher_outl = sizeof(cipher_out);
			if (!evp_cipher_update(cipher_ctx, cipher_out,
			                       &cipher_outl, b64_out, b64_outl)
			 || !output(ctx, cipher_out, cipher_outl))
				goto end;
		}
		else
		{
			if (!output(ctx, b64_out, b64_outl))
				goto end;
		}
		line_len = read_line(ctx->bio, line, 1);
	}
	b64_outl = sizeof(b64_out);
	if (evp_decode_final(b64_ctx, b64_out, &b64_outl) == -1)
		goto end;
	if (ctx->evp_cipher)
	{
		cipher_outl = sizeof(cipher_out);
		if (!evp_cipher_final(cipher_ctx, cipher_out, &cipher_outl)
		 || !output(ctx, cipher_out, cipher_outl))
			goto end;
	}
	else
	{
		if (!output(ctx, b64_out, b64_outl))
			goto end;
	}
	ret = 1;

end:
	free(key);
	evp_encode_ctx_free(b64_ctx);
	evp_cipher_ctx_free(cipher_ctx);
	return ret;
}

static int read_salt_iv(struct pem_read_ctx *ctx, char *line, int line_len)
{
	char *coma;
	int iv_len;

	if (line_len < 11)
		return 0;
	if (memcmp(line, "DEK-Info: ", 10))
		return 0;
	line += 10;
	coma = strchr(line, ',');
	if (!coma)
		return 0;
	*coma = '\0';
	ctx->evp_cipher = evp_get_cipherbyname(line);
	if (!ctx->evp_cipher)
		return 0;
	iv_len = strlen(coma + 1);
	if (!iv_len || iv_len & 1)
		return 0;
	ctx->salt_iv_len = iv_len / 2;
	ctx->salt_iv = malloc(ctx->salt_iv_len);
	if (!ctx->salt_iv)
		return 0;
	if (!hex2bin(ctx->salt_iv, coma + 1, iv_len))
		return 0;
	return 1;
}

static int find_begin_line(struct pem_read_ctx *ctx, struct bio *bio,
                           const char *text)
{
	ssize_t line_len;
	char begin_text[65];
	char line[67];

	if (text && snprintf(begin_text, sizeof(begin_text),
	                     "-----BEGIN %s-----",
	                     text) >= (int)sizeof(begin_text))
		return 0;
	while (1)
	{
		line_len = read_line(bio, line, 0);
		if (line_len < 0)
			return 0;
		if (text)
		{
			if (!strcmp(line, begin_text))
				return 1;
		}
		else if (!strncmp(line, "-----BEGIN ", 11))
		{
			char *end = strstr(&line[11], "-----");
			if (!end)
			{
				/* XXX skip until EOL */
				continue;
			}
			/* line cannot be more than 64 chars, so we're okay */
			strncpy(ctx->matched_text, &line[11], end - &line[11]);
			ctx->matched_text[end - &line[11]] = '\0';
			return 1;
		}
		/* XXX skip until EOL */
	}
	return 0;
}

static int pem_read_body(struct pem_read_ctx *ctx)
{
	char line[67];
	int line_len;
	int ret = 0;

	ctx->evp_cipher = NULL;
	ctx->data = NULL;
	ctx->salt_iv = NULL;
	ctx->len = 0;
	line_len = read_line(ctx->bio, line, 1);
	if (line_len <= 0)
		goto end;
	if (!strcmp(line, "Proc-Type: 4,ENCRYPTED"))
	{
		if ((line_len = read_line(ctx->bio, line, 1)) <= 0
		 || !read_salt_iv(ctx, line, line_len)
		 || (line_len = read_line(ctx->bio, line, 1)) < 0
		 || line_len > 0
		 || (line_len = read_line(ctx->bio, line, 1)) <= 0)
			goto end;
	}
	if (!read_file_data(ctx, line, line_len))
		goto end;
	ret = 1;

end:
	free(ctx->salt_iv);
	return ret;
}

int pem_read(struct pem_read_ctx *ctx)
{
	if (!find_begin_line(ctx, ctx->bio, ctx->text))
		return 0;
	return pem_read_body(ctx);
}
