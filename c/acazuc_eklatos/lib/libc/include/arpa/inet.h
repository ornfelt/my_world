#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <sys/socket.h>

#include <netinet/in.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t ntohs(uint16_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((v & 0x00FF) << 8)
	     | ((v & 0xFF00) >> 8);
#else
	return v;
#endif
}

static inline uint32_t ntohl(uint32_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((v & 0x000000FF) << 24)
	     | ((v & 0x0000FF00) <<  8)
	     | ((v & 0x00FF0000) >>  8)
	     | ((v & 0xFF000000) >> 24);
#else
	return v;
#endif
}

static inline uint16_t htons(uint16_t v)
{
	return ntohs(v);
}

static inline uint32_t htonl(uint32_t v)
{
	return ntohl(v);
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int inet_pton(int af, const char *src, void *dst);
char *inet_ntoa(struct in_addr in);
int inet_aton(const char *src, struct in_addr *dst);

#ifdef __cplusplus
}
#endif

#endif
