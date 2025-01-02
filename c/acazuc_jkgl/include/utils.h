#ifndef UTILS_H
#define UTILS_H

#include "gl.h"

#define DEF_MMMC(t, n) \
static inline GL##t mix##n(GL##t v1, GL##t v2, GLfloat f) \
{ \
	return v1 + f * (v2 - v1); \
} \
static inline GL##t max##n(GL##t v1, GL##t v2) \
{ \
	return v1 > v2 ? v1 : v2; \
} \
static inline GL##t min##n(GL##t v1, GL##t v2) \
{ \
	return v1 < v2 ? v1 : v2; \
} \
static inline GL##t clamp##n(GL##t v, GL##t min, GL##t max) \
{ \
	return min##n(max, max##n(min, v)); \
}

DEF_MMMC(byte, b)
DEF_MMMC(ubyte, ub)
DEF_MMMC(short, s)
DEF_MMMC(ushort, us)
DEF_MMMC(int, i)
DEF_MMMC(uint, ui)
DEF_MMMC(long, l)
DEF_MMMC(ulong, ul)
DEF_MMMC(float, f)
DEF_MMMC(double, d)

#endif
