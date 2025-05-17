#ifndef _GRP_H
#define _GRP_H

#include <stdio.h>

extern struct group grp_ent;
extern char grp_buf[1024];
extern FILE *grent_fp;

int parse_grline(struct group *group,
                 char *buf,
                 size_t buflen,
                 const char *line);
int search_grnam(struct group *grp,
                 char *buf,
                 size_t buflen,
                 struct group **result,
                 int (*cmp_fn)(struct group *grp, const void *ptr),
                 const void *cmp_ptr);

#endif
