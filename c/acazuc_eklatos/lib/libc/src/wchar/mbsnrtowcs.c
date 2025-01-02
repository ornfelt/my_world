#include <wchar.h>

size_t mbsnrtowcs(wchar_t *d, const char **s, size_t n, size_t len, mbstate_t *ps)
{
	size_t sum = 0;
	while (**s)
	{
		if (!n)
			return sum;
		wchar_t wc;
		size_t nb = 0;
		if ((*s)[0])
		{
			nb++;
			if ((*s)[1])
			{
				nb++;
				if ((*s)[2])
				{
					nb++;
					if ((*s)[3])
						nb++;
				}
			}
		}
		size_t wn = mbrtowc(&wc, *s, nb, ps);
		if (wn == (size_t)-1)
			return (size_t)-1;
		if (d)
		{
			if (!len)
				return sum;
			*(d++) = wc;
			len--;
		}
		sum++;
		*s += wn;
		n--;
	}
	*s = NULL;
	return sum;
}
