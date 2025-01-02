#include <sys/stat.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fetch.h>

FILE *fetchGetFile(struct url *url, const char *flags)
{
	(void)flags;
	return fopen(url->doc, "rb");
}

FILE *fetchPutFile(struct url *url, const char *flags)
{
	const char *mode;
	if (flags && strchr(flags, 'a'))
		mode = "w+b";
	else
		mode = "wb";
	return fopen(url->doc, mode);
}

int fetchStatFile(struct url *url, struct url_stat *ustat, const char *flags)
{
	struct stat st;

	(void)flags;
	if (stat(url->doc, &st) == -1)
		return -1;
	ustat->size = st.st_size;
	ustat->atime = st.st_atime;
	ustat->mtime = st.st_mtime;
	return 0;
}

struct url_ent *fetchListFile(struct url *url, const char *flags)
{
	(void)flags;
	DIR *dir = opendir(url->doc);
	if (!dir)
		return NULL;
	size_t entries_count = 0;
	struct url_ent *entries = malloc(sizeof(*entries));
	if (!entries)
	{
		closedir(dir);
		return NULL;
	}
	entries[0].name[0] = '\0';
	struct dirent *dirent;
	while ((dirent = readdir(dir)))
	{
		char path[MAXPATHLEN];
		snprintf(path, sizeof(path), "%s/%s", url->doc, dirent->d_name);
		struct stat st;
		if (stat(path, &st) == -1)
			continue;
		struct url_ent *tmp = realloc(entries, sizeof(*tmp) * (entries_count + 2));
		if (!tmp)
		{
			free(entries);
			closedir(dir);
			return NULL;
		}
		entries = tmp;
		strlcpy(entries[entries_count].name, dirent->d_name, sizeof(entries[entries_count].name));
		entries[entries_count].stat.size = st.st_size;
		entries[entries_count].stat.atime = st.st_atime;
		entries[entries_count].stat.mtime = st.st_mtime;
		entries_count++;
		entries[entries_count].name[0] = '\0';
	}
	closedir(dir);
	return entries;
}
