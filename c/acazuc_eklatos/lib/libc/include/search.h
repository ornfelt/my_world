#ifndef SEARCH_H
#define SEARCH_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void *lfind(const void *key, const void *base, size_t *nmemb, size_t size,
            int (*compare)(const void *, const void *));
void *lsearch(const void *key, void *base, size_t *nmemb, size_t size,
              int (*compare)(const void *, const void *));

void insque(void *elem, void *prev);
void remque(void *elem);

#ifdef __cplusplus
}
#endif

#endif
