#ifndef _ALLOC_H
#define _ALLOC_H

#include <stddef.h>

void *_malloc(size_t size);
void *_realloc(void *ptr, size_t size);
void _free(void *ptr);

#endif
