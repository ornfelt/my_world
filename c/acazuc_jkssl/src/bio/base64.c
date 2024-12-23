#include "bio/bio.h"

#include <jkssl/evp.h>

#include <string.h>

struct bio_base64
{
	struct evp_encode_ctx *encode_ctx;
	uint8_t decode_buf[EVP_DECODE_MAX_SIZE(4096)];
	size_t decode_buf_size;
	int enc;
};

static int bio_base64_ctr(struct bio *bio)
{
	struct bio_base64 *bio_base64 = bio->ctx;
	bio_base64->enc = -1;
	bio_base64->encode_ctx = evp_encode_ctx_new();
	if (!bio_base64->encode_ctx)
		return 0;
	return 1;
}

static void bio_base64_dtr(struct bio *bio)
{
	struct bio_base64 *bio_base64 = bio->ctx;
	evp_encode_ctx_free(bio_base64->encode_ctx);
}

static long bio_base64_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                            void *arg2)
{
	(void)arg1;
	(void)arg2;
	struct bio_base64 *bio_base64 = bio->ctx;
	switch (cmd)
	{
		case BIO_CTRL_RESET:
			bio_base64->decode_buf_size = 0;
			return bio_reset(bio->next);
		case BIO_CTRL_FLUSH:
			if (bio_base64->encode_ctx)
			{
				uint8_t out[66];
				size_t outl = sizeof(out);
				evp_encode_final(bio_base64->encode_ctx, out, &outl);
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
		default:
			return -1;
	}
}

static size_t readbuf(struct bio_base64 *bio_base64, void **data, size_t size)
{
	size_t tmp = bio_base64->decode_buf_size;
	if (!tmp)
		return 0;
	if (tmp > size)
		tmp = size;
	memcpy(*data, bio_base64->decode_buf, tmp);
	memmove(&bio_base64->decode_buf[0], &bio_base64->decode_buf[tmp],
	        bio_base64->decode_buf_size - tmp);
	bio_base64->decode_buf_size -= tmp;
	*data = (uint8_t*)*data + tmp;
	return tmp;
}

static ssize_t bio_base64_read(struct bio *bio, void *data, size_t size)
{
	struct bio_base64 *bio_base64 = bio->ctx;
	if (bio_base64->enc == -1)
	{
		evp_decode_init(bio_base64->encode_ctx);
		bio_base64->enc = 0;
	}
	else if (bio_base64->enc != 0)
	{
		return -1;
	}
	size_t rd = readbuf(bio_base64, &data, size);
	if (!bio->next)
		return rd;
	uint8_t out[4096];
	while (rd < size)
	{
		ssize_t blk = bio_read(bio->next, out, sizeof(out));
		if (blk < 0)
			return -1;
		if (!blk)
			break;
		size_t outl = sizeof(bio_base64->decode_buf);
		if (!evp_decode_update(bio_base64->encode_ctx,
		                       bio_base64->decode_buf,
		                       &outl, out, blk))
			return -1;
		bio_base64->decode_buf_size = outl;
		rd += readbuf(bio_base64, &data, size - rd);
	}
	return rd;
}

static ssize_t bio_base64_write(struct bio *bio, const void *data, size_t size)
{
	if (!bio->next)
		return -1;
	struct bio_base64 *bio_base64 = bio->ctx;
	if (bio_base64->enc == -1)
	{
		evp_decode_init(bio_base64->encode_ctx);
		bio_base64->enc = 1;
	}
	else if (bio_base64->enc != 1)
	{
		return -1;
	}
	uint8_t out[EVP_ENCODE_MAX_SIZE(4096)];
	size_t written = 0;
	while (written < size)
	{
		size_t blk = size - written;
		if (blk > 4096)
			blk = 4096;
		size_t outl = sizeof(out);
		if (!evp_encode_update(bio_base64->encode_ctx, out, &outl, data, blk))
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

const struct bio_method *bio_f_base64(void)
{
	static const struct bio_method method =
	{
		.ctx_size = sizeof(struct bio_base64),
		.op_ctr = bio_base64_ctr,
		.op_dtr = bio_base64_dtr,
		.op_ctrl = bio_base64_ctrl,
		.op_read = bio_base64_read,
		.op_write = bio_base64_write,
	};
	return &method;
}
