#ifndef LIBINTL_H
#define LIBINTL_H

#ifdef __cplusplus
extern "C" {
#endif

char *gettext(const char *msgid);
char *dgettext(const char *domain, const char *msgid);
char *dcgettext(const char *domain, const char *msgid, int category);

char *ngettext(const char *msgid, const char *msgid_plural, unsigned long n);
char *dngettext(const char *domain, const char *msgid,
                const char *msgid_plural, unsigned long n);
char *dcngettext(const char *domain, const char *msgid,
                 const char *msgid_plural, unsigned long n, int category);

char *textdomain(const char *domain);
char *bindtextdomain(const char *domain, const char *dirname);

#ifdef __cplusplus
}
#endif

#endif
