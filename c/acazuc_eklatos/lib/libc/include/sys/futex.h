#ifndef SYS_FUTEX_H
#define SYS_FUTEX_H

#define FUTEX_PRIVATE_FLAG   (1 << 7)
#define FUTEX_CLOCK_REALTIME (1 << 8)

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1

#define FUTEX_WAIT_PRIVATE (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)

#ifdef __cplusplus
extern "C" {
#endif

struct timespec;

int futex(int *uaddr, int op, int val, const struct timespec *timeout);

#ifdef __cplusplus
}
#endif

#endif
