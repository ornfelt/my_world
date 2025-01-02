#include <libintl.h>

char *dngettext(const char *domain, const char *msgid,
                const char *msgid_plural, unsigned long n)
{
	(void)domain;
	/* XXX */
	if (n > 1)
		return (char*)msgid_plural;
	return (char*)msgid;
}
