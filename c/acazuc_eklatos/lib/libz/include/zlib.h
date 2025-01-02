#ifndef ZLIB_H
#define ZLIB_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4

#define Z_OK             0
#define Z_STREAM_END     1
#define Z_ERRNO         -1
#define Z_STREAM_ERROR  -2
#define Z_DATA_ERROR    -3
#define Z_MEM_ERROR     -4
#define Z_BUF_ERROR     -5
#define Z_NEED_MORE     -6

#define Z_NO_COMPRESSION       0
#define Z_BEST_SPEED           1
#define Z_BEST_COMPRESSION     9
#define Z_DEFAULT_COMPRESSION -1

#define Z_BINARY  0
#define Z_TEXT    1
#define Z_ASCII   Z_TEXT
#define Z_UNKNOWN 2

#define Z_NULL 0

struct zlib_ctx;

typedef struct z_stream_s
{
	const uint8_t *next_in;
	size_t avail_in;
	uint8_t *next_out;
	size_t avail_out;
	const char *msg;
	void *internal_state;
	uint32_t adler;
} z_stream;

typedef struct gz_header_s
{
	int32_t text;
	uint32_t time;
	int32_t xflags;
	int32_t os;
	uint8_t *extra;
	uint32_t extra_len;
	uint32_t extra_max;
	uint8_t *name;
	uint32_t name_max;
	uint8_t *comment;
	uint32_t comm_max;
	int32_t hcrc;
	int32_t done;
} gz_header;

int deflateInit(z_stream *stream, int level);
int deflate(z_stream *stream, int flush);
int deflateEnd(z_stream *stream);

int inflateInit(z_stream *stream);
int inflate(z_stream *stream, int flush);
int inflateEnd(z_stream *stream);

int inflateGetHeader(z_stream *stream, gz_header *header);

struct gzFile_s;

typedef struct gzFile_s *gzFile;

gzFile gzopen(const char *path, const char *mode);
gzFile gzdopen(int fd, const char *mode);
int gzbuffer(gzFile file, unsigned size);
int gzread(gzFile file, void *buf, unsigned len);
size_t gzfread(void *buf, size_t size, size_t nitems, gzFile file);
int gzwrite(gzFile file, void *buf, unsigned len);
size_t gzfwrite(void *buf, size_t size, size_t nitems, gzFile file);
int gzclose(gzFile file);

uint32_t adler32(uint32_t adler, const uint8_t *buf, unsigned len);
uint32_t adler32_z(uint32_t adler, const uint8_t *buf, size_t len);

uint32_t crc32(uint32_t crc, const uint8_t *buf, unsigned len);
uint32_t crc32_z(uint32_t crc, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
