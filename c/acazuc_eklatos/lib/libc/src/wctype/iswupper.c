#include <wctype.h>
#include <ctype.h>

int
iswupper(wint_t wc)
{
	/* XXX */
	return isupper(wc);
}
