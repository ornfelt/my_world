#include <netdb.h>

const char *gai_strerror(int err)
{
	switch (err)
	{
		case EAI_NONAME:
			return "Host not found";
		case EAI_MEMORY:
			return "Out of memory";
		default:
			return "";
	}
}
