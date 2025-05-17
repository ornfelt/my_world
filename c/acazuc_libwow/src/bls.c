#include "bls.h"

#include "common.h"
#include "mpq.h"

#include <stdlib.h>
#include <string.h>

static bool
read_params(struct wow_bls_shader_param **params,
            uint32_t *nb,
            struct wow_mpq_file *mpq)
{
	if (wow_mpq_read(mpq, nb, sizeof(*nb)) != sizeof(*nb))
		return false;
	*params = WOW_MALLOC(sizeof(**params) * *nb);
	if (!*params)
		return false;
	if (wow_mpq_read(mpq, *params, sizeof(**params) * *nb) != sizeof(**params) * *nb)
	{
		WOW_FREE(*params);
		return false;
	}
	return true;
}

static bool
read_block(struct wow_bls_file *file,
           struct wow_bls_block *block,
           struct wow_mpq_file *mpq)
{
	block->shaders_nb = file->header.header.permutation_count;
	block->shaders = WOW_MALLOC(sizeof(*block->shaders) * block->shaders_nb);
	if (!block->shaders)
		return false;
	memset(block->shaders, 0, sizeof(*block->shaders) * block->shaders_nb);
	for (uint32_t i = 0; i < block->shaders_nb; ++i)
	{
		struct wow_bls_shader *shader = &block->shaders[i];
		if (!read_params(&shader->params, &shader->params_nb, mpq))
			return false;
		if (!read_params(&shader->consts, &shader->consts_nb, mpq))
			return false;
		uint32_t unk;
		if (wow_mpq_read(mpq, &unk, sizeof(unk)) != sizeof(unk)) /* 2 for abs, 0 for dx9 / combiners */
			return false;
		if (wow_mpq_read(mpq, &shader->code_len, sizeof(shader->code_len)) != sizeof(shader->code_len))
			return false;
		shader->code = WOW_MALLOC(shader->code_len);
		if (!shader->code)
			return false;
		if (wow_mpq_read(mpq, shader->code, shader->code_len) != shader->code_len)
			return false;
		if (shader->code_len % 4 != 0)
		{
			char pad[4];
			if (wow_mpq_read(mpq, pad, 4 - shader->code_len % 4) != 4 - shader->code_len % 4)
				return false;
		}
	}
	return true;
}

struct wow_bls_file *
wow_bls_file_new(struct wow_mpq_file *mpq)
{
	struct wow_bls_file *file;

	file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	file->blocks_nb = 0;
	file->blocks = NULL;
	if (wow_mpq_read(mpq, &file->header.header, sizeof(file->header.header)) != sizeof(file->header.header))
		goto err;
	if (file->header.header.magic == (('S' << 0) | ('V' << 8) | ('X' << 16) | ('G' << 24)))
	{
		if (wow_mpq_read(mpq, &file->header.vs, sizeof(file->header.vs)) != sizeof(file->header.vs))
			goto err;
		file->blocks_nb = 6;
		file->blocks = WOW_MALLOC(sizeof(*file->blocks) * file->blocks_nb);
		if (!file->blocks)
			goto err;
		memset(file->blocks, 0, sizeof(*file->blocks) * file->blocks_nb);
		for (uint32_t i = 0; i < file->blocks_nb; ++i)
		{
			if (!file->header.vs.offsets[i])
				continue;
			if (wow_mpq_seek(mpq, file->header.vs.offsets[i], SEEK_SET) == -1)
				goto err;
			if (!read_block(file, &file->blocks[i], mpq))
				goto err;
		}
	}
	else if (file->header.header.magic == (('S' << 0) | ('P' << 8) | ('X' << 16) | ('G' << 24)))
	{
		if (wow_mpq_read(mpq, &file->header.ps, sizeof(file->header.ps)) != sizeof(file->header.ps))
			goto err;
		file->blocks_nb = 12;
		file->blocks = WOW_MALLOC(sizeof(*file->blocks) * file->blocks_nb);
		if (!file->blocks)
			goto err;
		memset(file->blocks, 0, sizeof(*file->blocks) * file->blocks_nb);
		for (uint32_t i = 0; i < file->blocks_nb; ++i)
		{
			if (!file->header.ps.offsets[i])
				continue;
			if (wow_mpq_seek(mpq, file->header.ps.offsets[i], SEEK_SET) == -1)
				goto err;
			if (!read_block(file, &file->blocks[i], mpq))
				goto err;
		}
	}
	else
	{
		goto err;
	}
	return file;

err:
	wow_bls_file_delete(file);
	return NULL;
}

void
wow_bls_file_delete(struct wow_bls_file *file)
{
	if (!file)
		return;
	for (size_t i = 0; i < file->blocks_nb; ++i)
	{
		for (size_t j = 0; j < file->blocks[i].shaders_nb; ++j)
		{
			WOW_FREE(file->blocks[i].shaders[j].consts);
			WOW_FREE(file->blocks[i].shaders[j].params);
			WOW_FREE(file->blocks[i].shaders[j].code);
		}
		WOW_FREE(file->blocks[i].shaders);
	}
	WOW_FREE(file->blocks);
	WOW_FREE(file);
}
