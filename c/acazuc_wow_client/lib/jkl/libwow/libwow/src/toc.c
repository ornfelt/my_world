#include "toc.h"

#include "common.h"

#include <string.h>
#include <stdlib.h>

#define TOKEN_INTERFACE TOKEN_VAR " Interface: "
#define TOKEN_TITLE TOKEN_VAR " Title: "
#define TOKEN_NOTES TOKEN_VAR " Notes: "
#define TOKEN_DEPS TOKEN_VAR " Dependencies: "
#define TOKEN_URL TOKEN_VAR " URL: "
#define TOKEN_VAR "##"
#define TOKEN_COMMENT "#"

struct wow_toc_file *wow_toc_file_new(const uint8_t *data, uint32_t len)
{
	struct wow_toc_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	memset(file, 0, sizeof(*file));
	char *org = (char*)data;
	if (len >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) /* skip UTF-8 BOM */
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
		if (!strncmp(prv, TOKEN_INTERFACE, strlen(TOKEN_INTERFACE)))
		{
			char *src = prv + strlen(TOKEN_INTERFACE);
			file->version = strtoll(src, NULL, 10);
		}
		else if (!strncmp(prv, TOKEN_TITLE, strlen(TOKEN_TITLE)))
		{
			char *src = prv + strlen(TOKEN_TITLE);
			size_t var_len = line_len - strlen(TOKEN_TITLE);
			file->title = WOW_MALLOC(var_len + 1);
			if (!file->title)
				goto err;
			memcpy(file->title, src, var_len);
			file->title[var_len] = '\0';
		}
		else if (!strncmp(prv, TOKEN_NOTES, strlen(TOKEN_NOTES)))
		{
			char *src = prv + strlen(TOKEN_NOTES);
			size_t var_len = line_len - strlen(TOKEN_NOTES);
			file->notes = WOW_MALLOC(var_len + 1);
			if (!file->notes)
				goto err;
			memcpy(file->notes, src, var_len);
			file->notes[var_len] = '\0';
		}
		else if (!strncmp(prv, TOKEN_DEPS, strlen(TOKEN_DEPS)))
		{
			char *src = prv + strlen(TOKEN_DEPS);
			size_t var_len = line_len - strlen(TOKEN_DEPS);
			file->deps = WOW_MALLOC(var_len + 1);
			if (!file->deps)
				goto err;
			memcpy(file->deps, src, var_len);
			file->deps[var_len] = '\0';
		}
		else if (!strncmp(prv, TOKEN_URL, strlen(TOKEN_URL)))
		{
			char *src = prv + strlen(TOKEN_URL);
			size_t var_len = line_len - strlen(TOKEN_URL);
			file->url = WOW_MALLOC(var_len + 1);
			if (!file->url)
				goto err;
			memcpy(file->url, src, var_len);
			file->url[var_len] = '\0';
		}
		else if (!strncmp(prv, TOKEN_VAR, strlen(TOKEN_VAR)))
		{
			/* unknown var */
		}
		else if (!strncmp(prv, TOKEN_COMMENT, strlen(TOKEN_COMMENT)))
		{
			/* comment */
		}
		else
		{
			char **new_files = WOW_REALLOC(file->files, sizeof(*new_files) * (file->files_nb + 1));
			if (!new_files)
				goto err;
			file->files = new_files;
			file->files[file->files_nb] = WOW_MALLOC(line_len + 1);
			if (!file->files[file->files_nb])
				goto err;
			memcpy(file->files[file->files_nb], prv, line_len);
			file->files[file->files_nb][line_len] = '\0';
			file->files_nb++;
		}
		prv = ret + 1;
	}
	return file;

err:
	wow_toc_file_delete(file);
	return NULL;
}

void wow_toc_file_delete(struct wow_toc_file *file)
{
	if (!file)
		return;
	for (size_t i = 0; i < file->files_nb; ++i)
		WOW_FREE(file->files[i]);
	WOW_FREE(file->files);
	WOW_FREE(file->title);
	WOW_FREE(file->notes);
	WOW_FREE(file->deps);
	WOW_FREE(file->url);
	WOW_FREE(file);
}
