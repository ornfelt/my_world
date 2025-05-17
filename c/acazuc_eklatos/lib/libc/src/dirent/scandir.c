#include "_dirent.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int
scandir(const char *dirp,
        struct dirent ***namelist,
        int (*filter)(const struct dirent *),
        int (*cmp)(const struct dirent **, const struct dirent **))
{
	struct dirent *dirent;
	size_t n = 0;
	DIR *dir;

	dir = opendir(dirp);
	if (!dir)
		return -1;
	*namelist = NULL;
	while ((dirent = readdir(dir)))
	{
		struct dirent *new_dirent;
		struct dirent **tmp;

		if (filter && !filter(dirent))
			continue;
		new_dirent = malloc(dirent->d_reclen);
		if (!new_dirent)
			goto err;
		memcpy(new_dirent, dirent, dirent->d_reclen);
		tmp = realloc(*namelist, sizeof(**namelist) * (n + 1));
		if (!tmp)
		{
			free(new_dirent);
			goto err;
		}
		*namelist = tmp;
		(*namelist)[n] = new_dirent;
		n++;
	}
	if (cmp)
		qsort(*namelist,
		      n,
		      sizeof(**namelist),
		      (int(*)(const void *, const void*))cmp);
	closedir(dir);
	return n;

err:
	for (size_t i = 0; i < n; ++i)
		free((*namelist)[i]);
	free(*namelist);
	closedir(dir);
	return -1;
}
