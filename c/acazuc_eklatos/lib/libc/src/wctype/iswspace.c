#include <wctype.h>
#include <ctype.h>

int iswspace(wint_t wc)
{
	/* XXX */
	return isspace(wc);
}
