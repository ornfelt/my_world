#ifndef NET_LOCAL_H
#define NET_LOCAL_H

#include <refcount.h>
#include <mutex.h>

struct sockaddr_un
{
	sa_family_t sun_family;
	char sun_path[108];
};

struct pfls
{
	refcount_t refcount;
	struct node *node;
	struct mutex mutex;
	int type;
	struct sock *listen; /* socket bound to it */
};

struct file;

int pfls_alloc(struct pfls **pfls, struct node *node);
void pfls_free(struct pfls *pfls);
void pfls_ref(struct pfls *pfls);

int pfl_stream_open(int domain, int type, int protocol, struct sock **sock);
int pfl_stream_openpair(int domain, int type, int protocol, struct sock **socks);
int pfl_dgram_open(int domain, int type, int protocol, struct sock **sock);
int pfl_dgram_openpair(int domain, int type, int protocol, struct sock **socks);

int pfl_open(int domain, int type, int protocol, struct sock **sock);

#endif
