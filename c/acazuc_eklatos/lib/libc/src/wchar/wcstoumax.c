#include <wchar.h>

uintmax_t wcstoumax(const wchar_t *nptr, wchar_t **endptr, int base)
{
#if __SIZE_WIDTH__ == 32
	return wcstoull(nptr, endptr, base);
#elif __SIZE_WIDTH__ == 64
	return wcstoul(nptr, endptr, base);
#else
# error "unknown arch"
#endif
}
