#include <stdio.h>
#include <wchar.h>
#include <errno.h>

wint_t
fgetwc_unlocked(FILE *fp)
{
	char buf[4];

	for (size_t i = 0; i < 4; ++i)
	{
		int c = fgetc_unlocked(fp);
		if (c == EOF)
			return WEOF;
		buf[i] = c;
		wchar_t wc;
		switch (mbrtowc(&wc, buf, i + 1, NULL))
		{
			case (size_t)-2:
				break;
			case (size_t)-1:
				return WEOF;
			default:
				return wc;
		}
	}
	errno = EILSEQ;
	return WEOF;
}
