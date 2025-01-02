#ifndef NETDB_H
#define NETDB_H

#include <sys/socket.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EAI_NONAME -1
#define EAI_MEMORY -2

#define _PATH_SERVICES  "/etc/services"
#define _PATH_HOSTS     "/etc/hosts"
#define _PATH_PROTOCOLS "/etc/protocols"
#define _PATH_NETWORKS  "/etc/networks"

struct addrinfo
{
	int ai_flags;
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	socklen_t ai_addrlen;
	struct sockaddr *ai_addr;
	char *ai_canonname;
	struct addrinfo *ai_next;
};

struct servent
{
	char *s_name;
	char **s_aliases;
	int s_port;
	char *s_proto;
};

struct hostent
{
	char *h_name;
	char **h_aliases;
	int h_addrtype;
	int h_length;
	char **h_addr_list;
};

struct protoent
{
	char *p_name;
	char **p_aliases;
	int p_proto;
};

struct netent
{
	char *n_name;
	char **n_aliases;
	int n_addrtype;
	uint32_t n_net;
};

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints, struct addrinfo **result);
void freeaddrinfo(struct addrinfo *result);
const char *gai_strerror(int err);

struct servent *getservent(void);
struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int port, const char *proto);
void setservent(int stayopen);
void endservent(void);

struct hostent *gethostent(void);
struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
void sethostent(int stayopen);
void endhostent(void);

struct protoent *getprotoent(void);
struct protoent *getprotobyname(const char *name);
struct protoent *getprotobynumber(int proto);
void setprotoent(int stayopen);
void endprotoent(void);

struct netent *getnetent(void);
struct netent *getnetbyname(const char *name);
struct netent *getnetbyaddr(uint32_t net, int type);
void setnetent(int stayopen);
void endnetent(void);

int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host,
                socklen_t hostlen, char *serv, socklen_t servlen, int flags);

#ifdef __cplusplus
}
#endif

#endif
