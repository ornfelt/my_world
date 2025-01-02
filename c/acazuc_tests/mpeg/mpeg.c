#include "bitstream.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct mpeg
{
	struct bitstream bs;
	FILE *fp;
};

int read_system_header(struct mpeg *mpeg)
{
	bs_getbits(&mpeg->bs, 32);
	uint32_t length = bs_getbits(&mpeg->bs, 16);
	if (length == (uint32_t)-1)
	{
		fprintf(stderr, "length EOF\n");
		return 1;
	}
	if (length < 8)
	{
		fprintf(stderr, "length too short\n");
		return 1;
	}
	printf("length: %" PRIu32 "\n", length);
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 6\n");
		return 1;
	}
	uint32_t rate_bound = bs_getbits(&mpeg->bs, 22);
	if (rate_bound == (uint32_t)-1)
	{
		fprintf(stderr, "rate bound EOF\n");
		return 1;
	}
	printf("rate bound: %" PRIu32 "\n", rate_bound);
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 7\n");
		return 1;
	}
	uint32_t audio_bound = bs_getbits(&mpeg->bs, 6);
	if (audio_bound == (uint32_t)-1)
	{
		fprintf(stderr, "audio bound EOF\n");
		return 1;
	}
	printf("audio bound: %" PRIu32 "\n", audio_bound);
	uint32_t flags = bs_getbits(&mpeg->bs, 4);
	if (flags == (uint32_t)-1)
	{
		fprintf(stderr, "flags EOF\n");
		return 1;
	}
	printf("flags: 0x%" PRIx32 "\n", flags);
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 8\n");
		return 1;
	}
	uint32_t video_bound = bs_getbits(&mpeg->bs, 5);
	if (video_bound == (uint32_t)-1)
	{
		fprintf(stderr, "video bound EOF\n");
		return 1;
	}
	printf("video bound: %" PRIu32 "\n", video_bound);
	uint32_t reserved = bs_getbits(&mpeg->bs, 8);
	if (reserved == (uint32_t)-1)
	{
		fprintf(stderr, "reserved EOF\n");
		return 1;
	}
	if (reserved != 0xFF)
	{
		fprintf(stderr, "invalid reserved\n");
		return 1;
	}
	while (bs_peekbits(&mpeg->bs, 1) == 1)
	{
		uint32_t stream_id = bs_getbits(&mpeg->bs, 8);
		if (stream_id == (uint32_t)-1)
		{
			fprintf(stderr, "stream id EOF\n");
			return 1;
		}
		printf("stream id: 0x%" PRIx32 "\n", stream_id);
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker 9\n");
			return 1;
		}
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker 10\n");
			return 1;
		}
		uint32_t scale = bs_getbit(&mpeg->bs);
		if (scale == (uint32_t)-1)
		{
			fprintf(stderr, "scale EOF\n");
			return 1;
		}
		uint32_t size_bound = bs_getbits(&mpeg->bs, 13);
		if (size_bound == (uint32_t)-1)
		{
			fprintf(stderr, "size bound EOF\n");
			return 1;
		}
		printf("size bound: %" PRIu32 "\n", size_bound);
	}
	return 0;
}

