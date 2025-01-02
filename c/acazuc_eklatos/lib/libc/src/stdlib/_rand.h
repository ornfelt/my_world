#ifndef _RAND_H
#define _RAND_H

#include <stdint.h>

extern uint32_t rand_val;
extern uint16_t rand48_x[3];
extern uint64_t rand48_a;
extern uint64_t rand48_c;
extern uint32_t *random_val;

uint64_t rand48(uint16_t x[3]);

#endif
