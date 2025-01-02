#include <wctype.h>
#include <ctype.h>

int iswxdigit(wint_t wc)
{
	/* XXX */
	return isxdigit(wc);
}