int read_pkt(struct mpeg *mpeg)
{
	bs_getbits(&mpeg->bs, 24);
	uint32_t stream_id = bs_getbits(&mpeg->bs, 8);
	printf("stream id: 0x%" PRIx32 "\n", stream_id);
	uint32_t length = bs_getbits(&mpeg->bs, 16);
	if (length == (uint32_t)-1)
	{
		fprintf(stderr, "length EOF\n");
		return 1;
	}
	printf("length: %" PRIu32 "\n", length);
	for (size_t i = 0; i < 16 && bs_peekbits(&mpeg->bs, 8) == 0xFF; ++i)
	{
		bs_getbits(&mpeg->bs, 8);
		length--;
	}
	uint32_t buffer_scale;
	uint32_t buffer_size;
	if (bs_peekbits(&mpeg->bs, 2) == 0x2)
	{
		bs_getbits(&mpeg->bs, 2);
		buffer_scale = bs_getbit(&mpeg->bs);
		if (buffer_scale == (uint32_t)-1)
		{
			fprintf(stderr, "buffer scale EOF\n");
			return 1;
		}
		printf("buffer scale: %" PRIu32 "\n", buffer_scale);
		buffer_size = bs_getbits(&mpeg->bs, 13);
		if (buffer_size == (uint32_t)-1)
		{
			fprintf(stderr, "buffer size EOF\n");
			return 1;
		}
		printf("buffer size: %" PRIu32 "\n", buffer_size);
		length--;
	}
	else
	{
		buffer_scale = 0;
		buffer_size = 0;
	}
	if (bs_getbits(&mpeg->bs, 2) != 0)
	{
		fprintf(stderr, "invalid marker\n");
		return 1;
	}
	uint32_t flags = bs_getbits(&mpeg->bs, 2);
	if (flags == (uint32_t)-1)
	{
		fprintf(stderr, "pts/dts flags EOF\n");
		return 1;
	}
	if (flags == 1)
	{
		fprintf(stderr, "invalid pts/dts flags\n");
		return 1;
	}
	if (flags & 2)
	{
		uint32_t pts_hi = bs_getbits(&mpeg->bs, 3);
		if (pts_hi == (uint32_t)-1)
		{
			fprintf(stderr, "pts_hi EOF\n");
			return 1;
		}
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker\n");
			return 1;
		}
		uint32_t pts_mi = bs_getbits(&mpeg->bs, 15);
		if (pts_mi == (uint32_t)-1)
		{
			fprintf(stderr, "pts_mi EOF\n");
			return 1;
		}
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker\n");
			return 1;
		}
		uint32_t pts_lo = bs_getbits(&mpeg->bs, 15);
		if (pts_lo == (uint32_t)-1)
		{
			fprintf(stderr, "pts_lo EOF\n");
			return 1;
		}
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker\n");
			return 1;
		}
		uint32_t pts = pts_lo | (pts_mi << 15) | (pts_hi << 30);
		length -= 5;
	}
	if (flags & 1)
	{
		if (bs_getbits(&mpeg->bs, 4) != 0x1)
		{
			fprintf(stderr, "invalid dts mark\n");
			return 1;
		}
		uint32_t dts_hi = bs_getbits(&mpeg->bs, 3);
		if (dts_hi == (uint32_t)-1)
		{
			fprintf(stderr, "dts hi EOF\n");
			return 1;
		}
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker\n");
			return 1;
		}
		uint32_t dts_mi = bs_getbits(&mpeg->bs, 15);
		if (dts_mi == (uint32_t)-1)
		{
			fprintf(stderr, "dts mi EOF\n");
			return 1;
		}
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker\n");
			return 1;
		}
		uint32_t dts_lo = bs_getbits(&mpeg->bs, 15);
		if (dts_lo == (uint32_t)-1)
		{
			fprintf(stderr, "dts lo EOF\n");
			return 1;
		}
		if (bs_getbit(&mpeg->bs) != 1)
		{
			fprintf(stderr, "invalid marker\n");
			return 1;
		}
		uint32_t dts = dts_lo | (dts_mi << 15) | (dts_hi << 30);
		length -= 5;
	}
	if (!flags)
	{
		if (bs_getbits(&mpeg->bs, 4) != 0xF)
		{
			fprintf(stderr, "padding EOF\n");
			return 1;
		}
		length--;
	}
	while (length)
	{
		if (bs_getbits(&mpeg->bs, 8) == (uint32_t)-1)
		{
			fprintf(stderr, "pkt data EOF\n");
			return 1;
		}
		--length;
	}
	return 0;
}

