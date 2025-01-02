#ifndef _ENV_H
#define _ENV_H

const char *envcmp(const char *env, const char *key, unsigned long key_len);
char *makeenv(const char *name, const char *value);

extern char **environ;

#endif
