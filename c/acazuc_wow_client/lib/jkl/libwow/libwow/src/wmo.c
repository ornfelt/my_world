#include "wmo.h"

#include "wmo_group.h"
#include "common.h"
#include "mpq.h"

#include <stdlib.h>
#include <string.h>

#define READ_FLAT_CHUNK(chunk, magic_str) \
do \
{ \
	if (wow_mpq_read(mpq, &(chunk), sizeof(chunk)) != sizeof(chunk)) \
		goto err; \
	if (memcmp(&(chunk).header.magic, magic_str, 4)) \
		goto err; \
} while (0)

#define READ_ARRAY_CHUNK(chunk, magic_str) \
do \
{ \
	if (wow_mpq_read(mpq, &(chunk).header, sizeof((chunk).header)) != sizeof((chunk).header)) \
		goto err; \
	if (memcmp(&(chunk).header.magic, magic_str, 4)) \
		goto err; \
	(chunk).size = (chunk).header.size / sizeof(*(chunk).data); \
	(chunk).data = WOW_MALLOC((chunk).header.size); \
	if (!(chunk).data) \
		goto err; \
	if (wow_mpq_read(mpq, (chunk).data, (chunk).header.size) != (chunk).header.size) \
		goto err; \
} while (0)

struct wow_wmo_file *wow_wmo_file_new(struct wow_mpq_file *mpq)
{
	struct wow_wmo_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	memset(file, 0, sizeof(*file));
	READ_FLAT_CHUNK(file->mver, "REVM");
	READ_FLAT_CHUNK(file->mohd, "DHOM");
	READ_ARRAY_CHUNK(file->motx, "XTOM");
	READ_ARRAY_CHUNK(file->momt, "TMOM");
	READ_ARRAY_CHUNK(file->mogn, "NGOM");
	READ_ARRAY_CHUNK(file->mogi, "IGOM");
	READ_ARRAY_CHUNK(file->mosb, "BSOM");
	READ_ARRAY_CHUNK(file->mopv, "VPOM");
	READ_ARRAY_CHUNK(file->mopt, "TPOM");
	READ_ARRAY_CHUNK(file->mopr, "RPOM");
	READ_ARRAY_CHUNK(file->movv, "VVOM");
	READ_ARRAY_CHUNK(file->movb, "BVOM");
	READ_ARRAY_CHUNK(file->molt, "TLOM");
	READ_ARRAY_CHUNK(file->mods, "SDOM");
	READ_ARRAY_CHUNK(file->modn, "NDOM");
	READ_ARRAY_CHUNK(file->modd, "DDOM");
	READ_ARRAY_CHUNK(file->mfog, "GOFM");
	if (wow_mpq_read(mpq, &file->mcvp.header, sizeof(file->mcvp.header)) > 0)
	{
		if (memcmp(&file->mcvp.header.magic, "PVCM", 4))
			goto err;
		file->mcvp.size = file->mcvp.header.size / sizeof(*file->mcvp.data);
		file->mcvp.data = WOW_MALLOC(file->mcvp.header.size);
		if (!file->mcvp.data)
			goto err;
		if (wow_mpq_read(mpq, file->mcvp.data, file->mcvp.header.size) != file->mcvp.header.size)
			goto err;
	}
	return file;

err:
	wow_wmo_file_delete(file);
	return NULL;
}

void wow_wmo_file_delete(struct wow_wmo_file *file)
{
	if (!file)
		return;
	WOW_FREE(file->motx.data);
	WOW_FREE(file->momt.data);
	WOW_FREE(file->mogi.data);
	WOW_FREE(file->mopv.data);
	WOW_FREE(file->mopt.data);
	WOW_FREE(file->mopr.data);
	WOW_FREE(file->movv.data);
	WOW_FREE(file->movb.data);
	WOW_FREE(file->molt.data);
	WOW_FREE(file->mods.data);
	WOW_FREE(file->modn.data);
	WOW_FREE(file->modd.data);
	WOW_FREE(file->mfog.data);
	WOW_FREE(file->mcvp.data);
	WOW_FREE(file->mogn.data);
	WOW_FREE(file->mosb.data);
	WOW_FREE(file);
}
