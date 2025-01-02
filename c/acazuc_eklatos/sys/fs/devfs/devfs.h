#ifndef DEVFS_H
#define DEVFS_H

#include <vfs.h>

int devfs_init(void);

extern struct fs_type g_devfs_type;

#endif
