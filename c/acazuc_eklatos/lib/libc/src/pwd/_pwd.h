#ifndef _PWD_H
#define _PWD_H

#include <stdio.h>
#include <pwd.h>

extern struct passwd pwd_ent;
extern char pwd_buf[1024];
extern FILE *pwent_fp;

int parse_pwline(struct passwd *passwd,
                 char *buf,
                 size_t buflen,
                 const char *line);
int search_pwnam(struct passwd *pwd,
                 char *buf,
                 size_t buflen,
                 struct passwd **result,
                 int (*cmp_fn)(struct passwd *pwd, const void *ptr),
                 const void *cmp_ptr);

#endif
