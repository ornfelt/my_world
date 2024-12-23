#include "bio/bio.h"

#include <jkssl/evp.h>

#include <string.h>

struct bio_cipher
{
	struct evp_cipher_ctx *cipher_ctx;
	uint8_t decrypt_buf[4096];
	size_t decrypt_buf_size;
	int enc;
};

static int bio_cipher_ctr(struct bio *bio)
{
	struct bio_cipher *bio_cipher = bio->ctx;
	bio_cipher->enc = -1;
	bio_cipher->cipher_ctx = evp_cipher_ctx_new();
	if (!bio_cipher->cipher_ctx)
		return 0;
	return 1;
}

static void bio_cipher_dtr(struct bio *bio)
{
	struct bio_cipher *bio_cipher = bio->ctx;
	evp_cipher_ctx_free(bio_cipher->cipher_ctx);
}

static long bio_cipher_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                            void *arg2)
{
	(void)arg1;
	struct bio_cipher *bio_cipher = bio->ctx;
	switch (cmd)
	{
		case BIO_CTRL_FLUSH:
			if (bio_cipher->cipher_ctx)
			{
				uint8_t out[66];
				size_t outl = sizeof(out);
				evp_cipher_final(bio_cipher->cipher_ctx, out, &outl);
				if (outl)
				{
					ssize_t nxt = bio_write(bio->next, out, outl);
					if (nxt < 0)
						return nxt;
				}
			}
			return bio_flush(bio->next);
		case BIO_CTRL_EOF:
			return bio_eof(bio->next);
		case BIO_CTRL_CIPHER_GET:
			*(struct evp_cipher_ctx**)arg2 = bio_cipher->cipher_ctx;
			return 1;
		case BIO_CTRL_CIPHER_SET:
		{
			struct bio_cipher_set_ctx *set_ctx = arg2;
			if (bio_cipher->enc != -1)
				return -1;
			if (!evp_cipher_init(bio_cipher->cipher_ctx,
			                     set_ctx->cipher,
			                     set_ctx->key,
			                     set_ctx->iv,
			                     set_ctx->enc))
				return -1;
			bio_cipher->enc = set_ctx->enc;
			return 1;
		}
		default:
			return -1;
	}
}

static size_t readbuf(struct bio_cipher *bio_cipher, void **data, size_t size)
{
	size_t tmp = bio_cipher->decrypt_buf_size;
	if (!tmp)
		return 0;
	if (tmp > size)
		tmp = size;
	memcpy(*data, bio_cipher->decrypt_buf, tmp);
	memmove(&bio_cipher->decrypt_buf[0], &bio_cipher->decrypt_buf[tmp],
	        bio_cipher->decrypt_buf_size - tmp);
	bio_cipher->decrypt_buf_size -= tmp;
	*data = (uint8_t*)*data + tmp;
	return tmp;
}

static ssize_t bio_cipher_read(struct bio *bio, void *data, size_t size)
{
	if (!bio->next)
		return -1;
	struct bio_cipher *bio_cipher = bio->ctx;
	if (bio_cipher->enc != 0)
		return -1;
	size_t rd = readbuf(bio_cipher, &data, size);
	if (!bio->next)
		return rd;
	uint8_t out[4096];
	while (rd < size)
	{
		ssize_t blk = bio_read(bio->next, out, sizeof(out));
		if (blk < 0)
			return -1;
		if (!blk)
		{
			if (!bio_eof(bio->next))
				break;
			size_t outl = sizeof(bio_cipher->decrypt_buf);
			if (!evp_cipher_final(bio_cipher->cipher_ctx,
			                      bio_cipher->decrypt_buf,
			                      &outl))
				return -1;
			bio_cipher->decrypt_buf_size = outl;
			rd += readbuf(bio_cipher, &data, size - rd);
			break;
		}
		size_t outl = sizeof(bio_cipher->decrypt_buf);
		if (!evp_cipher_update(bio_cipher->cipher_ctx,
		                       bio_cipher->decrypt_buf,
		                       &outl, out, blk))
			return -1;
		bio_cipher->decrypt_buf_size = outl;
		rd += readbuf(bio_cipher, &data, size - rd);
	}
	return rd;
}

static ssize_t bio_cipher_write(struct bio *bio, const void *data, size_t size)
{
	if (!bio->next)
		return -1;
	struct bio_cipher *bio_cipher = bio->ctx;
	if (bio_cipher->enc != 1)
		return -1;
	uint8_t out[4096];
	size_t written = 0;
	while (written < size)
	{
		size_t blk = size - written;
		if (blk > 4096)
			blk = 4096;
		size_t outl = sizeof(out);
		if (!evp_cipher_update(bio_cipher->cipher_ctx, out, &outl, data, blk))
			return -1;
		if (outl)
		{
			ssize_t nxt = bio_write(bio->next, out, outl);
			if (nxt < 0)
				return nxt;
		}
		data = (uint8_t*)data + blk;
		written += blk;
	}
	return written;
}

static const struct bio_method g_bio_f_cipher =
{
	.ctx_size = sizeof(struct bio_cipher),
	.op_ctr = bio_cipher_ctr,
	.op_dtr = bio_cipher_dtr,
	.op_ctrl = bio_cipher_ctrl,
	.op_read = bio_cipher_read,
	.op_write = bio_cipher_write,
};

const struct bio_method *bio_f_cipher(void)
{
	return &g_bio_f_cipher;
}
