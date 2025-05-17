#include <libintl.h>

char *
ngettext(const char *msgid,
         const char *msgid_plural,
         unsigned long n)
{
	/* XXX */
	if (n > 1)
		return (char*)msgid_plural;
	return (char*)msgid;
}
