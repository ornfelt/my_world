#ifndef GUILD_H
#define GUILD_H

#include <stdint.h>

#include <jks/array.h>

struct guild_rank
{
};

struct guild_member
{
	uint64_t guid;
	char *name;
};

struct guild
{
	struct jks_array members; /* struct guild_member */
	char *name;
	char *motd;
	uint64_t guid;
};

struct guild *guild_new(void);
void guild_free(struct guild *guild);

#endif
