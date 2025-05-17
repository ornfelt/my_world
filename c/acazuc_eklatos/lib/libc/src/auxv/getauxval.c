#include <sys/auxv.h>

size_t *__libc_auxv;

size_t
getauxval(size_t type)
{
	for (size_t i = 0; __libc_auxv[i]; i += 2)
	{
		if (__libc_auxv[i] == type)
			return __libc_auxv[i + 1];
	}
	return 0;
}
