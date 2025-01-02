#include <net/net.h>

#include <errno.h>
#include <sma.h>
#include <std.h>

static struct sma netpkt_sma;

void netpkt_init(void)
{
	sma_init(&netpkt_sma, sizeof(struct netpkt), NULL, NULL, "netpkt");
}

struct netpkt *netpkt_alloc(size_t bytes)
{
	struct netpkt *pkt = sma_alloc(&netpkt_sma, 0);
	if (!pkt)
		return NULL;
	pkt->len = bytes;
	pkt->alloc = malloc(bytes, 0);
	if (!pkt->alloc)
	{
		sma_free(&netpkt_sma, pkt);
		return NULL;
	}
	pkt->data = pkt->alloc;
	return pkt;
}

void netpkt_free(struct netpkt *pkt)
{
	if (!pkt)
		return;
	free(pkt->alloc);
	sma_free(&netpkt_sma, pkt);
}

void netpkt_advance(struct netpkt *pkt, size_t bytes)
{
	pkt->data = &((uint8_t*)pkt->data)[bytes];
	pkt->len -= bytes;
}

void *netpkt_grow_front(struct netpkt *pkt, size_t bytes)
{
	size_t avail_front = (uint8_t*)pkt->data - (uint8_t*)pkt->alloc;
	if (avail_front >= bytes)
	{
		pkt->len += bytes;
		pkt->data = (uint8_t*)pkt->data - bytes;
		return pkt->data;
	}
	uint8_t *newdata = realloc(pkt->alloc, pkt->len + bytes, 0);
	if (!newdata)
		return NULL;
	memmove(&newdata[bytes], newdata, pkt->len);
	pkt->len += bytes;
	pkt->alloc = newdata;
	pkt->data = newdata;
	return newdata;
}

int netpkt_shrink_tail(struct netpkt *pkt, size_t bytes)
{
	if (bytes > pkt->len)
		return -EINVAL;
	pkt->len -= bytes;
	return 0;
}
