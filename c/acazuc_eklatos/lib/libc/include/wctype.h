#ifndef WCTYPE_H
#define WCTYPE_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned wctype_t;
typedef unsigned wctrans_t;

int iswalnum(wint_t wc);
int iswalpha(wint_t wc);
int iswcntrl(wint_t wc);
int iswdigit(wint_t wc);
int iswgraph(wint_t wc);
int iswlower(wint_t wc);
int iswprint(wint_t wc);
int iswpunct(wint_t wc);
int iswspace(wint_t wc);
int iswupper(wint_t wc);
int iswxdigit(wint_t wc);

int iswblank(wint_t wc);

int iswctype(wint_t wc, wctype_t type);
wctype_t wctype(const char *property);

wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);

wint_t towctrans(wint_t wc, wctrans_t trans);
wctrans_t wctrans(const char *property);

#ifdef __cplusplus
}
#endif

#endif
