#include <stdio.h>
#include <errno.h>
#include <wchar.h>

int fputws_unlocked(const wchar_t *ws, FILE *fp)
{
	int total = 0;
	while (*ws)
	{
		char buf[4096];
		const wchar_t *prv = ws;
		size_t n = wcsrtombs(buf, &ws, sizeof(buf), NULL);
		if (n == (size_t)-1)
			return WEOF;
		if (fwrite_unlocked(buf, n, 1, fp) != n)
			return WEOF;
		total += ws - prv;
	}
	return total;
}
