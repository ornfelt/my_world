#include "bitstream.h"
#include "huffman.h"
#include "ringbuf.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <zlib.h>

#define MAX_WINDOW_SIZE 32768

enum ctx_state
{
	CTX_ZLHEAD, /* zlib header */
	CTX_GZHEAD, /* gzip header */
	CTX_BLKHDR, /* final flag + block type */
	CTX_RAWHDR, /* raw block header */
	CTX_RAWBLK, /* raw block data */
	CTX_ZADLER, /* zlib end adler32 */
	CTX_GZECRC, /* gzip end crc32 */
	CTX_GZELEN, /* gzip end length */
	CTX_STREND, /* stream end */
};

struct ctx
{
	int inf_def; /* marker for internal_state */
	int level;
	enum ctx_state state;
	uint8_t output_buf[4096];
	uint8_t input_buf[MAX_WINDOW_SIZE];
	struct ringbuf output;
	struct ringbuf input;
	struct bitstream bs;
	uint32_t blk_len;
	uint32_t blk_pos;
	int last_blk;
	int is_gzip;
	int end;
	uint32_t gz_crc;
	uint32_t gz_len;
	uint32_t zlib_adler;
};

int deflateInit(z_stream *stream, int level)
{
	if (level < Z_DEFAULT_COMPRESSION || level > Z_BEST_COMPRESSION)
		return Z_STREAM_ERROR;
	if (level == Z_DEFAULT_COMPRESSION)
		level = 6;
	stream->msg = NULL;
	struct ctx *ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return Z_MEM_ERROR;
	ctx->inf_def = 1;
	ctx->level = level;
	ctx->gz_crc = crc32(0, NULL, 0);
	ctx->zlib_adler = adler32(0, NULL, 0);
	ringbuf_init(&ctx->output, ctx->output_buf, sizeof(ctx->output_buf));
	ringbuf_init(&ctx->input, ctx->input_buf, sizeof(ctx->input_buf));
	ctx->bs.ringbuf = &ctx->output;
#if 0
	ctx->state = CTX_GZHEAD;
	ctx->is_gzip = 1;
#else
	ctx->state = CTX_ZLHEAD;
	ctx->is_gzip = 0;
#endif
	stream->internal_state = ctx;
	return Z_OK;
}

