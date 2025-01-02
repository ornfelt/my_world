#include <arpa/inet.h>

#include <netinet/in.h>

#include <net/ethernet.h>

#include <sys/endian.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	switch (af)
	{
		case AF_INET:
		{
			const struct in_addr *in = src;
			uint8_t tmp[4];
			be32enc(tmp, ntohl(in->s_addr));
			int ret = snprintf(dst, size,
			                   "%" PRIu8 "."
			                   "%" PRIu8 "."
			                   "%" PRIu8 "."
			                   "%" PRIu8,
			                   tmp[0],
			                   tmp[1],
			                   tmp[2],
			                   tmp[3]);
			if (ret < 0 || (size_t)ret >= size)
			{
				errno = ENOSPC;
				return NULL;
			}
			return dst;
		}
		case AF_INET6:
		{
			const struct in6_addr *in6 = src;
			int ret = snprintf(dst, size,
			                   "%04" PRIx16 "."
			                   "%04" PRIx16 "."
			                   "%04" PRIx16 "."
			                   "%04" PRIx16 "."
			                   "%04" PRIx16 "."
			                   "%04" PRIx16 "."
			                   "%04" PRIx16 "."
			                   "%04" PRIx16,
			                   htons(((uint16_t*)in6->s6_addr)[0]),
			                   htons(((uint16_t*)in6->s6_addr)[1]),
			                   htons(((uint16_t*)in6->s6_addr)[2]),
			                   htons(((uint16_t*)in6->s6_addr)[3]),
			                   htons(((uint16_t*)in6->s6_addr)[4]),
			                   htons(((uint16_t*)in6->s6_addr)[5]),
			                   htons(((uint16_t*)in6->s6_addr)[6]),
			                   htons(((uint16_t*)in6->s6_addr)[7]));
			if (ret < 0 || (size_t)ret >= size)
			{
				errno = ENOSPC;
				return NULL;
			}
			return dst;
		}
		default:
			errno = EAFNOSUPPORT;
			return NULL;
	}
}

int inet_pton(int af, const char *src, void *dst)
{
	switch (af)
	{
		case AF_INET:
		{
			struct in_addr *in = dst;
			uint8_t values[4];
			char *endptr = (char*)src;
			for (int i = 0; i < 4; ++i)
			{
				static const char expected[] =
				{
					'.', '.', '.', '\0'
				};
				errno = 0;
				unsigned long value = strtoul(endptr, &endptr, 10);
				if (errno
				 || *endptr != expected[i]
				 || value > 0xFF)
					return 0;
				values[i] = value;
				endptr++;
			}
			in->s_addr = htonl(be32dec(values));
			return 1;
		}
		default:
			errno = EAFNOSUPPORT;
			return -1;
	}
}

char *inet_ntoa(struct in_addr in)
{
	static char buf[64];
	return (char*)inet_ntop(AF_INET, &in, buf, sizeof(buf));
}

int inet_aton(const char *src, struct in_addr *dst)
{
	return inet_pton(AF_INET, src, dst);
}

char *ether_ntoa(const struct ether_addr *addr)
{
	static char buf[64];
	snprintf(buf, sizeof(buf), "%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 ":"
	                           "%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8,
	                           addr->addr[0], addr->addr[1], addr->addr[2],
	                           addr->addr[3], addr->addr[4], addr->addr[5]);
	return buf;
}
