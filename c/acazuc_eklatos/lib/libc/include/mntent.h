#ifndef MNTENT_H
#define MNTENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define _PATH_MOUNTED "/etc/mtab"
#define _PATH_MNTTAB  "/etc/fstab"

struct mntent
{
	char *mnt_fsname;
	char *mnt_dir;
	char *mnt_type;
	char *mnt_opts;
	int mnt_freq;
	int mnt_passno;
};

FILE *setmnt(const char *file, const char *type);
struct mntent *getmntent(FILE *fp);
int addmntent(FILE *fp, const struct mntent *mnt);
int endmntent(FILE *fp);
char *hasmntopt(const struct mntent *mnt, const char *opt);

#ifdef __cplusplus
}
#endif

#endif
