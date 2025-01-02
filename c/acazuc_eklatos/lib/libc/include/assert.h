#ifdef assert
#undef assert
#endif

#ifdef static_assert
#undef static_assert
#endif

#ifndef NDEBUG

#include <stdlib.h>
#include <stdio.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG

#ifndef __ASSERT_FAILURE
#define __ASSERT_FAILURE

void __assert_fail(const char *expr, const char *file, unsigned line,
                   const char *func);

#endif

#define assert(e) (__builtin_expect(!!(e), 1) ? (void)0 : __assert_fail(#e, __FILE__, __LINE__, __func__))

#else

#define assert(e)

#endif

#ifndef __cplusplus
#define static_assert _Static_assert
#endif

#ifdef __cplusplus
}
#endif
