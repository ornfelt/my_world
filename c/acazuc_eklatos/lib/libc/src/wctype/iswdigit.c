#include <wctype.h>
#include <ctype.h>

int
iswdigit(wint_t wc)
{
	/* XXX */
	return isdigit(wc);
}
