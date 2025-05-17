#ifndef LIBH261_H261_H
#define LIBH261_H261_H

#include <stdint.h>
#include <stdio.h>

struct h261;

struct h261 *h261_new(void);
void h261_free(struct h261 *h261);
void h261_init_io(struct h261 *h261, FILE *fp);
int h261_decode_frame(struct h261 *h261, 
                      void *data,
                      uint32_t *width,
                      uint32_t *height);
const char *h261_get_err(struct h261 *h261);

#endif
