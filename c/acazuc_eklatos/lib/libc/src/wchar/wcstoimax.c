#include <wchar.h>

intmax_t wcstoimax(const wchar_t *nptr, wchar_t **endptr, int base)
{
#if __SIZE_WIDTH__ == 32
	return wcstoll(nptr, endptr, base);
#elif __SIZE_WIDTH__ == 64
	return wcstol(nptr, endptr, base);
#else
# error "unknown arch"
#endif
}
