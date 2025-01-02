#include "bitstream.h"
#include "huffman.h"
#include "ringbuf.h"

#include <arpa/inet.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>

#define MAX_WINDOW_SIZE 32768

enum ctx_state
{
	CTX_ZLHEAD, /* zlib header */
	CTX_ZLDICT, /* zlib dict */
	CTX_GZHEAD, /* gzip header */
	CTX_GZXTRA, /* gzip extra field */
	CTX_GZNAME, /* gzip file name */
	CTX_GZCOMM, /* gzip file comment */
	CTX_GZHCRC, /* gzip crc16 */
	CTX_BLKHDR, /* final flag + block type */
	CTX_RAWHDR, /* raw block header */
	CTX_RAWBLK, /* raw block data */
	CTX_DYNHDR, /* hlit len + hdist len + hclen */
	CTX_DYNLEN, /* code lengths */
	CTX_DYNLIT, /* lit lengths */
	CTX_HUFCOD, /* huffman code from lit */
	CTX_EXTLEN, /* code extra length */
	CTX_HUFDST, /* code distance */
	CTX_EXTDST, /* code extra distance */
	CTX_REFCPY, /* copy back-referenced data */
	CTX_ZADLER, /* zlib end adler32 */
	CTX_GZECRC, /* gzip end crc32 */
	CTX_GZELEN, /* gzip end length */
	CTX_STREND, /* stream end */
};

struct ctx
{
	int inf_def; /* marker for interal_state */
	enum ctx_state state;
	struct bitstream bs;
	uint8_t output_buf[MAX_WINDOW_SIZE];
	uint8_t input_buf[4096];
	struct ringbuf output;
	struct ringbuf input;
	uint32_t last;
	int is_gzip;
	uint32_t gz_crc;
	uint32_t gz_len;
	uint32_t zlib_adler;
	struct huffman huff_lit;
	struct huffman huff_dist;
	uint32_t raw_len;
	uint32_t raw_nlen;
	uint32_t dyn_hlit;
	uint32_t dyn_hdist;
	uint32_t dyn_hclen;
	uint8_t dyn_lit_lengths_distances[286 + 32];
	struct huffman dyn_code_lengths_huff;
	size_t dyn_hlit_pos;
	int dyn_hlit_hufcode;
	int hufcode_char;
	uint16_t back_size;
	uint8_t extra_length_bits;
	int hufcode_dist;
	uint16_t back_dist;
	uint8_t extra_dist_bits;
	struct
	{
		uint8_t cmf;
		uint8_t flg;
		uint32_t dict;
	} zlib;
	struct
	{
		uint8_t id1;
		uint8_t id2;
		uint8_t cm;
		uint8_t flg;
		uint32_t mtime;
		uint8_t xfl;
		uint8_t os;
		uint16_t xlen;
		uint8_t *extra;
		uint16_t extra_len;
		char *name;
		size_t name_len;
		char *comment;
		size_t comment_len;
		uint16_t crc16;
	} gzip;
};

static void write_byte(struct ctx *ctx, uint8_t c)
{
	ringbuf_write_byte(&ctx->output, c);
	ctx->gz_len++;
	ctx->gz_crc = crc32(ctx->gz_crc, &c, 1);
	ctx->zlib_adler = adler32(ctx->zlib_adler, &c, 1);
}

static void generate_static_huffman(struct ctx *ctx)
{
	/* XXX hardcode it in static memory */
	uint32_t hlit = 288;
	uint32_t hdist = 32;
	uint8_t lit_lengths_distances[288 + 32];
	for (size_t i = 0; i < 144; ++i)
		lit_lengths_distances[i] = 8;
	for (size_t i = 144; i < 256; ++i)
		lit_lengths_distances[i] = 9;
	for (size_t i = 256; i < 280; ++i)
		lit_lengths_distances[i] = 7;
	for (size_t i = 280; i < 288; ++i)
		lit_lengths_distances[i] = 8;
	for (size_t i = 288; i < 288 + 32; ++i)
		lit_lengths_distances[i] = 5;
	huffman_generate(&ctx->huff_lit, lit_lengths_distances, hlit);
	huffman_generate(&ctx->huff_dist, &lit_lengths_distances[hlit], hdist);
}

