#include <string.h>

int memcmp(const void *s1, const void *s2, size_t n)
{
	size_t i = 0;
	if (n >= sizeof(size_t))
	{
		size_t end = n - n % sizeof(size_t);
		do
		{
			if (*(size_t*)&((char*)s1)[i] != *(size_t*)&((char*)s2)[i])
				break;
			i += sizeof(size_t);
		} while (i < end);
	}
	while (i < n)
	{
		int d = (int)((char*)s1)[i] - (int)((char*)s2)[i];
		if (d)
			return d;
		i++;
	}
	return 0;
}
