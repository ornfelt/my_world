#include "game/guild.h"

#include "memory.h"

MEMORY_DECL(GENERIC);

static void member_destroy(void *ptr)
{
	struct guild_member *member = ptr;
	mem_free(MEM_GENERIC, member->name);
}

struct guild *guild_new(void)
{
	struct guild *guild = mem_zalloc(MEM_GENERIC, sizeof(*guild));
	if (!guild)
		return NULL;
	jks_array_init(&guild->members, sizeof(struct guild_member), member_destroy, &jks_array_memory_fn_GENERIC);
	return guild;
}

void guild_free(struct guild *guild)
{
	if (!guild)
		return;
	jks_array_destroy(&guild->members);
	mem_free(MEM_GENERIC, guild->name);
	mem_free(MEM_GENERIC, guild->motd);
	mem_free(MEM_GENERIC, guild);
}
