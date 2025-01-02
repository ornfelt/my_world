#include "h261.h"

#include <stdlib.h>

struct h261 *h261_new(void)
{
	struct h261 *h261 = calloc(1, sizeof(*h261));
	if (!h261)
		return NULL;
	return h261;
}

void h261_free(struct h261 *h261)
{
	if (!h261)
		return;
	free(h261);
}

static int bs_get(struct bitstream *bs)
{
	struct h261 *h261 = bs->userdata;
	uint8_t avail = (sizeof(bs->buf) * 8 - bs->len) / 8;
	uint8_t buf[sizeof(bs->buf)];
	size_t rd = fread(buf, 1, avail, h261->fp);
	if (ferror(h261->fp))
	{
		H261_ERR(h261, "failed to read from file");
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
	struct h261 *h261 = bs->userdata;
	uint8_t avail = (sizeof(bs->buf) * 8 - bs->len) / 8;
	uint8_t buf[sizeof(bs->buf)];
	for (size_t i = 0; i < avail; ++i)
		buf[i] = bs->buf >> ((sizeof(bs->buf) - 1 - i) * 8);
	size_t wr = fwrite(buf, 1, avail, h261->fp);
	if (ferror(h261->fp))
	{
		H261_ERR(h261, "failed to read from file");
		return 1;
	}
	bs->buf <<= wr * 8;
	bs->len += wr * 8;
	return 0;
}

void h261_init_io(struct h261 *h261, FILE *fp)
{
	h261->fp = fp;
	bs_init_read(&h261->bs);
	h261->bs.userdata = h261;
	h261->bs.get = bs_get;
	h261->bs.put = bs_put;
}

const char *h261_get_err(struct h261 *h261)
{
	return h261->errbuf;
}
