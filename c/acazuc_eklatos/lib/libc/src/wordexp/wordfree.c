#include <wordexp.h>
#include <stdlib.h>

void
wordfree(wordexp_t *we)
{
	if (!we || !we->we_wordv)
		return;
	for (size_t i = 0; i < we->we_offs + we->we_wordc; ++i)
		free(we->we_wordv[i]);
	free(we->we_wordv);
	we->we_wordv = NULL;
	we->we_wordc = 0;
}
