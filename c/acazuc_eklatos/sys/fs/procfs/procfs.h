#ifndef PROCFS_H
#define PROCFS_H

#include <vfs.h>

int procfs_init(void);

extern struct fs_type g_procfs_type;

#endif
