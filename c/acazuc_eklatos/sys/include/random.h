#ifndef RANDOM_H
#define RANDOM_H

#include <types.h>

typedef ssize_t (*random_collect_t)(void *buf, size_t size, void *userdata);

int random_register(random_collect_t collect, void *userdata);
ssize_t random_get(void *dst, size_t count);
void random_init(void);

#endif
