#include <stdlib.h>

intmax_t
strtoimax(const char *nptr, char **endptr, int base)
{
#if __SIZE_WIDTH__ == 32
	return strtoll(nptr, endptr, base);
#elif __SIZE_WIDTH__ == 64
	return strtol(nptr, endptr, base);
#else
# error "unknown arch"
#endif
}
