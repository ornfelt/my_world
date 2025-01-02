#include <net/local.h>

#include <errno.h>
#include <sock.h>
#include <sma.h>
#include <std.h>

static struct sma pfls_sma;

void pfls_init(void)
{
	sma_init(&pfls_sma, sizeof(struct pfls), NULL, NULL, "pfls");
}

int pfls_alloc(struct pfls **pflsp, struct node *node)
{
	struct pfls *pfls = sma_alloc(&pfls_sma, M_ZERO);
	if (!pfls)
		return -ENOMEM;
	pfls->node = node;
	mutex_init(&pfls->mutex, 0);
	refcount_init(&pfls->refcount, 1);
	*pflsp = pfls;
	return 0;
}

void pfls_free(struct pfls *pfls)
{
	if (!pfls)
		return;
	if (refcount_dec(&pfls->refcount))
		return;
	mutex_destroy(&pfls->mutex);
	sma_free(&pfls_sma, pfls);
}

void pfls_ref(struct pfls *pfls)
{
	refcount_inc(&pfls->refcount);
}

int pfl_open(int domain, int type, int protocol, struct sock **sock)
{
	if (domain != PF_LOCAL)
		return -EINVAL;
	switch (type)
	{
		case SOCK_STREAM:
			return pfl_stream_open(domain, type, protocol, sock);
		case SOCK_DGRAM:
			return pfl_dgram_open(domain, type, protocol, sock);
		default:
			return -EINVAL;
	}
}
