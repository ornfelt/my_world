#include <wctype.h>
#include <ctype.h>

int iswalpha(wint_t wc)
{
	/* XXX */
	return isalpha(wc);
}
