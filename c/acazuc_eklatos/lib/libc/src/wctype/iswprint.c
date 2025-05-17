#include <wctype.h>
#include <ctype.h>

int
iswprint(wint_t wc)
{
	/* XXX */
	return isprint(wc);
}
