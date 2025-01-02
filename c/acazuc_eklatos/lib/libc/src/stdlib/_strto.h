#ifndef _STRTO_H
#define _STRTO_H

int strto_getc(const char *it, int base, char *c);
const char *strto_init(const char *nptr, int *base, int *is_neg);

#endif
