#ifndef JKGL_UTILS_H
#define JKGL_UTILS_H

#include <stdint.h>

#define DEF_MMMC(t, n) \
static inline t mix##n(t v1, t v2, float f) \
{ \
	return v1 + f * (v2 - v1); \
} \
static inline t max##n(t v1, t v2) \
{ \
	return v1 > v2 ? v1 : v2; \
} \
static inline t min##n(t v1, t v2) \
{ \
	return v1 < v2 ? v1 : v2; \
} \
static inline t clamp##n(t v, t min, t max) \
{ \
	return min##n(max, max##n(min, v)); \
}

DEF_MMMC(int8_t, b)
DEF_MMMC(uint8_t, ub)
DEF_MMMC(int16_t, s)
DEF_MMMC(uint16_t, us)
DEF_MMMC(int32_t, i)
DEF_MMMC(uint32_t, ui)
DEF_MMMC(int64_t, l)
DEF_MMMC(uint64_t, ul)
DEF_MMMC(float, f)
DEF_MMMC(double, d)

#endif
