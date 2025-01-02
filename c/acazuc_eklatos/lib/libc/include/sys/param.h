#ifndef SYS_PARAM_H
#define SYS_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXPATHLEN 4096
#define PIPE_BUF 4096

#define _POSIX_AIO_LISTIO_MAX 2
#define _POSIX_AIO_MAX        1
#define _POSIX_ARG_MAX        4096
#define _POSIX_CHILD_MAX      6
#define _POSIX_NGROUPS_MAX    0
#define _POSIX_OPEN_MAX       16
#define _POSIX_SSIZE_MAX      32767
#define _POSIX_STREAM_MAX     8
#define _POSIX_TZNAME_MAX     3
#define _POSIX2_RE_DUP_MAX    255
#define _POSIX_HOST_NAME_MAX  255
#define _POSIX_LOGIN_NAME_MAX 9
#define _POSIX_SYMLOOP_MAX    64
#define _POSIX_TTY_NAME_MAX   9

#define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define PDP_ENDIAN __ORDER_PDP_ENDIAN__
#define BYTE_ORDER __BYTE_ORDER__

#ifdef __cplusplus
}
#endif

#endif
