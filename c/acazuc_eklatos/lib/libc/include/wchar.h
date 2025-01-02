#ifndef WCHAR_H
#define WCHAR_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WEOF ((wint_t)-1)

typedef unsigned int wint_t;
typedef struct FILE FILE;

struct tm;

typedef struct mbstate
{
	int dummy;
} mbstate_t;

size_t mbrtowc(wchar_t *wc, const char *s, size_t n, mbstate_t *ps);
size_t mbsrtowcs(wchar_t *d, const char **s, size_t n, mbstate_t *ps);
size_t mbsnrtowcs(wchar_t *d, const char **s, size_t n, size_t len, mbstate_t *ps);
int mbsinit(mbstate_t *ps);
size_t mbrlen(const char *s, size_t n, mbstate_t *ps);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
size_t wcsrtombs(char *d, const wchar_t **s, size_t n, mbstate_t *ps);
size_t wcsnrtombs(char *d, const wchar_t **s, size_t n, size_t len,
                  mbstate_t *ps);

wint_t btowc(int c);
int wctob(wint_t c);

wint_t fputwc(wchar_t wc, FILE *fp);
wint_t fputwc_unlocked(wchar_t wc, FILE *fp);
wint_t putwc(wchar_t wc, FILE *fp);
wint_t putwc_unlocked(wchar_t wc, FILE *fp);
int fputws(const wchar_t *ws, FILE *fp);
int fputws_unlocked(const wchar_t *ws, FILE *fp);
int fwide(FILE *fp, int mode);
wint_t putwchar(wchar_t wc);
wint_t putwchar_unlocked(wchar_t wc);

wint_t fgetwc(FILE *fp);
wint_t fgetwc_unlocked(FILE *fp);
wint_t getwc(FILE *fp);
wint_t getwc_unlocked(FILE *fp);
wint_t getwchar(void);
wint_t getwchar_unlocked(void);
wchar_t *fgetws(wchar_t *ws, int size, FILE *fp);
wchar_t *fgetws_unlocked(wchar_t *ws, int size, FILE *fp);

wchar_t *wmemchr(const wchar_t *s, wchar_t wc, size_t n);
wchar_t *wmemrchr(const wchar_t *s, wchar_t wc, size_t n);
int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wmemcpy(wchar_t *d, const wchar_t *s, size_t n);
wchar_t *wmemmove(wchar_t *d, const wchar_t *s, size_t n);
wchar_t *wmemset(wchar_t *d, wchar_t wc, size_t n);
wchar_t *wmemmem(const wchar_t *haystack, size_t haystacklen,
                 const wchar_t *needle, size_t needlelen);
wchar_t *wmemccpy(wchar_t *d, const wchar_t *s, wchar_t wc, size_t n);

size_t wcslen(const wchar_t *s);
wchar_t *wcscat(wchar_t *d, const wchar_t *s);
wchar_t *wcschr(const wchar_t *s, wchar_t wc);
wchar_t *wcschrnul(const wchar_t *s, wchar_t wc);
wchar_t *wcsrchr(const wchar_t *s, wchar_t wc);
wchar_t *wcsdup(const wchar_t *s);
wchar_t *wcsndup(const wchar_t *s, size_t n);
int wcscmp(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcscpy(wchar_t *d, const wchar_t *s);
size_t wcscspn(const wchar_t *s, const wchar_t *reject);
wchar_t *wcsncat(wchar_t *d, const wchar_t *s, size_t n);
int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wcsncpy(wchar_t *d, const wchar_t *s, size_t n);
size_t wcsnlen(const wchar_t *s, size_t n);
wchar_t *wcspbrk(const wchar_t *s, const wchar_t *accept);
size_t wcsspn(const wchar_t *s, const wchar_t *accept);
wchar_t *wcsstr(const wchar_t *s, const wchar_t *needle);
wchar_t *wcsnstr(const wchar_t *s, const wchar_t *needle, size_t n);
int wcscasecmp(const wchar_t *s1, const wchar_t *s2);
int wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wcstok(wchar_t *s, const wchar_t *delim, wchar_t **saveptr);
size_t wcslcpy(wchar_t *d, const wchar_t *s, size_t n);
size_t wcslcat(wchar_t *d, const wchar_t *s, size_t n);
wchar_t *wcpcpy(wchar_t *d, const wchar_t *s);
wchar_t *wcpncpy(wchar_t *d, const wchar_t *s, size_t n);

int wprintf(const wchar_t *fmt, ...);
int vwprintf(const wchar_t *fmt, va_list va_arg);
int swprintf(wchar_t *d, size_t n, const wchar_t *fmt, ...);
int vswprintf(wchar_t *d, size_t n, const wchar_t *fmt, va_list va_arg);
int fwprintf(FILE *fp, const wchar_t *fmt, ...);
int vfwprintf(FILE *fp, const wchar_t *fmt, va_list va_arg);

unsigned long wcstoul(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long long wcstoull(const wchar_t *nptr, wchar_t **endptr, int base);
long wcstol(const wchar_t *nptr, wchar_t **endptr, int base);
long long wcstoll(const wchar_t *nptr, wchar_t **endptr, int base);
intmax_t wcstoimax(const wchar_t *nptr, wchar_t **endptr, int base);
uintmax_t wcstoumax(const wchar_t *nptr, wchar_t **endptr, int base);

float wcstof(const wchar_t *nptr, wchar_t **endptr);
double wcstod(const wchar_t *nptr, wchar_t **endptr);
long double wcstold(const wchar_t *nptr, wchar_t **endptr);

size_t wcsftime(wchar_t *s, size_t max, const wchar_t *format,
                const struct tm *tm);

#ifdef __cplusplus
}
#endif

#endif
