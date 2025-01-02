#ifndef _WCSTO_H
#define _WCSTO_H

#include <wchar.h>

int wcsto_getc(const wchar_t *it, int base, wchar_t *c);
const wchar_t *wcsto_init(const wchar_t *nptr, int *base, int *is_neg);

#endif
