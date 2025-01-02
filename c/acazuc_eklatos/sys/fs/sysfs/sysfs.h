#ifndef SYSFS_H
#define SYSFS_H

#include <vfs.h>

int sysfs_init(void);

extern struct fs_type g_sysfs_type;

#endif