static int decode_code(struct bitstream *bs, uint8_t *dst,
                       size_t *i, int v)
{
	if (v == 16)
	{
		if (!*i)
			return Z_DATA_ERROR;
		if (!bitstream_has_read(bs, 2))
			return Z_NEED_MORE;
		uint32_t rp;
		bitstream_read(bs, &rp, 2);
		uint16_t cpy = dst[(*i) - 1];
		rp += 3;
		for (size_t j = 0; j < rp; ++j)
			dst[(*i)++] = cpy;
		return Z_OK;
	}
	if (v == 17)
	{
		if (!bitstream_has_read(bs, 3))
			return Z_NEED_MORE;
		uint32_t rp;
		bitstream_read(bs, &rp, 3);
		rp += 3;
		for (size_t j = 0; j < rp; ++j)
			dst[(*i)++] = 0;
		return Z_OK;
	}
	if (v == 18)
	{
		if (!bitstream_has_read(bs, 7))
			return Z_NEED_MORE;
		uint32_t rp;
		bitstream_read(bs, &rp, 7);
		rp += 11;
		for (size_t j = 0; j < rp; ++j)
			dst[(*i)++] = 0;
		return Z_OK;
	}
	if (v > MAX_BITS)
		return Z_DATA_ERROR;
	dst[(*i)++] = v;
	return Z_OK;
}

static int handle_zlhead(struct ctx *ctx)
{
	uint32_t magic;
	if (!bitstream_has_read(&ctx->bs, 16))
		return Z_NEED_MORE;
	bitstream_peek(&ctx->bs, &magic, 16);
	if ((magic & 0x0F) == 0x08)
	{
		uint32_t ringbuf_size = 1 << (8 + ((magic >> 4) & 0xF));
		if (ringbuf_size > MAX_WINDOW_SIZE)
			return Z_DATA_ERROR;
		ringbuf_init(&ctx->output, ctx->output_buf, ringbuf_size);
		bitstream_skip(&ctx->bs, 16);
		ctx->zlib.cmf = magic & 0xFF;
		ctx->zlib.flg = (magic >> 8) & 0xFF;
		if (magic & (1 << 13))
		{
			ctx->state = CTX_ZLDICT;
			return Z_OK;
		}
	}
	else
	{
		ringbuf_init(&ctx->output, ctx->output_buf, sizeof(ctx->output_buf));
	}
	ctx->state = CTX_GZHEAD;
	return Z_OK;
}

static int handle_zldict(struct ctx *ctx)
{
	if (!bitstream_has_read(&ctx->bs, 32))
		return Z_NEED_MORE;
	bitstream_peek(&ctx->bs, &ctx->zlib.dict, 32);
	ctx->state = CTX_GZHEAD;
	return Z_OK;
}

