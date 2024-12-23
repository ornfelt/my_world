#include "wmo_group.h"

#include "common.h"
#include "mpq.h"

#include <string.h>
#include <stdlib.h>

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

struct wow_wmo_group_file *wow_wmo_group_file_new(struct wow_mpq_file *mpq)
{
	struct wow_wmo_group_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	memset(file, 0, sizeof(*file));
	READ_FLAT_CHUNK(file->mver, "REVM");
	READ_FLAT_CHUNK(file->mogp, "PGOM");
	READ_ARRAY_CHUNK(file->mopy, "YPOM");
	READ_ARRAY_CHUNK(file->movi, "IVOM");
	READ_ARRAY_CHUNK(file->movt, "TVOM");
	READ_ARRAY_CHUNK(file->monr, "RNOM");
	READ_ARRAY_CHUNK(file->motv, "VTOM");
	READ_ARRAY_CHUNK(file->moba, "ABOM");
	if (file->mogp.flags & WOW_MOGP_FLAGS_LIGHT)
		READ_ARRAY_CHUNK(file->molr, "RLOM");
	if (file->mogp.flags & WOW_MOGP_FLAGS_DOODAD)
		READ_ARRAY_CHUNK(file->modr, "RDOM");
	if (file->mogp.flags & WOW_MOGP_FLAGS_BSP)
	{
		READ_ARRAY_CHUNK(file->mobn, "NBOM");
		READ_ARRAY_CHUNK(file->mobr, "RBOM");
	}
	if (file->mogp.flags & WOW_MOGP_FLAGS_COLOR)
		READ_ARRAY_CHUNK(file->mocv, "VCOM");
	if (file->mogp.flags & WOW_MOGP_FLAGS_LIQUID)
	{
		if (wow_mpq_read(mpq, &file->mliq.header, sizeof(file->mliq.header) - 2) != sizeof(file->mliq.header) - 2)
			goto err;
		if (memcmp(&file->mliq.header.header.magic, "QILM", 4))
			goto err;
		file->mliq.vertexes_nb = file->mliq.header.xverts * file->mliq.header.yverts;
		file->mliq.vertexes = WOW_MALLOC(file->mliq.header.xverts * file->mliq.header.yverts * sizeof(*file->mliq.vertexes));
		if (!file->mliq.vertexes)
			goto err;
		if (wow_mpq_read(mpq, file->mliq.vertexes, file->mliq.header.xverts * file->mliq.header.yverts * sizeof(*file->mliq.vertexes)) != file->mliq.header.xverts * file->mliq.header.yverts * sizeof(*file->mliq.vertexes))
			goto err;
		file->mliq.tiles_nb = file->mliq.header.xtiles * file->mliq.header.ytiles;
		file->mliq.tiles = WOW_MALLOC(file->mliq.header.xtiles * file->mliq.header.ytiles * sizeof(*file->mliq.tiles));
		if (!file->mliq.tiles)
			goto err;
		if (wow_mpq_read(mpq, file->mliq.tiles, file->mliq.header.xtiles * file->mliq.header.ytiles * sizeof(*file->mliq.tiles)) != file->mliq.header.xtiles * file->mliq.header.ytiles * sizeof(*file->mliq.tiles))
			goto err;
	}
	return file;
err:
	wow_wmo_group_file_delete(file);
	return NULL;
}

void wow_wmo_group_file_delete(struct wow_wmo_group_file *file)
{
	if (!file)
		return;
	WOW_FREE(file->molr.data);
	WOW_FREE(file->modr.data);
	WOW_FREE(file->mobn.data);
	WOW_FREE(file->mobr.data);
	WOW_FREE(file->mocv.data);
	WOW_FREE(file->mliq.vertexes);
	WOW_FREE(file->mliq.tiles);
	WOW_FREE(file->moba.data);
	WOW_FREE(file->motv.data);
	WOW_FREE(file->monr.data);
	WOW_FREE(file->movt.data);
	WOW_FREE(file->movi.data);
	WOW_FREE(file->mopy.data);
	WOW_FREE(file);
}
