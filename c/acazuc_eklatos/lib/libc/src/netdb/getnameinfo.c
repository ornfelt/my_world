#include <netdb.h>

int getnameinfo(const struct sockaddr *addr, socklen_t addrlen,
                char *host, socklen_t hostlen, char *serv,
                socklen_t servlen, int flags)
{
	(void)addr;
	(void)addrlen;
	(void)host;
	(void)hostlen;
	(void)serv;
	(void)servlen;
	(void)flags;
	/* XXX  */
	return EAI_NONAME;
}