int read_pack(struct mpeg *mpeg)
{
	printf("pack\n");
	uint32_t magic = bs_getbits(&mpeg->bs, 32);
	if (magic == (uint32_t)-1)
	{
		fprintf(stderr, "magic EOF\n");
		return 1;
	}
	if (magic != 0x000001BA)
	{
		fprintf(stderr, "invalid ps magic: 0x%" PRIx32 "\n", magic);
		return 1;
	}
	uint32_t marker = bs_getbits(&mpeg->bs, 4);
	if (marker == (uint32_t)-1)
	{
		fprintf(stderr, "marker EOF\n");
		return 1;
	}
	if (marker != 0x2)
	{
		fprintf(stderr, "invalid marker 0\n");
		return 1;
	}
	printf("marker: 0x%" PRIx8 "\n", marker);
	uint32_t scr_hi = bs_getbits(&mpeg->bs, 3);
	if (scr_hi == (uint32_t)-1)
	{
		fprintf(stderr, "scr_hi EOF\n");
		return 1;
	}
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 1\n");
		return 1;
	}
	uint32_t scr_mi = bs_getbits(&mpeg->bs, 15);
	if (scr_mi == (uint32_t)-1)
	{
		fprintf(stderr, "scr_mi EOF\n");
		return 1;
	}
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 2\n");
		return 1;
	}
	uint32_t scr_lo = bs_getbits(&mpeg->bs, 15);
	if (scr_lo == (uint32_t)-1)
	{
		fprintf(stderr, "scr_lo EOF\n");
		return 1;
	}
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 3\n");
		return 1;
	}
	uint32_t scr = scr_lo | (scr_mi << 15) | (scr_hi << 30);
	printf("scr: 0x%" PRIx32 "\n", scr);
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 4\n");
		return 1;
	}
	uint32_t bitrate = bs_getbits(&mpeg->bs, 22);
	if (bitrate == (uint32_t)-1)
	{
		fprintf(stderr, "bitrate EOF\n");
		return 1;
	}
	printf("bitrate: %" PRIu32 "\n", bitrate);
	if (bs_getbit(&mpeg->bs) != 1)
	{
		fprintf(stderr, "invalid marker 5\n");
		return 1;
	}
	if (bs_peekbits(&mpeg->bs, 32) == 0x000001BB)
	{
		printf("system header\n");
		if (read_system_header(mpeg))
			return 1;
	}
	while (1)
	{
		uint32_t next = bs_peekbits(&mpeg->bs, 32);
		if (next == (uint32_t)-1)
		{
			fprintf(stderr, "mpeg eof\n");
			return 1;
		}
		if (next >= 0x1BD && next <= 0x1EF)
		{
			printf("pkt\n");
			if (read_pkt(mpeg))
				return 1;
		}
		else
		{
			break;
		}
	}
	return 0;
}

struct mpeg *mpeg_new(void)
{
	struct mpeg *mpeg = calloc(1, sizeof(*mpeg));
	if (!mpeg)
		return NULL;
	return mpeg;
}

void mpeg_free(struct mpeg *mpeg)
{
	if (!mpeg)
		return;
	free(mpeg);
}

static int bs_get(struct bitstream *bs)
{
	struct mpeg *mpeg = bs->userdata;
	uint8_t avail = (sizeof(bs->buf) * 8 - bs->len) / 8;
	uint8_t buf[sizeof(bs->buf)];
	size_t rd = fread(buf, 1, avail, mpeg->fp);
	if (ferror(mpeg->fp))
	{
		fprintf(stderr, "failed to read from file\n");
		return 1;
	}
	for (size_t i = 0; i < rd; ++i)
	{
		bs->buf <<= 8;
		bs->buf |= buf[i];
	}
	bs->len += rd * 8;
	return 0;
}

static int bs_put(struct bitstream *bs)
{
	struct mpeg *mpeg = bs->userdata;
	uint8_t avail = (sizeof(bs->buf) * 8 - bs->len) / 8;
	uint8_t buf[sizeof(bs->buf)];
	for (size_t i = 0; i < avail; ++i)
		buf[i] = bs->buf >> ((sizeof(bs->buf) - 1 - i) * 8);
	size_t wr = fwrite(buf, 1, avail, mpeg->fp);
	if (ferror(mpeg->fp))
	{
		fprintf(stderr, "failed to read from file\n");
		return 1;
	}
	bs->buf <<= wr * 8;
	bs->len += wr * 8;
	return 0;
}

void mpeg_init_io(struct mpeg *mpeg, FILE *fp)
{
	mpeg->fp = fp;
	bs_init_read(&mpeg->bs);
	mpeg->bs.userdata = mpeg;
	mpeg->bs.get = bs_get;
	mpeg->bs.put = bs_put;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s filename.mpeg\n", argv[0]);
		return EXIT_FAILURE;
	}
	FILE *fp = fopen(argv[1], "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: fopen(%s): %s\n", argv[0], argv[1],
		        strerror(errno));
		return EXIT_FAILURE;
	}
	struct mpeg *mpeg = mpeg_new();
	if (!mpeg)
	{
		fprintf(stderr, "mpeg_new failed\n");
		return EXIT_FAILURE;
	}
	mpeg_init_io(mpeg, fp);
	while (1)
	{
		if (read_pack(mpeg))
			return EXIT_FAILURE;
	}
	mpeg_free(mpeg);
	fclose(fp);
	return EXIT_SUCCESS;
}
