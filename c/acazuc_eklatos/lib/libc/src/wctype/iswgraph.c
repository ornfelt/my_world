#include <wctype.h>
#include <ctype.h>

int
iswgraph(wint_t wc)
{
	 /* XXX */
	return isgraph(wc);
}
