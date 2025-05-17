#include "wdt.h"

#include "common.h"
#include "mpq.h"

#include <string.h>
#include <stdlib.h>

struct wow_wdt_file *
wow_wdt_file_new(struct wow_mpq_file *mpq)
{
	struct wow_wdt_file *file;

	file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	file->modf.data = NULL;
	file->mwmo.data = NULL;
	if (wow_mpq_read(mpq, &file->mver, sizeof(file->mver)) != sizeof(file->mver))
		goto err;
	if (memcmp(&file->mver.header.magic, "REVM", 4))
		goto err;
	if (wow_mpq_read(mpq, &file->mphd, sizeof(file->mphd)) != sizeof(file->mphd))
		goto err;
	if (memcmp(&file->mphd.header.magic, "DHPM", 4))
		goto err;
	if (wow_mpq_read(mpq, &file->main, sizeof(file->main)) != sizeof(file->main))
		goto err;
	if (memcmp(&file->main.header.magic, "NIAM", 4))
		goto err;
	if (wow_mpq_read(mpq, &file->mwmo.header, sizeof(file->mwmo.header)) != sizeof(file->mwmo.header))
		return file; /* MWMO is optional */
	if (file->mwmo.header.size)
	{
		file->mwmo.size = file->mwmo.header.size;
		file->mwmo.data = WOW_MALLOC(file->mwmo.size);
		if (!file->mwmo.data)
			goto err;
		if (wow_mpq_read(mpq, file->mwmo.data, file->mwmo.header.size) != file->mwmo.header.size)
			goto err;
		if (wow_mpq_read(mpq, &file->modf.header, sizeof(file->modf.header)) != sizeof(file->modf.header))
			goto err;
		file->modf.size = file->modf.header.size / sizeof(*file->modf.data);
		file->modf.data = WOW_MALLOC(sizeof(*file->modf.data) * file->modf.size);
		if (!file->modf.data)
			goto err;
		if (wow_mpq_read(mpq, file->modf.data, file->modf.header.size) != file->modf.header.size)
			goto err;
	}
	return file;

err:
	wow_wdt_file_delete(file);
	return NULL;
}

void
wow_wdt_file_delete(struct wow_wdt_file *file)
{
	if (!file)
		return;
	WOW_FREE(file->mwmo.data);
	WOW_FREE(file->modf.data);
	WOW_FREE(file);
}
