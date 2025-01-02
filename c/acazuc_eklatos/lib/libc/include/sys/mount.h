#ifndef SYS_MOUNT_H
#define SYS_MOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

int mount(const char *source, const char *target, const char *type,
          unsigned long flags, const void *data);

#ifdef __cplusplus
}
#endif

#endif
