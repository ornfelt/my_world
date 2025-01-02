#include <wchar.h>
#include <errno.h>

size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
{
	if (!wc)
		return 0;
	if (!s)
		return 0;
	if (wc < 0x80)
	{
		s[0] = wc & 0x7F;
		return 1;
	}
	if (wc < 0x800)
	{
		s[0] = 0xC0 | ((wc >> 0x6) & 0x1F);
		s[1] = 0x80 | ((wc >> 0x0) & 0x3F);
		return 2;
	}
	if (wc < 0x10000)
	{
		s[0] = 0xE0 | ((wc >> 0xC) & 0x0F);
		s[1] = 0x80 | ((wc >> 0x6) & 0x3F);
		s[2] = 0x80 | ((wc >> 0x0) & 0x3F);
		return 3;
	}
	if (wc < 0x10FFFF)
	{
		s[0] = 0xF0 | ((wc >> 0x12) & 0x07);
		s[1] = 0x80 | ((wc >> 0x0C) & 0x3F);
		s[2] = 0x80 | ((wc >> 0x06) & 0x3F);
		s[3] = 0x80 | ((wc >> 0x00) & 0x3F);
		return 4;
	}
	return (size_t)-1;
}
