#include <sys/param.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

static int print_dynamic_section(struct env *env, struct elfN *elf,
                                 const ElfN_Shdr *shdr)
{
	uint8_t *data;
	if (elfN_read_section(elf, shdr, (void**)&data, NULL))
	{
		fprintf(stderr, "%s: failed to read section: %s\n",
		        env->progname, strerror(errno));
		return 1;
	}
	for (ElfN_Off i = 0; i < shdr->sh_size; i += shdr->sh_entsize)
	{
		const ElfN_Dyn *dyn = (const ElfN_Dyn*)&data[i];
		if (dyn->d_tag == DT_NULL)
			break;
		if (dyn->d_tag != DT_NEEDED)
			continue;
		const char *name = elfN_get_dynstr_str(elf, dyn->d_un.d_val);
		char path[MAXPATHLEN];
		if (!get_path(path, sizeof(path), name))
			strlcat(path, "not found", sizeof(path));
		printf("%s => %s\n", name, path);
	}
	return 0;
}

int print_elfN(struct env *env, struct elfN *elf)
{
	for (ElfN_Word i = 0; i < elfN_get_shnum(elf); ++i)
	{
		const ElfN_Shdr *shdr = elfN_get_shdr(elf, i);
		if (shdr->sh_type != SHT_DYNAMIC)
			continue;
		if (print_dynamic_section(env, elf, shdr))
			return 1;
	}
	return 0;
}
