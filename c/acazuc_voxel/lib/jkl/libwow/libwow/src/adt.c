#include "adt.h"

#include "common.h"
#include "mpq.h"

#include <string.h>
#include <stdlib.h>

#define READ_ADT_CHUNK(chunk, magic_str, offset) \
do \
{ \
	if (wow_mpq_seek(mpq, offset, SEEK_SET) == -1) \
		goto err; \
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

struct wow_adt_file *wow_adt_file_new(struct wow_mpq_file *mpq)
{
	struct wow_adt_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	memset(file, 0, sizeof(*file));
	if (wow_mpq_read(mpq, &file->mver, sizeof(file->mver)) != sizeof(file->mver))
		goto err;
	if (memcmp(&file->mver.header.magic, "REVM", 4))
		goto err;
	if (wow_mpq_read(mpq, &file->mhdr, sizeof(file->mhdr)) != sizeof(file->mhdr))
		goto err;
	if (memcmp(&file->mhdr.header.magic, "RDHM", 4))
		goto err;
	size_t mhdr_base = sizeof(file->mver) + sizeof(struct wow_chunk_header);
	if (wow_mpq_seek(mpq, mhdr_base + file->mhdr.mcin, SEEK_SET) == -1)
		goto err;
	if (wow_mpq_read(mpq, &file->mcin, sizeof(file->mcin)) != sizeof(file->mcin))
		goto err;
	if (memcmp(&file->mcin.header.magic, "NICM", 4))
		goto err;
	READ_ADT_CHUNK(file->mwmo, "OMWM", mhdr_base + file->mhdr.mwmo);
	READ_ADT_CHUNK(file->mwid, "DIWM", mhdr_base + file->mhdr.mwid);
	READ_ADT_CHUNK(file->modf, "FDOM", mhdr_base + file->mhdr.modf);
	READ_ADT_CHUNK(file->mmdx, "XDMM", mhdr_base + file->mhdr.mmdx);
	READ_ADT_CHUNK(file->mmid, "DIMM", mhdr_base + file->mhdr.mmid);
	READ_ADT_CHUNK(file->mddf, "FDDM", mhdr_base + file->mhdr.mddf);
	for (size_t i = 0; i < 16 * 16; ++i)
	{
		struct wow_mcin_data *mcin = &file->mcin.data[i];
		struct wow_mcnk *mcnk = &file->mcnk[i];
		if (wow_mpq_seek(mpq, mcin->offset, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->header, sizeof(mcnk->header)) != sizeof(mcnk->header))
			goto err;
		if (memcmp(&mcnk->header.header.magic, "KNCM", 4))
			goto err;
		if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcvt, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->mcvt, sizeof(mcnk->mcvt)) != sizeof(mcnk->mcvt))
			goto err;
		if (memcmp(&mcnk->mcvt.header.magic, "TVCM", 4))
			goto err;
		if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcnr, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->mcnr, sizeof(mcnk->mcnr)) != sizeof(mcnk->mcnr))
			goto err;
		if (memcmp(&mcnk->mcnr.header.magic, "RNCM", 4))
			goto err;
		if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcly, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->mcly.header, sizeof(mcnk->mcly.header)) != sizeof(mcnk->mcly.header))
			goto err;
		if (memcmp(&mcnk->mcly.header.magic, "YLCM", 4))
			goto err;
		if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcrf, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->mcrf.header, sizeof(mcnk->mcrf.header)) != sizeof(mcnk->mcrf.header))
			goto err;
		if (memcmp(&mcnk->mcrf.header.magic, "FRCM", 4))
			goto err;
		mcnk->mcrf.doodads_nb = mcnk->header.doodad_refs_nb;
		mcnk->mcrf.doodads = WOW_MALLOC(sizeof(*mcnk->mcrf.doodads) * mcnk->mcrf.doodads_nb);
		if (mcnk->mcrf.doodads == NULL)
			goto err;
		if (wow_mpq_read(mpq, mcnk->mcrf.doodads, sizeof(*mcnk->mcrf.doodads) * mcnk->mcrf.doodads_nb) != sizeof(*mcnk->mcrf.doodads) * mcnk->mcrf.doodads_nb)
			goto err;
		mcnk->mcrf.wmos_nb = mcnk->header.map_obj_refs_nb;
		mcnk->mcrf.wmos = WOW_MALLOC(sizeof(*mcnk->mcrf.wmos) * mcnk->mcrf.wmos_nb);
		if (!mcnk->mcrf.wmos)
			goto err;
		if (wow_mpq_read(mpq, mcnk->mcrf.wmos, sizeof(*mcnk->mcrf.wmos) * mcnk->mcrf.wmos_nb) != sizeof(*mcnk->mcrf.wmos) * mcnk->mcrf.wmos_nb)
			goto err;
		if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcal, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->mcal.header, sizeof(mcnk->mcal.header)) != sizeof(mcnk->mcal.header))
			goto err;
		if (memcmp(&mcnk->mcal.header.magic, "LACM", 4))
			goto err;
		if (mcnk->header.flags & WOW_MCNK_FLAGS_MCSH)
		{
			if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcsh, SEEK_SET) == -1)
				goto err;
			if (wow_mpq_read(mpq, &mcnk->mcsh, sizeof(mcnk->mcsh)) != sizeof(mcnk->mcsh))
				goto err;
			if (memcmp(&mcnk->mcsh.header.magic, "HSCM", 4))
				goto err;
		}
		if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcse, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->mcse.header, sizeof(mcnk->mcse.header)) != sizeof(mcnk->mcse.header))
			goto err;
		if (memcmp(&mcnk->mcse.header.magic, "ESCM", 4))
			goto err;
		if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mclq, SEEK_SET) == -1)
			goto err;
		if (wow_mpq_read(mpq, &mcnk->mclq.header, sizeof(mcnk->mclq.header)) != sizeof(mcnk->mclq.header))
			goto err;
		if (memcmp(&mcnk->mclq.header.magic, "QLCM", 4))
			goto err;
		mcnk->mclq.size = 0;
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_RIVER)
			mcnk->mclq.size++;
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_OCEAN)
			mcnk->mclq.size++;
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_MAGMA)
			mcnk->mclq.size++;
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_SLIME)
			mcnk->mclq.size++;
		mcnk->mclq.data = WOW_MALLOC(sizeof(*mcnk->mclq.data) * mcnk->mclq.size);
		if (!mcnk->mclq.data)
			goto err;
		uint32_t nlq = 0;
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_RIVER)
		{
			if (wow_mpq_read(mpq, &mcnk->mclq.data[nlq++], sizeof(*mcnk->mclq.data)) != sizeof(*mcnk->mclq.data))
				goto err;
		}
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_OCEAN)
		{
			if (wow_mpq_read(mpq, &mcnk->mclq.data[nlq++], sizeof(*mcnk->mclq.data)) != sizeof(*mcnk->mclq.data))
				goto err;
		}
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_MAGMA)
		{
			if (wow_mpq_read(mpq, &mcnk->mclq.data[nlq++], sizeof(*mcnk->mclq.data)) != sizeof(*mcnk->mclq.data))
				goto err;
		}
		if (mcnk->header.flags & WOW_MCNK_FLAGS_LQ_SLIME)
		{
			if (wow_mpq_read(mpq, &mcnk->mclq.data[nlq++], sizeof(*mcnk->mclq.data)) != sizeof(*mcnk->mclq.data))
				goto err;
		}
		/* wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mccv, SEEK_SET);
		 * wow_mpq_read(mpq, &mcnk->mccv, sizeof(mcnk->mccv));
		 * wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mclv, SEEK_SET);
		 * wow_mpq_read(mpq, &mcnk->mclv, sizeof(mcnk->mclv));
		 */
		if (mcnk->header.layers)
		{
			mcnk->mcal.size = mcnk->mcal.header.size;
			mcnk->mcal.data = WOW_MALLOC(mcnk->mcal.size);
			if (!mcnk->mcal.data)
				goto err;
			if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcal + sizeof(mcnk->mcal.header), SEEK_SET) == -1)
				goto err;
			if (wow_mpq_read(mpq, mcnk->mcal.data, mcnk->mcal.size) != mcnk->mcal.size)
				goto err;
			mcnk->mcly.size = mcnk->header.layers;
			mcnk->mcly.data = WOW_MALLOC(sizeof(*mcnk->mcly.data) * mcnk->mcly.size);
			if (!mcnk->mcly.data)
				goto err;
			for (uint8_t l = 0; l < mcnk->header.layers; ++l)
			{
				if (wow_mpq_seek(mpq, mcin->offset + mcnk->header.ofs_mcly + sizeof(mcnk->mcly.header) + l * sizeof(*mcnk->mcly.data), SEEK_SET) == -1)
					goto err;
				if (wow_mpq_read(mpq, &mcnk->mcly.data[l], sizeof(*mcnk->mcly.data)) != sizeof(*mcnk->mcly.data))
					goto err;
			}
		}
	}
	if (wow_mpq_seek(mpq, mhdr_base + file->mhdr.mtex, SEEK_SET) == -1)
		goto err;
	if (wow_mpq_read(mpq, &file->mtex.header, sizeof(file->mtex.header)) != sizeof(file->mtex.header))
		goto err;
	if (memcmp(&file->mtex.header.magic, "XETM", 4))
		goto err;
	file->mtex.size = file->mtex.header.size;
	file->mtex.data = WOW_MALLOC(file->mtex.size);
	if (!file->mtex.data)
		goto err;
	if (wow_mpq_read(mpq, file->mtex.data, file->mtex.size) != file->mtex.size)
		goto err;
	file->textures_nb = 0;
	for (char *tmp = file->mtex.data; tmp - file->mtex.data < file->mtex.size; tmp += strlen(tmp) + 1)
		file->textures_nb++;
	file->textures = WOW_MALLOC(sizeof(*file->textures) * file->textures_nb);
	if (!file->textures)
		goto err;
	uint32_t i = 0;
	for (char *tmp = file->mtex.data; tmp - file->mtex.data < file->mtex.size; tmp += strlen(tmp) + 1)
		file->textures[i++] = tmp;
	return file;

err:
	wow_adt_file_delete(file);
	return NULL;
}

void wow_adt_file_delete(struct wow_adt_file *file)
{
	if (!file)
		return;
	WOW_FREE(file->mwmo.data);
	WOW_FREE(file->modf.data);
	WOW_FREE(file->mmdx.data);
	WOW_FREE(file->mmid.data);
	WOW_FREE(file->mddf.data);
	WOW_FREE(file->mwid.data);
	for (uint32_t i = 0; i < 16 * 16; ++i)
	{
		struct wow_mcnk *mcnk = &file->mcnk[i];
		WOW_FREE(mcnk->mcly.data);
		WOW_FREE(mcnk->mcrf.doodads);
		WOW_FREE(mcnk->mcrf.wmos);
		WOW_FREE(mcnk->mcal.data);
		WOW_FREE(mcnk->mcse.data);
		WOW_FREE(mcnk->mclq.data);
	}
	WOW_FREE(file->mtex.data);
	WOW_FREE(file->textures);
	WOW_FREE(file);
}
