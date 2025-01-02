#include <wctype.h>
#include <ctype.h>

int iswpunct(wint_t wc)
{
	/* XXX */
	return ispunct(wc);
}