static int handle_zlhead(struct ctx *ctx)
{
	if (!bitstream_has_write(&ctx->bs, 16))
		return Z_NEED_MORE;
	uint32_t v = 0x9C78; /* XXX compression level */
	bitstream_write(&ctx->bs, v, 16);
	ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_gzhead(struct ctx *ctx)
{
	if (!bitstream_has_write(&ctx->bs, 16 + 64))
		return Z_NEED_MORE;
	uint32_t tmp = 0x8B1F;
	bitstream_write(&ctx->bs, tmp, 16);
	tmp = 8; /* CM */
	bitstream_write(&ctx->bs, tmp, 8);
	tmp = 0; /* FLG */
	bitstream_write(&ctx->bs, tmp, 8);
	tmp = 0; /* mtime */
	bitstream_write(&ctx->bs, tmp, 32);
	tmp = 0; /* XFL */
	bitstream_write(&ctx->bs, tmp, 8);
	tmp = 3; /* OS (unix) */
	bitstream_write(&ctx->bs, tmp, 8);
	ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_blkhdr(struct ctx *ctx, int flush)
{
	if (ctx->last_blk)
	{
		ctx->state = CTX_ZADLER;
		return Z_OK;
	}
	if (flush == Z_NO_FLUSH
	 && ringbuf_read_size(&ctx->input) < MAX_WINDOW_SIZE - 1)
		return Z_NEED_MORE;
	if (!bitstream_has_write(&ctx->bs, 3))
		return Z_NEED_MORE;
	ctx->blk_len = ringbuf_read_size(&ctx->input);
	ctx->blk_pos = 0;
	ctx->last_blk = (flush == Z_FINISH);
	uint32_t last = ctx->last_blk;
	uint32_t type = 0;
	bitstream_write(&ctx->bs, last, 1);
	bitstream_write(&ctx->bs, type, 2);
	ctx->state = CTX_RAWHDR;
	return Z_OK;
}

static int handle_rawhdr(struct ctx *ctx)
{
	uint32_t req = 32;
	if (ctx->bs.pos)
		req += 8 - (ctx->bs.pos % 8);
	if (!bitstream_has_write(&ctx->bs, req))
		return Z_NEED_MORE;
	uint32_t raw_len = ctx->blk_len;
	uint32_t raw_nlen = ~raw_len;
	if (ctx->bs.pos)
	{
		uint32_t tmp = 0;
		bitstream_write(&ctx->bs, tmp, 8 - (ctx->bs.pos % 8));
	}
	bitstream_write(&ctx->bs, raw_len, 16);
	bitstream_write(&ctx->bs, raw_nlen, 16);
	ctx->state = CTX_RAWBLK;
	return Z_OK;
}

static int handle_rawblk(struct ctx *ctx)
{
	int first = 1;
	while (ctx->blk_len)
	{
		uint32_t cpy_len = ctx->blk_len;
		uint32_t avail_in = ringbuf_contiguous_read_size(&ctx->input);
		if (cpy_len > avail_in)
			cpy_len = avail_in;
		uint32_t avail_out = ringbuf_contiguous_write_size(&ctx->output);
		if (cpy_len > avail_out)
			cpy_len = avail_out;
		if (!cpy_len)
			return first ? Z_NEED_MORE : Z_OK;
		memmove(ringbuf_write_ptr(&ctx->output),
		        ringbuf_read_ptr(&ctx->input),
		        cpy_len);
		ringbuf_advance_read(&ctx->input, cpy_len);
		ringbuf_advance_write(&ctx->output, cpy_len);
		ctx->blk_len -= cpy_len;
		first = 0;
	}
	ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_zadler(struct ctx *ctx)
{
	if (ctx->bs.pos)
	{
		uint8_t req = 8 - (ctx->bs.pos % 8);
		if (!bitstream_has_write(&ctx->bs, req))
			return Z_NEED_MORE;
		uint32_t tmp = 0;
		bitstream_write(&ctx->bs, tmp, req);
	}
	if (!bitstream_has_write(&ctx->bs, 32))
		return Z_NEED_MORE;
	bitstream_write(&ctx->bs, ntohl(ctx->zlib_adler), 32);
	if (ctx->is_gzip)
	{
		ctx->state = CTX_GZECRC;
		return Z_OK;
	}
	ctx->state = CTX_STREND;
	return Z_OK;
}

static int handle_gzecrc(struct ctx *ctx)
{
	if (ctx->bs.pos)
	{
		uint8_t req = 8 - (ctx->bs.pos % 8);
		if (!bitstream_has_write(&ctx->bs, req))
			return Z_NEED_MORE;
		uint32_t tmp = 0;
		bitstream_write(&ctx->bs, tmp, req);
	}
	if (!bitstream_has_write(&ctx->bs, 32))
		return Z_NEED_MORE;
	bitstream_write(&ctx->bs, ctx->gz_crc, 32);
	ctx->state = CTX_GZELEN;
	return Z_OK;
}

static int handle_gzelen(struct ctx *ctx)
{
	if (!bitstream_has_write(&ctx->bs, 32))
		return Z_NEED_MORE;
	bitstream_write(&ctx->bs, ctx->gz_len, 32);
	ctx->state = CTX_STREND;
	return Z_OK;
}

static int handle_strend(struct ctx *ctx)
{
	(void)ctx;
	return Z_STREAM_END;
}

static size_t copy_in(z_stream *stream)
{
	struct ctx *ctx = stream->internal_state;
	size_t cpy_len = ringbuf_write_size(&ctx->input);
	if (!cpy_len)
		return 0;
	if (cpy_len > stream->avail_in)
		cpy_len = stream->avail_in;
	if (!cpy_len)
		return 0;
	ringbuf_write(&ctx->input, stream->next_in, cpy_len);
	ctx->gz_len += cpy_len;
	ctx->gz_crc = crc32(ctx->gz_crc, stream->next_in, cpy_len);
	ctx->zlib_adler = adler32(ctx->zlib_adler, stream->next_in, cpy_len);
	stream->avail_in -= cpy_len;
	stream->next_in += cpy_len;
	return cpy_len;
}

static size_t copy_out(z_stream *stream)
{
	struct ctx *ctx = stream->internal_state;
	size_t cpy_len = ringbuf_read_size(&ctx->output);
	if (!cpy_len)
		return 0;
	if (cpy_len > stream->avail_out)
		cpy_len = stream->avail_out;
	if (!cpy_len)
		return 0;
	ringbuf_read(&ctx->output, stream->next_out, cpy_len);
	stream->avail_out -= cpy_len;
	stream->next_out += cpy_len;
	return cpy_len;
}

int deflate(z_stream *stream, int flush)
{
	struct ctx *ctx = stream->internal_state;
	if (!ctx->inf_def)
		return Z_STREAM_ERROR;
	int ret = Z_OK;
	while (ret == Z_OK
	 && (stream->avail_in || ringbuf_read_size(&ctx->input))
	 && (stream->avail_out || ringbuf_write_size(&ctx->output)))
	{
		if (stream->avail_in)
			copy_in(stream);
		do
		{
			switch (ctx->state)
			{
				case CTX_ZLHEAD:
					ret = handle_zlhead(ctx);
					break;
				case CTX_GZHEAD:
					ret = handle_gzhead(ctx);
					break;
				case CTX_BLKHDR:
					ret = handle_blkhdr(ctx, flush);
					break;
				case CTX_RAWHDR:
					ret = handle_rawhdr(ctx);
					break;
				case CTX_RAWBLK:
					ret = handle_rawblk(ctx);
					break;
				case CTX_ZADLER:
					ret = handle_zadler(ctx);
					break;
				case CTX_GZECRC:
					ret = handle_gzecrc(ctx);
					break;
				case CTX_GZELEN:
					ret = handle_gzelen(ctx);
					break;
				case CTX_STREND:
					ret = handle_strend(ctx);
					break;
				default:
					ret = Z_MEM_ERROR;
					break;
			}
		}
		while (ret == Z_OK);
		if (stream->avail_out)
		{
			if (copy_out(stream) && ret == Z_NEED_MORE)
				ret = Z_OK;
		}
		if (stream->avail_in)
		{
			if (copy_in(stream) && ret == Z_NEED_MORE)
				ret = Z_OK;
		}
	}
	if (ret == Z_STREAM_END && ringbuf_read_size(&ctx->output))
		ret = Z_OK;
	if (ret == Z_NEED_MORE)
		ret = Z_OK;
	return ret;
}

int deflateEnd(z_stream *stream)
{
	if (!stream || !stream->internal_state)
		return Z_STREAM_ERROR;
	struct ctx *ctx = stream->internal_state;
	if (!ctx->inf_def)
		return Z_STREAM_ERROR;
	free(ctx);
	stream->internal_state = NULL;
	return Z_OK;
}
