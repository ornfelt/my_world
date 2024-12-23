#include "trs.h"

#include "common.h"
#include "mpq.h"

#include <stdlib.h>
#include <string.h>

struct wow_trs_file *wow_trs_file_new(struct wow_mpq_file *mpq)
{
	(void)mpq;
	struct wow_trs_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	file->dirs = NULL;
	file->dirs_nb = 0;
	char *org = (char*)mpq->data;
	size_t len = mpq->size;
	if (len >= 3 && ((uint8_t*)org)[0] == 0xEF && ((uint8_t*)org)[1] == 0xBB && ((uint8_t*)org)[2] == 0xBF) /* skip UTF-8 BOM */
	{
		org += 3;
		len -= 3;
	}
	char *ret;
	char *prv = org;
	while ((ret = (char*)memchr(prv, '\n', len - (prv - org))))
	{
		if (ret == prv)
		{
			prv++;
			continue;
		}
		size_t line_len = ret - prv;
		if (prv[line_len - 1] == '\r')
			line_len--;
		if (!line_len)
		{
			prv = ret;
			continue;
		}
		if (!strncmp(prv, "dir: ", 5))
		{
			struct wow_trs_file_dir *dirs = WOW_REALLOC(file->dirs, sizeof(*file->dirs) * (file->dirs_nb + 1));
			if (!dirs)
				goto err;
			file->dirs = dirs;
			file->dirs[file->dirs_nb].entries = NULL;
			file->dirs[file->dirs_nb].entries_nb = 0;
			file->dirs[file->dirs_nb].name = WOW_MALLOC(line_len - 4);
			if (!file->dirs[file->dirs_nb].name)
				goto err;
			memcpy(file->dirs[file->dirs_nb].name, prv + 5, line_len - 5);
			file->dirs[file->dirs_nb].name[line_len - 5] = '\0';
			file->dirs_nb++;
		}
		else
		{
			if (!file->dirs)
				goto err;
			const char *tab = memchr(prv, '\t', line_len);
			if (!tab)
				goto err;
			struct wow_trs_file_dir *dir = &file->dirs[file->dirs_nb - 1];
			struct wow_trs_file_entry *entries = WOW_REALLOC(dir->entries, sizeof(*dir->entries) * (dir->entries_nb + 1));
			if (!entries)
				goto err;
			dir->entries = entries;
			struct wow_trs_file_entry *entry = &dir->entries[dir->entries_nb];
			size_t name_len = tab - prv;
			entry->name = WOW_MALLOC(name_len + 1);
			entry->hash = WOW_MALLOC(line_len - (name_len + 1) + 1);
			if (!entry->name || !entry->hash)
				goto err;
			memcpy(entry->name, prv, name_len);
			entry->name[name_len] = '\0';
			memcpy(entry->hash, &tab[1], line_len - (name_len + 1));
			entry->hash[line_len - (name_len + 1)] = '\0';
			dir->entries_nb++;
		}
		prv = ret + 1;
	}
	return file;

err:
	wow_trs_file_delete(file);
	return NULL;
}

void wow_trs_file_delete(struct wow_trs_file *file)
{
	if (!file)
		return;
	for (size_t i = 0 ; i < file->dirs_nb; ++i)
	{
		struct wow_trs_file_dir *dir = &file->dirs[i];
		for (size_t j = 0; j < dir->entries_nb; ++j)
		{
			struct wow_trs_file_entry *entry = &dir->entries[j];
			WOW_FREE(entry->name);
			WOW_FREE(entry->hash);
		}
		WOW_FREE(dir->entries);
		WOW_FREE(dir->name);
	}
	WOW_FREE(file->dirs);
	WOW_FREE(file);
}