static int handle_gzhead(struct ctx *ctx)
{
	if (!ctx->gzip.id1)
	{
		uint32_t magic;
		if (!bitstream_has_read(&ctx->bs, 16))
			return Z_NEED_MORE;
		bitstream_peek(&ctx->bs, &magic, 16);
		if (magic != 0x8B1F)
		{
			ctx->state = CTX_BLKHDR;
			return Z_OK;
		}
		ctx->is_gzip = 1;
		ctx->gzip.id1 = magic & 0xFF;
		ctx->gzip.id2 = (magic >> 8) & 0xFF;
		bitstream_skip(&ctx->bs, 16);
	}
	if (!bitstream_has_read(&ctx->bs, 64))
		return Z_NEED_MORE;
	uint32_t tmp;
	bitstream_read(&ctx->bs, &tmp, 8);
	ctx->gzip.cm = tmp;
	bitstream_read(&ctx->bs, &tmp, 8);
	ctx->gzip.flg = tmp;
	bitstream_read(&ctx->bs, &tmp, 32);
	ctx->gzip.mtime = tmp;
	bitstream_read(&ctx->bs, &tmp, 8);
	ctx->gzip.xfl = tmp;
	bitstream_read(&ctx->bs, &tmp, 8);
	ctx->gzip.os = tmp;
	if (ctx->gzip.flg & (1 << 2))
		ctx->state = CTX_GZXTRA;
	else if (ctx->gzip.flg & (1 << 3))
		ctx->state = CTX_GZNAME;
	else if (ctx->gzip.flg & (1 << 4))
		ctx->state = CTX_GZCOMM;
	else if (ctx->gzip.flg & (1 << 1))
		ctx->state = CTX_GZHCRC;
	else
		ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_gzxtra(struct ctx *ctx)
{
	if (!ctx->gzip.xlen)
	{
		uint32_t xlen;
		if (!bitstream_has_read(&ctx->bs, 16))
			return Z_NEED_MORE;
		bitstream_read(&ctx->bs, &xlen, 16);
		ctx->gzip.xlen = xlen;
		if (!xlen)
			goto end;
		ctx->gzip.extra = malloc(xlen);
		if (!ctx->gzip.extra)
			return Z_MEM_ERROR;
	}
	size_t avail = bitstream_avail_read(&ctx->bs) / 8;
	if (!avail)
		return Z_NEED_MORE;
	size_t rem = ctx->gzip.xlen - ctx->gzip.extra_len;
	if (avail > rem)
		avail = rem;
	for (size_t i = 0; i < avail; ++i)
	{
		uint32_t tmp;
		bitstream_read(&ctx->bs, &tmp, 8);
		ctx->gzip.extra[ctx->gzip.extra_len++] = i;
	}
	if (ctx->gzip.extra_len != ctx->gzip.xlen)
		return Z_OK;

end:
	if (ctx->gzip.flg & (1 << 3))
		ctx->state = CTX_GZNAME;
	else if (ctx->gzip.flg & (1 << 4))
		ctx->state = CTX_GZCOMM;
	else if (ctx->gzip.flg & (1 << 1))
		ctx->state = CTX_GZHCRC;
	else
		ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_gzname(struct ctx *ctx)
{
	size_t avail = bitstream_avail_read(&ctx->bs) / 8;
	if (!avail)
		return Z_NEED_MORE;
	for (size_t i = 0; i < avail; ++i)
	{
		char *new_name = realloc(ctx->gzip.name,
		                         ctx->gzip.name_len + 1);
		if (!new_name)
			return Z_MEM_ERROR;
		uint32_t tmp;
		bitstream_read(&ctx->bs, &tmp, 8);
		new_name[ctx->gzip.name_len++] = tmp;
		ctx->gzip.name = new_name;
		if (!tmp)
			goto end;
	}
	return Z_NEED_MORE;

end:
	if (ctx->gzip.flg & (1 << 4))
		ctx->state = CTX_GZCOMM;
	else if (ctx->gzip.flg & (1 << 1))
		ctx->state = CTX_GZHCRC;
	else
		ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_gzcomm(struct ctx *ctx)
{
	size_t avail = bitstream_avail_read(&ctx->bs) / 8;
	if (!avail)
		return Z_NEED_MORE;
	for (size_t i = 0; i < avail; ++i)
	{
		char *new_comment = realloc(ctx->gzip.comment,
		                            ctx->gzip.comment_len + 1);
		if (!new_comment)
			return Z_MEM_ERROR;
		uint32_t tmp;
		bitstream_read(&ctx->bs, &tmp, 8);
		new_comment[ctx->gzip.comment_len++] = tmp;
		ctx->gzip.comment = new_comment;
		if (!tmp)
			goto end;
	}
	return Z_NEED_MORE;

end:
	if (ctx->gzip.flg & (1 << 1))
		ctx->state = CTX_GZHCRC;
	else
		ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_gzhcrc(struct ctx *ctx)
{
	if (!bitstream_has_read(&ctx->bs, 16))
		return Z_NEED_MORE;
	uint32_t tmp;
	bitstream_read(&ctx->bs, &tmp, 16);
	ctx->gzip.crc16 = tmp;
	/* XXX verify header crc */
	return Z_OK;
}

static int handle_blkhdr(struct ctx *ctx)
{
	if (ctx->last)
	{
		ctx->state = CTX_ZADLER;
		return Z_OK;
	}
	if (!bitstream_has_read(&ctx->bs, 3))
		return Z_NEED_MORE;
	uint32_t block_type;
	bitstream_read(&ctx->bs, &ctx->last, 1);
	bitstream_read(&ctx->bs, &block_type, 2);
	switch (block_type)
	{
		case 0:
			ctx->state = CTX_RAWHDR;
			return Z_OK;
		case 1:
			generate_static_huffman(ctx);
			ctx->state = CTX_HUFCOD;
			ctx->hufcode_char = -1;
			return Z_OK;
		case 2:
			ctx->state = CTX_DYNHDR;
			return Z_OK;
		default:
			return Z_STREAM_ERROR;
	}
}

static int handle_rawhdr(struct ctx *ctx)
{
	if (ctx->bs.pos % 8)
		bitstream_skip(&ctx->bs, 8 - (ctx->bs.pos % 8));
	if (!bitstream_has_read(&ctx->bs, 32))
		return Z_NEED_MORE;
	bitstream_read(&ctx->bs, &ctx->raw_len, 16);
	bitstream_read(&ctx->bs, &ctx->raw_nlen, 16);
	if (((ctx->raw_len + ctx->raw_nlen) & 0xFFFF) != 0xFFFF)
		return Z_STREAM_ERROR;
	ctx->state = CTX_RAWBLK;
	return Z_OK;
}

static int handle_rawblk(struct ctx *ctx)
{
	int first = 1;
	while (ctx->raw_len)
	{
		uint32_t cpy_len = ctx->raw_len;
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
		ctx->gz_len += cpy_len;
		ctx->gz_crc = crc32(ctx->gz_crc,
		                    ringbuf_read_ptr(&ctx->input),
		                    cpy_len);
		ctx->zlib_adler = adler32(ctx->zlib_adler,
		                          ringbuf_read_ptr(&ctx->input),
		                          cpy_len);
		ringbuf_advance_read(&ctx->input, cpy_len);
		ringbuf_advance_write(&ctx->output, cpy_len);
		ctx->raw_len -= cpy_len;
		first = 0;
	}
	ctx->state = CTX_BLKHDR;
	return Z_OK;
}

static int handle_dynhdr(struct ctx *ctx)
{
	if (!bitstream_has_read(&ctx->bs, 14))
		return Z_NEED_MORE;
	bitstream_read(&ctx->bs, &ctx->dyn_hlit, 5);
	bitstream_read(&ctx->bs, &ctx->dyn_hdist, 5);
	bitstream_read(&ctx->bs, &ctx->dyn_hclen, 4);
	ctx->dyn_hlit += 257;
	ctx->dyn_hdist += 1;
	ctx->dyn_hclen += 4;
	ctx->state = CTX_DYNLEN;
	return Z_OK;
}

static int handle_dynlen(struct ctx *ctx)
{
	if (!bitstream_has_read(&ctx->bs, ctx->dyn_hclen * 3))
		return Z_NEED_MORE;
	uint8_t code_lengths[19];
	static const uint8_t code_lengths_idx[19] =
	{
		16, 17, 18, 0,
		8 , 7 , 9 , 6,
		10, 5 , 11, 4,
		12, 3 , 13, 2,
		14, 1 , 15,
	};
	for (size_t i = 0; i < ctx->dyn_hclen; ++i)
	{
		uint32_t val;
		bitstream_read(&ctx->bs, &val, 3);
		code_lengths[code_lengths_idx[i]] = val;
	}
	for (size_t i = ctx->dyn_hclen; i < 19; ++i)
		code_lengths[code_lengths_idx[i]] = 0;
	huffman_generate(&ctx->dyn_code_lengths_huff, code_lengths, 19);
	ctx->dyn_hlit_pos = 0;
	ctx->state = CTX_DYNLIT;
	ctx->dyn_hlit_hufcode = -1;
	return Z_OK;
}

static int handle_dynlit(struct ctx *ctx)
{
	while (ctx->dyn_hlit_pos < ctx->dyn_hlit + ctx->dyn_hdist)
	{
		if (ctx->dyn_hlit_hufcode < 0)
		{
			ctx->dyn_hlit_hufcode = huffman_decode(&ctx->bs,
			                                       &ctx->dyn_code_lengths_huff);
			if (ctx->dyn_hlit_hufcode < 0)
				return ctx->dyn_hlit_hufcode;
		}
		int ret = decode_code(&ctx->bs, ctx->dyn_lit_lengths_distances,
		                      &ctx->dyn_hlit_pos,
		                      ctx->dyn_hlit_hufcode);
		if (ret != Z_OK)
			return ret;
		ctx->dyn_hlit_hufcode = -1;
	}
	huffman_generate(&ctx->huff_lit, ctx->dyn_lit_lengths_distances,
	                 ctx->dyn_hlit);
	huffman_generate(&ctx->huff_dist,
	                 &ctx->dyn_lit_lengths_distances[ctx->dyn_hlit],
	                 ctx->dyn_hdist);
	ctx->state = CTX_HUFCOD;
	ctx->hufcode_char = -1;
	return Z_OK;
}

static int handle_hufcod(struct ctx *ctx)
{
	if (ctx->hufcode_char >= 0)
	{
		uint8_t c = ctx->hufcode_char;
		if (!ringbuf_write_size(&ctx->output))
			return Z_NEED_MORE;
		write_byte(ctx, c);
	}
	ctx->hufcode_char = huffman_decode(&ctx->bs, &ctx->huff_lit);
	if (ctx->hufcode_char < 0)
		return ctx->hufcode_char;
	if (ctx->hufcode_char <= 255)
	{
		uint8_t c = ctx->hufcode_char;
		if (!ringbuf_write_size(&ctx->output))
			return Z_NEED_MORE;
		write_byte(ctx, c);
		ctx->hufcode_char = -1;
		return Z_OK;
	}
	if (ctx->hufcode_char == 256)
	{
		ctx->state = CTX_BLKHDR;
		return Z_OK;
	}
	if (ctx->hufcode_char == 285)
	{
		ctx->back_size = 258;
		ctx->state = CTX_HUFDST;
		return Z_OK;
	}
	static const uint8_t extra_lengths[] =
	{
		/* 257 */ 3  , 4  , 5  , 6  , 7,
		/* 262 */ 8  , 9  , 10 , 11 , 13,
		/* 267 */ 15 , 17 , 19 , 23 , 27,
		/* 272 */ 31 , 35 , 43 , 51 , 59,
		/* 277 */ 67 , 83 , 99 , 115, 131,
		/* 282 */ 163, 195, 227,
	};
	static const uint8_t extra_length_bits[] =
	{
		/* 257 */ 0, 0, 0, 0,
		/* 261 */ 0, 0, 0, 0,
		/* 265 */ 1, 1, 1, 1,
		/* 269 */ 2, 2, 2, 2,
		/* 273 */ 3, 3, 3, 3,
		/* 277 */ 4, 4, 4, 4,
		/* 281 */ 5, 5, 5, 5,
	};
	ctx->back_size = extra_lengths[ctx->hufcode_char - 257];
	ctx->extra_length_bits = extra_length_bits[ctx->hufcode_char - 257];
	if (ctx->extra_length_bits)
		ctx->state = CTX_EXTLEN;
	else
		ctx->state = CTX_HUFDST;
	return Z_OK;
}

static int handle_extlen(struct ctx *ctx)
{
	uint32_t add;
	if (!bitstream_has_read(&ctx->bs, ctx->extra_length_bits))
		return Z_NEED_MORE;
	bitstream_read(&ctx->bs, &add, ctx->extra_length_bits);
	ctx->back_size += add;
	ctx->state = CTX_HUFDST;
	return Z_OK;
}

static int handle_hufdst(struct ctx *ctx)
{
	ctx->hufcode_dist = huffman_decode(&ctx->bs, &ctx->huff_dist);
	if (ctx->hufcode_dist < 0)
		return ctx->hufcode_dist;
	static const uint8_t extra_dist_bits[] =
	{
		/*  0 */ 0 , 0 , 0 , 0,
		/*  4 */ 1 , 1 , 2 , 2,
		/*  8 */ 3 , 3 , 4 , 4,
		/* 12 */ 5 , 5 , 6 , 6,
		/* 16 */ 7 , 7 , 8 , 8,
		/* 20 */ 9 , 9 , 10, 10,
		/* 24 */ 11, 11, 12, 12,
		/* 28 */ 13, 13,
	};
	static const uint16_t extra_dists[] =
	{
		/*  0 */ 1    , 2    , 3   , 4,
		/*  4 */ 5    , 7    , 9   , 13,
		/*  8 */ 17   , 25   , 33  , 49,
		/* 12 */ 65   , 97   , 129 , 193,
		/* 16 */ 257  , 385  , 513 , 769,
		/* 20 */ 1025 , 1537 , 2049, 3073,
		/* 24 */ 4097 , 6145 , 8193, 12289,
		/* 28 */ 16385, 24577,
	};
	ctx->back_dist = extra_dists[ctx->hufcode_dist];
	ctx->extra_dist_bits = extra_dist_bits[ctx->hufcode_dist];
	if (ctx->extra_dist_bits)
		ctx->state = CTX_EXTDST;
	else
		ctx->state = CTX_REFCPY;
	return Z_OK;
}

static int handle_extdst(struct ctx *ctx)
{
	uint32_t add;
	if (!bitstream_has_read(&ctx->bs, ctx->extra_dist_bits))
		return Z_NEED_MORE;
	bitstream_read(&ctx->bs, &add, ctx->extra_dist_bits);
	ctx->back_dist += add;
	ctx->state = CTX_REFCPY;
	return Z_OK;
}

static int handle_refcpy(struct ctx *ctx)
{
	/* XXX check back_dist isn't going further begining of ringbuf */
	while (ctx->back_size)
	{
		if (!ringbuf_write_size(&ctx->output))
			return Z_NEED_MORE;
		uint8_t c = *(uint8_t*)ringbuf_write_ptr_at(&ctx->output,
		                                            ctx->output.size - ctx->back_dist);
		write_byte(ctx, c);
		ctx->back_size--;
	}
	ctx->state = CTX_HUFCOD;
	ctx->hufcode_char = -1;
	return Z_OK;
}

static int handle_zadler(struct ctx *ctx)
{
	if (ctx->bs.pos % 8)
		bitstream_skip(&ctx->bs, 8 - (ctx->bs.pos % 8));
	if (!bitstream_has_read(&ctx->bs, 32))
		return Z_NEED_MORE;
	uint32_t adler;
	bitstream_read(&ctx->bs, &adler, 32);
	if (adler != ntohl(ctx->zlib_adler))
		return Z_DATA_ERROR;
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
	if (!bitstream_has_read(&ctx->bs, 32))
		return Z_NEED_MORE;
	uint32_t crc;
	bitstream_read(&ctx->bs, &crc, 32);
	if (crc != ctx->gz_crc)
		return Z_DATA_ERROR;
	ctx->state = CTX_GZELEN;
	return Z_OK;
}

static int handle_gzelen(struct ctx *ctx)
{
	if (!bitstream_has_read(&ctx->bs, 32))
		return Z_NEED_MORE;
	uint32_t len;
	bitstream_read(&ctx->bs, &len, 32);
	if (len != ctx->gz_len)
		return Z_DATA_ERROR;
	ctx->state = CTX_STREND;
	return Z_OK;
}

static int handle_strend(struct ctx *ctx)
{
	(void)ctx;
	return Z_STREAM_END;
}

int inflateInit(z_stream *stream)
{
	stream->msg = NULL;
	struct ctx *ctx = calloc(1, sizeof(struct ctx));
	if (!ctx)
		return Z_MEM_ERROR;
	stream->internal_state = ctx;
	ctx->inf_def = 0;
	ringbuf_init(&ctx->input, ctx->input_buf, sizeof(ctx->input_buf));
	ctx->bs.ringbuf = &ctx->input;
	ctx->gz_crc = crc32(0, NULL, 0);
	ctx->zlib_adler = adler32(0, NULL, 0);
	return Z_OK;
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

int inflate(z_stream *stream, int flush)
{
	(void)flush; /* XXX */
	struct ctx *ctx = stream->internal_state;
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
				case CTX_ZLDICT:
					ret = handle_zldict(ctx);
					break;
				case CTX_GZHEAD:
					ret = handle_gzhead(ctx);
					break;
				case CTX_GZXTRA:
					ret = handle_gzxtra(ctx);
					break;
				case CTX_GZNAME:
					ret = handle_gzname(ctx);
					break;
				case CTX_GZCOMM:
					ret = handle_gzcomm(ctx);
					break;
				case CTX_GZHCRC:
					ret = handle_gzhcrc(ctx);
					break;
				case CTX_BLKHDR:
					ret = handle_blkhdr(ctx);
					break;
				case CTX_RAWHDR:
					ret = handle_rawhdr(ctx);
					break;
				case CTX_RAWBLK:
					ret = handle_rawblk(ctx);
					break;
				case CTX_DYNHDR:
					ret = handle_dynhdr(ctx);
					break;
				case CTX_DYNLEN:
					ret = handle_dynlen(ctx);
					break;
				case CTX_DYNLIT:
					ret = handle_dynlit(ctx);
					break;
				case CTX_HUFCOD:
					ret = handle_hufcod(ctx);
					break;
				case CTX_EXTLEN:
					ret = handle_extlen(ctx);
					break;
				case CTX_HUFDST:
					ret = handle_hufdst(ctx);
					break;
				case CTX_EXTDST:
					ret = handle_extdst(ctx);
					break;
				case CTX_REFCPY:
					ret = handle_refcpy(ctx);
					break;
				case CTX_GZECRC:
					ret = handle_gzecrc(ctx);
					break;
				case CTX_ZADLER:
					ret = handle_zadler(ctx);
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

int inflateEnd(z_stream *stream)
{
	if (!stream || !stream->internal_state)
		return Z_STREAM_ERROR;
	struct ctx *ctx = stream->internal_state;
	if (ctx->inf_def)
		return Z_STREAM_ERROR;
	free(ctx->gzip.extra);
	free(ctx->gzip.name);
	free(ctx->gzip.comment);
	free(ctx);
	stream->internal_state = NULL;
	return Z_OK;
}

int inflateGetHeader(z_stream *stream, gz_header *header)
{
	(void)stream;
	(void)header;
	/* XXX */
	return Z_OK;
}
