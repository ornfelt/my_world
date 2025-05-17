#include <string.h>
#include <wchar.h>

size_t
wcsnrtombs(char *d, const wchar_t **s, size_t n, size_t len, mbstate_t *ps)
{
	char buf[4];
	size_t sum = 0;

	while (**s)
	{
		if (!n)
			return sum;
		size_t wn = wcrtomb(buf, **s, ps);
		if (wn == (size_t)-1)
			return (size_t)-1;
		if (d)
		{
			if (wn > len)
				return sum;
			memcpy(d, buf, wn);
			d += wn;
			len -= wn;
		}
		sum += wn;
		(*s)++;
		n--;
	}
	*s = NULL;
	return sum;
}
