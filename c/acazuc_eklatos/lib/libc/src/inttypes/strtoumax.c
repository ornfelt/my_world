#include <stdlib.h>

uintmax_t strtoumax(const char *nptr, char **endptr, int base)
{
#if __SIZE_WIDTH__ == 32
	return strtoull(nptr, endptr, base);
#elif __SIZE_WIDTH__ == 64
	return strtoul(nptr, endptr, base);
#else
# error "unknown arch"
#endif
}
