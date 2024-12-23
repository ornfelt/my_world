#ifndef CVARS_H
#define CVARS_H

#include <stdbool.h>

#include <jks/hmap.h>

struct cvars
{
	struct jks_hmap vars;
};

struct cvars *cvars_new(void);
void cvars_free(struct cvars *cvars);
const char *cvar_get(struct cvars *cvars, const char *key);
bool cvar_set(struct cvars *cvars, const char *key, const char *val);

#endif
