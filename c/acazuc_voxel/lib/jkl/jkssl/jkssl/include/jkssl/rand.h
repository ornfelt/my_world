#ifndef JKSSL_RAND_H
#define JKSSL_RAND_H

#include <stdint.h>
#include <stddef.h>

int rand_bytes(void *data, size_t size);

#endif
