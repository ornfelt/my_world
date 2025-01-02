#ifndef ALLOCA_H
#define ALLOCA_H

#ifdef __cplusplus
extern "C" {
#endif

#define alloca(size) __builtin_alloca(size)

#ifdef __cplusplus
}
#endif

#endif
