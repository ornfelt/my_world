#include "wdl.h"

#include "common.h"
#include "mpq.h"

#include <string.h>
#include <stdlib.h>

struct wow_wdl_file *wow_wdl_file_new(struct wow_mpq_file *mpq)
{
	struct wow_wdl_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	if (wow_mpq_read(mpq, &file->mver, sizeof(file->mver)) != sizeof(file->mver))
		goto err;
	if (wow_mpq_read(mpq, &file->maof.header, sizeof(file->maof.header)) != sizeof(file->maof.header))
		goto err;
	if (wow_mpq_read(mpq, file->maof.offsets, file->maof.header.size) != file->maof.header.size)
		goto err;
	for (uint32_t z = 0; z < 64; ++z)
	{
		for (uint32_t x = 0; x < 64; ++x)
		{
			if (!file->maof.offsets[z][x])
			{
				memset(&file->mare[z][x], 0, sizeof(file->mare[z][x]));
				continue;
			}
			if (wow_mpq_seek(mpq, file->maof.offsets[z][x], SEEK_SET) == -1)
				goto err;
			if (wow_mpq_read(mpq, &file->mare[z][x], sizeof(file->mare[z][x])) != sizeof(file->mare[z][x]))
				goto err;
		}
	}
	return file;

err:
	wow_wdl_file_delete(file);
	return NULL;
}

void wow_wdl_file_delete(struct wow_wdl_file *file)
{
	if (!file)
		return;
	WOW_FREE(file);
}
