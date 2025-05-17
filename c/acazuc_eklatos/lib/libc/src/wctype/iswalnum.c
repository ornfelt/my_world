#include <wctype.h>
#include <ctype.h>

int
iswalnum(wint_t wc)
{
	/* XXX */
	return isalnum(wc);
}
