#include "nbt.h"

#include <stdlib.h>
#include <string.h>

struct nbt_stream_op
{
	int (*read)(struct nbt_stream *stream, void *data, size_t size);
	int (*write)(struct nbt_stream *stream, const void *data, size_t size);
};

struct nbt_stream
{
	const struct nbt_stream_op *op;
	union
	{
		struct
		{
			FILE *fp;
		} file;
		struct
		{
			gzFile fp;
		} gzip;
		struct
		{
			const void *data;
			size_t size;
			size_t pos;
		} memory;
	};
};

static int memory_read(struct nbt_stream *stream, void *data, size_t size)
{
	if (stream->memory.size - stream->memory.pos < size)
		return 0;
	memcpy(data, &((uint8_t*)stream->memory.data)[stream->memory.pos], size);
	stream->memory.pos += size;
	return 1;
}

static int memory_write(struct nbt_stream *stream, const void *data, size_t size)
{
	/* XXX */
}

static const struct nbt_stream_op memory_op =
{
	.read = memory_read,
	.write = memory_write,
};

static int file_read(struct nbt_stream *stream, void *data, size_t size)
{
	size_t rd = fread(data, 1, size, stream->file.fp);
	if (ferror(stream->file.fp))
		return 0;
	if (rd < size)
		return 0;
	return 1;
}

static int file_write(struct nbt_stream *stream, const void *data, size_t size)
{
	size_t wr = fwrite(data, 1, size, stream->file.fp);
	if (ferror(stream->file.fp))
		return 0;
	if (wr < size)
		return 0;
	return 1;
}

static const struct nbt_stream_op file_op =
{
	.read = file_read,
	.write = file_write,
};

static int gzip_read(struct nbt_stream *stream, void *data, size_t size)
{
	int rd = gzread(stream->gzip.fp, data, size);
	if (rd < 0)
		return 0;
	if ((size_t)rd != size)
		return 0;
	return 1;
}

static int gzip_write(struct nbt_stream *stream, const void *data, size_t size)
{
	int wr = gzwrite(stream->gzip.fp, data, size);
	if (wr < 0)
		return 0;
	if ((size_t)wr != size)
		return 0;
	return 1;
}

static const struct nbt_stream_op gzip_op =
{
	.read = gzip_read,
	.write = gzip_write,
};

static struct nbt_stream *nbt_stream_alloc(const struct nbt_stream_op *op)
{
	struct nbt_stream *stream = calloc(sizeof(*stream), 1);
	if (!stream)
		return NULL;
	stream->op = op;
	return stream;
}

struct nbt_stream *nbt_stream_memory_new(const void *data, size_t size)
{
	struct nbt_stream *stream = nbt_stream_alloc(&memory_op);
	if (!stream)
		return NULL;
	stream->memory.data = data;
	stream->memory.size = size;
	return stream;
}

struct nbt_stream *nbt_stream_file_new(FILE *fp)
{
	struct nbt_stream *stream = nbt_stream_alloc(&file_op);
	if (!stream)
		return NULL;
	stream->file.fp = fp;
	return stream;
}

struct nbt_stream *nbt_stream_gzip_new(gzFile fp)
{
	struct nbt_stream *stream = nbt_stream_alloc(&gzip_op);
	if (!stream)
		return NULL;
	stream->gzip.fp = fp;
	return stream;
}

void nbt_stream_delete(struct nbt_stream *stream)
{
	if (!stream)
		return;
	free(stream);
}

int nbt_stream_write(struct nbt_stream *stream, const void *data, size_t size)
{
	return stream->op->write(stream, data, size);
}

int nbt_stream_read(struct nbt_stream *stream, void *data, size_t size)
{
	return stream->op->read(stream, data, size);
}

static inline uint16_t swap16(uint16_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((v & 0xFF00) >> 8)
	     | ((v & 0x00FF) << 8);
#else
	return v;
#endif
}

static inline uint32_t swap32(uint32_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((v & 0xFF000000) >> 24)
	     | ((v & 0x00FF0000) >>  8)
	     | ((v & 0x0000FF00) <<  8)
	     | ((v & 0x000000FF) << 24);
#else
	return v;
#endif
}

static inline uint64_t swap64(uint64_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((v & 0xFF00000000000000ULL) >> 56)
	     | ((v & 0x00FF000000000000ULL) >> 40)
	     | ((v & 0x0000FF0000000000ULL) >> 24)
	     | ((v & 0x000000FF00000000ULL) >>  8)
	     | ((v & 0x00000000FF000000ULL) <<  8)
	     | ((v & 0x0000000000FF0000ULL) << 24)
	     | ((v & 0x000000000000FF00ULL) << 40)
	     | ((v & 0x00000000000000FFULL) << 56);
#else
	return v;
#endif
}

int nbt_stream_w8(struct nbt_stream *stream, int8_t v)
{
	return nbt_stream_write(stream, &v, 1);
}

int nbt_stream_w16(struct nbt_stream *stream, int16_t v)
{
	v = swap16(v);
	return nbt_stream_write(stream, &v, 2);
}

int nbt_stream_w32(struct nbt_stream *stream, int32_t v)
{
	v = swap32(v);
	return nbt_stream_write(stream, &v, 4);
}

int nbt_stream_w64(struct nbt_stream *stream, int64_t v)
{
	v = swap64(v);
	return nbt_stream_write(stream, &v, 8);
}

int nbt_stream_r8(struct nbt_stream *stream, int8_t *v)
{
	return nbt_stream_read(stream, v, 1);
}

int nbt_stream_r16(struct nbt_stream *stream, int16_t *v)
{
	int16_t tmp;
	if (!nbt_stream_read(stream, &tmp, 2))
		return 0;
	*v = swap16(tmp);
	return 1;
}

int nbt_stream_r32(struct nbt_stream *stream, int32_t *v)
{
	int32_t tmp;
	if (!nbt_stream_read(stream, &tmp, 4))
		return 0;
	*v = swap32(tmp);
	return 1;
}

int nbt_stream_r64(struct nbt_stream *stream, int64_t *v)
{
	int64_t tmp;
	if (!nbt_stream_read(stream, &tmp, 8))
		return 0;
	*v = swap64(tmp);
	return 1;
}
