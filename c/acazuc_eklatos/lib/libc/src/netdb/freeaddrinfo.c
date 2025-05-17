#include <stdlib.h>
#include <netdb.h>

void
freeaddrinfo(struct addrinfo *result)
{
	while (result)
	{
		free(result->ai_addr);
		free(result->ai_canonname);
		struct addrinfo *next = result->ai_next;
		free(result);
		result = next;
	}
}
