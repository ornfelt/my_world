#include "mpq.h"

#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <zlib.h>

extern uint32_t wow_crypt_table[0x500];

static bool
file_seek(FILE *file, uint32_t off)
{
#ifdef _WIN32
	return _fseeki64(file, off, SEEK_SET);
#else
	return fseek(file, off, SEEK_SET);
#endif
}

static void
decrypt_table(void *data, size_t len, uint32_t key)
{
	uint32_t seed = 0xEEEEEEEE;
	uint32_t *wdata = (uint32_t*)data;
	len /= 4;
	for (size_t i = 0; i < len; ++i)
	{
		seed += wow_crypt_table[WOW_MPQ_CRYPT_OFFSET_DECRYPT_TABLE + (key & 0xFF)];
		uint32_t ch = wdata[i] ^ (key + seed);
		wdata[i] = ch;
		key = ((~key << 0x15) + 0x11111111) | (key >> 0x0B);
		seed = ch + seed + (seed << 5) + 3;
	}
}

uint32_t
wow_mpq_hash_string(const char *str, uint32_t hash_type)
{
	uint32_t seed1 = 0x7FED7FED;
	uint32_t seed2 = 0xEEEEEEEE;
	while (*str)
	{
		uint8_t ch = toupper(*str++);
		seed1 = wow_crypt_table[hash_type + ch] ^ (seed1 + seed2);
		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
	}
	return seed1;
}

struct wow_mpq_archive *
wow_mpq_archive_new(const char *filename)
{
	struct wow_mpq_archive *archive;
	FILE *file = NULL;

	archive = WOW_MALLOC(sizeof(*archive));
	if (!archive)
		return NULL;
	archive->block_table = NULL;
	archive->hash_table = NULL;
	archive->filename = WOW_MALLOC(strlen(filename) + 1);
	if (!archive->filename)
		goto err;
	strcpy(archive->filename, filename);
	file = fopen(filename, "rb");
	if (!file)
		goto err;
	if (!fread((char*)&archive->header, sizeof(archive->header), 1, file))
		goto err;
	if (archive->header.format_version >= 1)
	{
		if (!fread((char*)&archive->header2, sizeof(archive->header2), 1, file))
			goto err;
	}
	if (archive->header.block_table_size)
	{
		archive->block_table = WOW_MALLOC(sizeof(*archive->block_table) * archive->header.block_table_size);
		if (file_seek(file, archive->header.block_table_pos))
			goto err;
		if (!fread((char*)archive->block_table, sizeof(*archive->block_table) * archive->header.block_table_size, 1, file))
			goto err;
		decrypt_table(archive->block_table, sizeof(*archive->block_table) * archive->header.block_table_size, WOW_MPQ_KEY_BLOCK_TABLE);
	}
	if (archive->header.hash_table_size)
	{
		archive->hash_table = WOW_MALLOC(sizeof(*archive->hash_table) * archive->header.hash_table_size);
		if (file_seek(file, archive->header.hash_table_pos))
			goto err;
		if (!fread((char*)archive->hash_table, sizeof(*archive->hash_table) * archive->header.hash_table_size, 1, file))
			goto err;
		decrypt_table(archive->hash_table, sizeof(*archive->hash_table) * archive->header.hash_table_size, WOW_MPQ_KEY_HASH_TABLE);
	}
	fclose(file);
	return archive;

err:
	if (file)
		fclose(file);
	wow_mpq_archive_delete(archive);
	return NULL;
}

void
wow_mpq_archive_delete(struct wow_mpq_archive *archive)
{
	if (archive == NULL)
		return;
	WOW_FREE(archive->block_table);
	WOW_FREE(archive->hash_table);
	WOW_FREE(archive->filename);
	WOW_FREE(archive);
}

struct wow_mpq_compound *
wow_mpq_compound_new(void)
{
	struct wow_mpq_compound *compound;

	compound = WOW_MALLOC(sizeof(*compound));
	if (!compound)
		return NULL;
	compound->archives = NULL;
	compound->archives_nb = 0;
	return compound;
}

void
wow_mpq_compound_delete(struct wow_mpq_compound *compound)
{
	if (!compound)
		return;
	for (size_t i = 0; i < compound->archives_nb; ++i)
	{
		WOW_FREE(compound->archives[i].buffer);
		fclose(compound->archives[i].file);
	}
	WOW_FREE(compound->archives);
	WOW_FREE(compound);
}

bool
wow_mpq_compound_add_archive(struct wow_mpq_compound *compound,
                             const struct wow_mpq_archive *archive)
{
	struct wow_mpq_archive_view *archives;

	archives = WOW_REALLOC(compound->archives,
	                       sizeof(*compound->archives) * (compound->archives_nb + 1));
	if (!archives)
		return false;
	compound->archives = archives;
	compound->archives[compound->archives_nb].archive = archive;
	compound->archives[compound->archives_nb].buffer = WOW_MALLOC((size_t)512 << archive->header.block_size);
	if (!compound->archives[compound->archives_nb].buffer)
		return false;
	compound->archives[compound->archives_nb].file = fopen(archive->filename, "rb");
	if (!compound->archives[compound->archives_nb].file)
	{
		WOW_FREE(compound->archives[compound->archives_nb].buffer);
		return false; /* don't care of realloc since archives_nb isn't updated */
	}
	compound->archives_nb++;
	return true;
}

static bool
read_sector(const struct wow_mpq_block *block,
            uint32_t offset,
            uint32_t in_size,
            uint32_t out_size,
            FILE *file,
            uint8_t *data,
            size_t *data_size,
            uint8_t *buffer)
{
	/* XXX: check for CRC */
	if (file_seek(file, offset))
		return false;
	uint8_t compression = 0;
	if ((block->flags & WOW_MPQ_BLOCK_COMPRESS) && in_size < out_size)
	{
		in_size--;
		if (!fread((char*)&compression, 1, 1, file))
			return false;
	}
	switch (compression)
	{
		case WOW_MPQ_COMPRESSION_NONE:
		{
			size_t old_size = *data_size;
			*data_size = *data_size + out_size;
			if (!fread((char*)(data + old_size), out_size, 1, file))
				return false;
			break;
		}
		case WOW_MPQ_COMPRESSION_ZLIB:
		{
			if (!fread((char*)buffer, in_size, 1, file))
				return false;
			z_stream zstream;
			memset(&zstream, 0, sizeof(zstream));
			if (inflateInit(&zstream) != Z_OK)
				return false;
			zstream.avail_in = in_size;
			zstream.next_in = buffer;
			zstream.avail_out = out_size;
			zstream.next_out = data + *data_size;
			int ret = inflate(&zstream,  Z_FINISH);
			inflateEnd(&zstream);
			if (ret != Z_STREAM_END)
				return false;
			*data_size += out_size - zstream.avail_out;
			break;
		}
		default:
			return false;
	}
	return true;
}

static uint8_t *
read_block(const struct wow_mpq_archive_view *archive_view,
           const struct wow_mpq_block *block,
           size_t *data_size)
{
	uint8_t *data;

	data = WOW_MALLOC(block->file_size);
	if (!data)
		return NULL;
	uint32_t max_sector_size = 512 << archive_view->archive->header.block_size;
	uint32_t *sectors = NULL;
	uint32_t sectors_nb = 0;
	uint32_t sectors_capacity = 2 + (block->file_size + max_sector_size - 1) / max_sector_size;
	uint32_t tmp;
	*data_size = 0;
	if (!(block->flags & WOW_MPQ_BLOCK_COMPRESS) || (block->flags & WOW_MPQ_BLOCK_SINGLE_UNIT))
	{
		uint8_t *buffer;
		if (block->flags & WOW_MPQ_BLOCK_COMPRESS)
		{
			/* buffer is too short for full file */
			buffer = WOW_MALLOC(block->block_size);
			if (!buffer)
				goto err;
		}
		else
		{
			buffer = archive_view->buffer;
		}
		if (!read_sector(block, block->offset, block->block_size, block->file_size, archive_view->file, data, data_size, buffer))
		{
			if (buffer != archive_view->buffer)
				WOW_FREE(buffer);
			goto err;
		}
		if (buffer != archive_view->buffer)
			WOW_FREE(buffer);
		return data;
	}
	if (file_seek(archive_view->file, block->offset))
	{
		WOW_FREE(data);
		return NULL;
	}
	sectors = WOW_MALLOC(sizeof(*sectors) * sectors_capacity);
	if (!sectors)
		goto err;
	while (true)
	{
		if (!fread((char*)&tmp, sizeof(tmp), 1, archive_view->file))
			goto err;
		/*if (block.flags & WOW_MPQ_BLOCK_ENCRYPTED)
			decrypt_table(&val, sizeof(val), KEY_BLOCK_TABLE);*/
		sectors[sectors_nb++] = tmp;
		if (tmp > block->block_size)
			goto err;
		if (tmp == block->block_size)
			break;
	}
	if (block->flags & WOW_MPQ_BLOCK_SECTOR_CRC)
		sectors_nb--; /* XXX: don't bother with CRC */
	for (size_t i = 0; i < sectors_nb; ++i)
	{
		uint32_t offset = block->offset + sectors[i];
		uint32_t in_size = sectors[i + 1] - sectors[i];
		uint32_t out_size = block->file_size - *data_size;
		if (out_size > max_sector_size)
			out_size = max_sector_size;
		if (!read_sector(block, offset, in_size, out_size, archive_view->file, data, data_size, archive_view->buffer))
			goto err;
		if (*data_size >= block->file_size)
			break;
	}
	WOW_FREE(sectors);
	return data;

err:
	WOW_FREE(sectors);
	WOW_FREE(data);
	return NULL;
}

static const struct wow_mpq_block *
get_block(const struct wow_mpq_archive *archive,
          uint32_t bucket,
          uint32_t name_a,
          uint32_t name_b)
{
	if (!archive->hash_table || !archive->block_table)
		return NULL;
	uint32_t start_hash = bucket & (archive->header.hash_table_size - 1);
	for (uint32_t i = start_hash; i < archive->header.hash_table_size; ++i)
	{
		const struct wow_mpq_hash *hash = &archive->hash_table[i];
		if (hash->name_a == name_a && hash->name_b == name_b)
		{
			uint32_t block_index = hash->block_index;
			if (block_index >= archive->header.block_table_size)
				return NULL;
			const struct wow_mpq_block *block = &archive->block_table[block_index];
			if (block->flags & WOW_MPQ_BLOCK_DELETE_MARKER)
				return NULL;
			return block;
		}
		if (hash->block_index == 0xFFFFFFFF)
			return NULL;
	}
	return NULL;
}

struct wow_mpq_file *
get_file(const struct wow_mpq_archive_view *archive_view,
         uint32_t bucket,
         uint32_t name_a,
         uint32_t name_b)
{
	const struct wow_mpq_block *block = get_block(archive_view->archive, bucket, name_a, name_b);
	if (!block)
		return NULL;
	size_t data_size;
	uint8_t *data = read_block(archive_view, block, &data_size);
	if (!data)
		return NULL;
	struct wow_mpq_file *file = WOW_MALLOC(sizeof(*file));
	if (!file)
	{
		WOW_FREE(data);
		return NULL;
	}
	file->data = data;
	file->size = data_size;
	file->pos = 0;
	return file;
}

const struct wow_mpq_block *
wow_mpq_get_archive_block(const struct wow_mpq_archive_view *archive_view,
                          const char *filename)
{
	uint32_t bucket = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_BUCKET);
	uint32_t name_a = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_A);
	uint32_t name_b = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_B);
	return get_block(archive_view->archive, bucket, name_a, name_b);
}

struct wow_mpq_file *
wow_mpq_get_archive_file(const struct wow_mpq_archive_view *archive_view,
                         const char *filename)
{
	uint32_t bucket = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_BUCKET);
	uint32_t name_a = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_A);
	uint32_t name_b = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_B);
	return get_file(archive_view, bucket, name_a, name_b);
}

const struct wow_mpq_block *
wow_mpq_get_block(const struct wow_mpq_compound *compound,
                  const char *filename)
{
	uint32_t bucket = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_BUCKET);
	uint32_t name_a = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_A);
	uint32_t name_b = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_B);
	for (size_t i = 0; i < compound->archives_nb; ++i)
	{
		const struct wow_mpq_block *block = get_block(compound->archives[i].archive, bucket, name_a, name_b);
		if (block)
			return block;
	}
	return NULL;
}

struct wow_mpq_file *
wow_mpq_get_file(const struct wow_mpq_compound *compound,
                 const char *filename)
{
	uint32_t bucket = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_BUCKET);
	uint32_t name_a = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_A);
	uint32_t name_b = wow_mpq_hash_string(filename, WOW_MPQ_CRYPT_OFFSET_HASH_NAME_B);
	for (size_t i = 0; i < compound->archives_nb; ++i)
	{
		struct wow_mpq_file *file = get_file(&compound->archives[i], bucket, name_a, name_b);
		if (file)
			return file;
	}
	return NULL;
}

void
wow_mpq_file_delete(struct wow_mpq_file *file)
{
	if (!file)
		return;
	WOW_FREE(file->data);
	WOW_FREE(file);
}

uint32_t
wow_mpq_read(struct wow_mpq_file *file, void *data, uint32_t size)
{
	if (size > file->size - file->pos)
		size = file->size - file->pos;
	if (!size)
		return 0;
	memcpy(data, file->data + file->pos, size);
	file->pos += size;
	return size;
}

int32_t
wow_mpq_seek(struct wow_mpq_file *file, int32_t offset, int32_t whence)
{
	uint32_t base;
	switch (whence)
	{
		case SEEK_SET:
			base = 0;
			break;
		case SEEK_CUR:
			base = file->pos;
			break;
		case SEEK_END:
			base = file->size;
			break;
		default:
			return -1;
	}
	if (offset < 0 && (uint32_t)-offset > base)
		return -1;
	if (offset > 0 && base + offset > file->size)
		return -1;
	file->pos = base + offset;
	return file->pos;
}

void
wow_mpq_normalize_mpq_fn(char *fn, size_t size)
{
	(void)size;
	size_t j = 0;
	for (size_t i = 0; fn[i]; ++i, ++j)
	{
		if (fn[i] == '/')
			fn[j] = '\\';
		else
			fn[j] = toupper(fn[i]);
		if (fn[j] == '\\' && j > 0 && fn[j - 1] == '\\')
		{
			fn[j] = '\0';
			j--;
		}
	}
	fn[j] = '\0';
}

void
wow_mpq_normalize_blp_fn(char *fn, size_t size)
{
	if (!fn[0])
		return;
	wow_mpq_normalize_mpq_fn(fn, size);
	size_t len = strlen(fn);
	if (len < 4
	 || fn[len - 1] != 'P'
	 || fn[len - 2] != 'L'
	 || fn[len - 3] != 'B'
	 || fn[len - 4] != '.')
		snprintf(fn + len, size - len, ".BLP");
}

void
wow_mpq_normalize_m2_fn(char *fn, size_t size)
{
	wow_mpq_normalize_mpq_fn(fn, size);
	size_t len = strlen(fn);
	if (len >= 3
	 && (fn[len - 1] == 'X' || fn[len - 1] == 'L')
	 &&  fn[len - 2] == 'D' && fn[len - 3] == 'M')
	{
		fn[len - 2] = '2';
		fn[len - 1] = '\0';
	}
}

uint32_t
wow_crypt_table[0x500] =
{
	0x55C636E2, 0x02BE0170, 0x584B71D4, 0x2984F00E,
	0xB682C809, 0x91CF876B, 0x775A9C24, 0x597D5CA5,
	0x5A1AFEB2, 0xD3E9CE0D, 0x32CDCDF8, 0xB18201CD,
	0x3CCE05CE, 0xA55D13BE, 0xBB0AFE71, 0x9376AB33,
	0x848F645E, 0x87E45A45, 0x45B86017, 0x5E656CA8,
	0x1B851A95, 0x2542DBD7, 0xAB4DF9E4, 0x5976AE9B,
	0x6C317E7D, 0xCDDD2F94, 0x3C3C13E5, 0x335B1371,
	0x31A592CA, 0x51E4FC4C, 0xF7DB5B2F, 0x8ABDBE41,
	0x8BEAA674, 0x20D6B319, 0xDE6C9A9D, 0xC5AC84E5,
	0x445A5FEB, 0x94958CB0, 0x1E7D3847, 0xF35D29B0,
	0xCA5CCEDA, 0xB732C8B5, 0xFDCC41DD, 0x0EDCEC16,
	0x9D01FEAE, 0x1165D38E, 0x9EE193C8, 0xBF33B13C,
	0x61BC0DFC, 0xEF3E7BE9, 0xF8D4D4C5, 0xC79B7694,
	0x5A255943, 0x0B3DD20A, 0x9D1AB5A3, 0xCFA8BA57,
	0x5E6D7069, 0xCB89B731, 0x3DC0D15B, 0x0D4D7E7E,
	0x97E37F2B, 0xFEFC2BB1, 0xF95B16B5, 0x27A55B93,
	0x45F22729, 0x4C986630, 0x7C666862, 0x5FA40847,
	0xA3F16205, 0x791B7764, 0x386B36D6, 0x6E6C3FEF,
	0xC75855DB, 0x4ABC7DC7, 0x4A328F9B, 0xCEF20C0F,
	0x60B88F07, 0xF7BB4B8F, 0x830B5192, 0x94F711EC,
	0x20250752, 0x399D21A3, 0xE5C0840D, 0xE76CFFA5,
	0x624FAB29, 0x5DF133E6, 0x83E0B9B8, 0xC5796BFB,
	0x4A7AB2D0, 0xBA59A821, 0x03A81E4C, 0xCD3ADFDB,
	0x32B26B8C, 0x8E35C533, 0x9E6300E9, 0x8CF92AC5,
	0x880D18EB, 0x131A53B3, 0x2ED2DC64, 0xB23257C1,
	0xA06450C1, 0x1B92CB8E, 0x72ED730E, 0x19A685F0,
	0x82836483, 0x42D94E8A, 0xEE9BD6F6, 0x556D0B6A,
	0xBA65589A, 0xDE24CCE4, 0x53329F6C, 0xC754FE8B,
	0x503D2DC7, 0x10027BA4, 0xD3B60A8B, 0x68E68D83,
	0x0A9128A9, 0x595FA35F, 0x0B03B5BE, 0x150A45C4,
	0xB1629CCE, 0xE5F7497B, 0x8A7098A4, 0xB8233E69,
	0x8EA0F978, 0x5B579970, 0xEAB14318, 0x4B28B263,
	0xB6766CEF, 0x06782877, 0x155C6DD0, 0xC711333C,
	0xF819CEDF, 0x00EB1D68, 0xD6FFFA6E, 0x439E5962,
	0xD765D6DB, 0xCB0BCEE9, 0x6D3C5647, 0x965466F3,
	0x0CA983C9, 0x74ECC1CE, 0xFC0563B6, 0x42B08FEE,
	0xC5B38853, 0xFE502CEB, 0x7B432FAF, 0xC309E610,
	0x2C3997D8, 0x43774654, 0x15BD9D2C, 0xED6A420D,
	0xC7FF520C, 0xB8A97FD1, 0x5E4D60CC, 0xB9738D11,
	0xDA2181FF, 0x73AC2597, 0x3A8EEC8D, 0xAC85E779,
	0xF3F975D6, 0xB9FE7B91, 0x0F155D1E, 0x2860B6DD,
	0x835977CB, 0xB0607436, 0x9CAB7F6B, 0x8AB91186,
	0xC12B51E9, 0x20084E8B, 0x44BA8EAD, 0xA542B130,
	0x82BCD5C4, 0xCC747F4E, 0x0F1909D8, 0xDA242E1C,
	0x6F7D1AA0, 0xD2626486, 0x88D0781E, 0xAB695CCD,
	0xFA569145, 0xB4FEB55C, 0xBE47E896, 0xE70A7A88,
	0xD56185A2, 0xACF4C871, 0x09282332, 0x1DDEEAA8,
	0x590C7ADB, 0xF4A97667, 0xBFD85705, 0x0EA77CCC,
	0xA9F85364, 0x83195869, 0x8BFB041A, 0xDB842F5C,
	0xD6F0F315, 0xA7756EA7, 0x0A51B439, 0xA9EDF8A3,
	0xD9084E2F, 0x827407F8, 0xD4AC8284, 0x09739D0D,
	0xB3BB6CFC, 0xD539C77D, 0x6BBC9AC0, 0x35C641AA,
	0x934C96B0, 0xD17AF317, 0x29C6BAEF, 0xB275CDAC,
	0xD72662DE, 0x9F5C2544, 0xC1A98F75, 0xD98E8F9A,
	0x47BD5C86, 0x70C610A6, 0xB5482ED4, 0x23B9C68C,
	0x3C1BAE66, 0x69556E7F, 0xD902F5E0, 0x653D195B,
	0xDE6541FB, 0x07BCC6AC, 0xC6EE7788, 0x801534D4,
	0x2C1F35C0, 0xD9DE614D, 0xBDCCAC85, 0xB4D4A0DA,
	0x242D549B, 0x9D964796, 0xB9CEB982, 0x59FA99A9,
	0xD8986CC1, 0x9E90C1A1, 0x01BBD82F, 0xD7F1C5FD,
	0xDD847EBA, 0x883D305D, 0x25F13152, 0x4A92694D,
	0x77F1E601, 0x8024E6E7, 0x02A5F53D, 0x9C3EF4D9,
	0xAF403CCC, 0xE2AD03C0, 0x46EDF6EC, 0x6F9BD3E6,
	0xCC24AD7A, 0x47AFAB12, 0x82298DF7, 0x708C9EEC,
	0x76F8C1B1, 0xB39459D2, 0x3F1E26D9, 0xE1811BE7,
	0x56ED1C4D, 0xC9D18AF8, 0xE828060E, 0x91CADA2E,
	0x5CCBF9B7, 0xF1A552D4, 0x3C9D4343, 0xE1008785,
	0x2ADFEEBF, 0xF90240A0, 0x3D08CCE7, 0x426E6FB0,
	0x573C984F, 0x13A843AE, 0x406B7439, 0x636085D9,
	0x5000BA9A, 0xAD4A47AB, 0xAF001D8D, 0x419907AE,
	0x185C8F96, 0xE5E9ED4D, 0x61764133, 0xD3703D97,
	0xAC98F0C6, 0xDBC3A37C, 0x85F010C4, 0x90491E32,
	0xF12E18BF, 0xC88C96E1, 0xD3FBD6D9, 0xE3C28B08,
	0xD5BF08CC, 0xB1E78859, 0x2546DDCF, 0xB030B200,
	0xAAFD2811, 0x55B22D21, 0xD38BF567, 0x469C7A2B,
	0x5AD05792, 0xA1A5981E, 0x7DFB8384, 0x34D1CA0A,
	0x7EB0DBE0, 0xD61CE0F6, 0x398068B7, 0xE6406D1F,
	0x95AE6B47, 0xE4281230, 0xB0843061, 0xA70A3A68,
	0xE340F625, 0x72DCBFFD, 0x8EB8AFCD, 0x18B6661F,
	0x17EF5A5C, 0x000C5B22, 0x6BA13836, 0x6165E383,
	0x74481C5B, 0xE56F0711, 0xA26F5024, 0x5FF22E60,
	0x31A5E829, 0xA1094BF0, 0xC680EC6C, 0x8CF327D7,
	0xEBF1348A, 0x6A227D2F, 0x74065184, 0x8DF65112,
	0x2BBD05EE, 0xE4D00ED6, 0x2980EE1A, 0x6AE1DA73,
	0xE84614DA, 0x6C9906AB, 0xCF8E02DB, 0xD3723E97,
	0x92F66CAF, 0xAC8491C7, 0xAEC65696, 0xB98997CF,
	0xFA16C762, 0x6D73C65F, 0x205D22A6, 0x4DD3AAA5,
	0x2DEB6BC0, 0x9F37686C, 0x71A5282B, 0x376BB9E0,
	0x7FFF2A1B, 0xDE67982F, 0x9CBF33CE, 0x2E6DAB37,
	0x6E3424B9, 0x0EE143BC, 0x832A60D9, 0xBB6329E1,
	0x13F6BEFD, 0x5965FB84, 0xF60B233C, 0x3D695183,
	0x433224A1, 0xB5D9CAE5, 0x82459BAB, 0x9F21B311,
	0xAF6C5247, 0xB447B13A, 0x7B2676C3, 0xC38979CD,
	0x8526AE25, 0xC550AD5B, 0x685099A7, 0x65E9C2BD,
	0xE5C6DC36, 0xE10B37A9, 0x88016878, 0xCE81D4E4,
	0x24D6FC80, 0x4106152D, 0x6D4F5F90, 0xC4DC74BE,
	0xDB48676C, 0x6CB569B7, 0xF3BF598F, 0x042B08D9,
	0x02CCB2DE, 0xB1056F65, 0x47994AF4, 0xFA141BA4,
	0x9376AB2E, 0x07A76737, 0x75E7E6FC, 0x449D80A1,
	0x03B7259D, 0xF6DF358A, 0x5A75D5B9, 0x47286923,
	0x3B1A30EF, 0xEEBE3D6A, 0x9DB1AA00, 0x007A90D9,
	0x24667071, 0x019C73CF, 0x69039BCD, 0x95900744,
	0x6518B1EB, 0x6905F202, 0xEE3951B2, 0xE141FCA9,
	0x797FA832, 0x5A95E55B, 0xD6263B15, 0x5B61F394,
	0x897ACB1C, 0x005F83A9, 0x22420F71, 0xF495176E,
	0x7E138F3D, 0x1392E384, 0x373BF7AA, 0x8E512816,
	0xA960B3CA, 0x0474D74C, 0xFFACD6D7, 0x2EF5ED9E,
	0x60992AAA, 0x7E690E99, 0x23C0749D, 0xD8E29105,
	0x555D5909, 0x15631BFE, 0xA69C5A1C, 0x501017CA,
	0x99438048, 0x38733AC7, 0xE682E2C8, 0xD4655FD6,
	0x956E4C04, 0x347DF643, 0x2F4B177B, 0x93ED3AA4,
	0xA77E1DD5, 0x7AE55702, 0xD2A52FD9, 0xEF8BA18C,
	0xB7D3C1EE, 0x8078BA8D, 0xAB5AAADB, 0x752BE08F,
	0x068B31C1, 0x078AAE3C, 0xAA5A8343, 0x123D9268,
	0x2CEAEE43, 0x8EBDB239, 0x650251F3, 0x04883648,
	0x8C62E12E, 0x12B32167, 0xE5112E9A, 0x10002548,
	0x3E7A818D, 0x077E5327, 0xF140CC21, 0x6CE7D75D,
	0x9B99F9A5, 0x3215741C, 0xB6AADBAE, 0x738768DC,
	0x82A3742F, 0x76517020, 0xDD872AD8, 0x9D0902B2,
	0x7D1A6B04, 0x49381592, 0x63A652A5, 0x0C15E626,
	0xE22F70D6, 0x01E84385, 0xB29DE134, 0x20C5000E,
	0xE961F443, 0x2D31662E, 0x3CE6BC28, 0x34F9DD94,
	0xFA45DE53, 0x497588BD, 0x9468215B, 0x0777FA5C,
	0x6F7114C0, 0xE0E82694, 0xE4371986, 0x57112DE2,
	0xE0CAC289, 0xF2A3CEE0, 0x6A41E1B9, 0xBFCEA77D,
	0xF927FD52, 0x69747D98, 0xBEA76CDB, 0x8DD39557,
	0x04DB5ECE, 0x2A0885C8, 0x3BE4E8EE, 0x21D785DC,
	0x09DE7C0E, 0x3258EA33, 0x51922982, 0xEE8DD024,
	0x3DF6965D, 0x30C1237B, 0xF7F6686A, 0x9FACA186,
	0x7C400076, 0x85ACEF8A, 0xF4B6D220, 0xDDC3481C,
	0x439EAEC4, 0x717BBE63, 0x8259FAA7, 0xD682BD68,
	0x932A8610, 0x38BF0A7F, 0x6212E2C7, 0x88EE3168,
	0xB3C27047, 0x6133CB1E, 0x15295506, 0x5AE66246,
	0x1D208DDD, 0xA91D3DBA, 0xC315968D, 0x6AA2664B,
	0x716D0CCA, 0x891F4956, 0x80866BFF, 0xBD56C847,
	0x9093425A, 0x28DD9E87, 0x84EF3E08, 0x690A49D6,
	0x6A7EFF82, 0xABCFE400, 0x3D3BE5CA, 0x381B650C,
	0x4B7C8622, 0x3E0246F3, 0xA3561654, 0x9488865C,
	0x3AEF1BF2, 0x5E5D68A2, 0xD32F1DDC, 0x51972BF0,
	0x177A213B, 0x469375C2, 0x37640BD0, 0xFC3324C8,
	0x07091A09, 0x2D63D3FB, 0x2153F023, 0x48223875,
	0x61A55826, 0x8C136538, 0x49F71D98, 0x84C7D51E,
	0x85551A73, 0x13D604C5, 0xD701A626, 0x87B844CA,
	0x741EB29D, 0x2A2C977C, 0xC797CA03, 0x6C4085D7,
	0x2DACF79B, 0x734FA2EB, 0xCC290557, 0xFA1E75E4,
	0x06B29A27, 0xBECE2A7A, 0x70A4554B, 0xC935942E,
	0xA764BBC1, 0x1FE391D6, 0x7807F0C2, 0x40606ED9,
	0xE5153086, 0xE91D7DD2, 0xED5D3BA9, 0xAA14B64A,
	0x83B24DD9, 0xEC1FF5CD, 0xBA33EAD3, 0xE4EF735C,
	0xBC062438, 0xD8BFD523, 0x473D1E04, 0x2007F8A7,
	0xB02903ED, 0x86EA8ADA, 0x95AB69CF, 0xFD1F9809,
	0x9CB3D8BB, 0x51F45958, 0x9CDD4276, 0xC245865E,
	0x8F0C836B, 0x4EE7DC07, 0xF6368D9D, 0xEF2C1DC1,
	0xEE56B54B, 0xBD62CE2F, 0xF4916AAD, 0xC81CB594,
	0x41729F49, 0x24BEF0A4, 0xDEF487A9, 0x222E05B8,
	0x8D3BF5C6, 0x11B55009, 0xAD09D2B3, 0x19DB9FD1,
	0xD7427085, 0x33DBFC8B, 0x526B9378, 0x790E1BC8,
	0xB2998A00, 0xA5641703, 0x0676D249, 0x6B9185CC,
	0x30E4348F, 0x82C52F65, 0x57C7DC24, 0x489C1ECD,
	0x9FCAB02A, 0x56D61117, 0xFE869CAC, 0x55FC5140,
	0x7FBBB382, 0x9E5AFC79, 0x10047C99, 0xFC9F5984,
	0x56587E2D, 0xB98193F0, 0x98FE5E8E, 0x29B15B6B,
	0x9561F055, 0xBB0CAA25, 0x1E4ECC15, 0x23F5393B,
	0x0845B458, 0xCEFF67CA, 0xB099900C, 0x00B1564F,
	0x39EEF3D1, 0xFCC1BF84, 0xAC8893B5, 0x6484BF0E,
	0x91C02AB3, 0x8C0C0C70, 0x686FA8C6, 0xE171BED6,
	0xDFAE37DF, 0xD5A1A4E7, 0xE3EB49A1, 0x5E6014E0,
	0x205B21AC, 0xFD58B3DA, 0x2E7C07CD, 0xEF2CC85A,
	0xD7587B46, 0xF417847D, 0x8A30CEC1, 0x70984F6C,
	0xF0B63388, 0xC220C98D, 0xEDE62936, 0x92C0A7B3,
	0x1EF371E8, 0x2005F7AF, 0x91A47265, 0xB0CF5504,
	0xD500ABA8, 0xCB5C4BD3, 0x9B3BCBC3, 0xCF6644B5,
	0xCE9488EF, 0x003FC96E, 0xAA42222F, 0x4844F3D0,
	0x4DB89D77, 0x08681AAE, 0x662F3A28, 0x761552DB,
	0x1DF7A17A, 0x93FEED9A, 0xCC496A4F, 0xA217CFCD,
	0x3BA3C930, 0x268F7E77, 0x0797B4A1, 0x8BEBFC51,
	0x068930C4, 0x16C874E2, 0xC242DA24, 0xFB229F76,
	0xA0795B02, 0x689FC036, 0x17A73732, 0xD21AEC00,
	0xAC00A692, 0x5B217F18, 0xAE421624, 0x2BC05CC0,
	0x48C1DB7A, 0x4F4E63B4, 0x1667F04E, 0x34020F94,
	0x972B2555, 0x9A07355B, 0x01665970, 0x7DB60C6F,
	0x3AD7103B, 0x5C3D09C0, 0xEEA3DADA, 0x88C21C10,
	0x102436D7, 0x6A3B3400, 0xEB523C4C, 0xFB97D896,
	0x964CB86B, 0xDD878038, 0x0529DA4D, 0x0B1468A5,
	0x18739AC8, 0xF7F26668, 0xF64F4471, 0x5C14F5C3,
	0x44A081FB, 0x39AC7E37, 0x8A17C26B, 0x868F5E67,
	0x3931978D, 0x6EDF7817, 0x4951CC67, 0x943407F3,
	0xCC5E748F, 0x2B7EE729, 0xCBB320F0, 0x11FEC8E7,
	0xFCCFC658, 0x03454354, 0x373AA1EC, 0x1D58FE9A,
	0x064710AE, 0xA88AA0BA, 0xD183A23E, 0x40D150A3,
	0xF531B8D1, 0xA7D99F85, 0x11838CD5, 0xB19E64B3,
	0x3D67A5E9, 0xB02C5AC6, 0x99B9B9E8, 0x4C202B7A,
	0x15F261D3, 0xA84C2D0D, 0x50F185A6, 0x33BA41D5,
	0x39791013, 0x4BAFF44E, 0xEEEEAA1C, 0xE0488314,
	0x559CCD2B, 0xA104F445, 0x636F37C4, 0x264D5E3B,
	0x75C17F35, 0x75424131, 0xBB115739, 0x74FE755A,
	0x7D3A7AA6, 0x2D8BE784, 0x83ED154A, 0xFC2673D8,
	0x44DD4A7F, 0x79056CC8, 0x82CC8831, 0x9D3C1B7C,
	0xE9453BFA, 0x24315694, 0x661F3253, 0x75549F5C,
	0xBB2B63ED, 0x67E00D96, 0xF48966C7, 0x0D7BEA56,
	0xC25F92EF, 0xA947A79D, 0xDE4ADF6F, 0xAC0F0342,
	0xD3EB246B, 0xA4AA118E, 0x3C3E6A46, 0x457F4441,
	0xA50A406F, 0x6C508D9F, 0xE9AC18E7, 0x1ECDB4BA,
	0x39AC7E3A, 0x7FB304FA, 0x6F38F8E8, 0x4AECEA6D,
	0x61035E73, 0x81708907, 0xEBC07205, 0x90FD7614,
	0xB52D217F, 0x6C4DE195, 0x1DD49084, 0x64EE482C,
	0x94C7A521, 0x540C09D8, 0x75DF8DD5, 0x414131F7,
	0x3698FD76, 0xF784DB4F, 0xF8C97A03, 0x048F39B9,
	0x3BF4F0BD, 0x8CB50992, 0x9B58D9EE, 0xE5AB79CC,
	0x9A5F6052, 0xBD9591B0, 0xFAD2232B, 0x5A632254,
	0x0286E618, 0x8AD3C8F7, 0xE4060176, 0x754C4617,
	0x5C10490B, 0x6F7D6FFF, 0x2187B42A, 0x5775095B,
	0x02F4C663, 0x5A5DCA06, 0xFE4AD4C7, 0x53E19F7D,
	0x59FF46B5, 0xBCC42BA5, 0xFD2F4A97, 0xBED6D905,
	0x95629B6B, 0x21A1C0DB, 0xAA10B45D, 0xE6EF6D58,
	0x2892CF4D, 0x9FED6C10, 0x1E386BF7, 0x9BE0C6E8,
	0x2B2F15EF, 0x19F5AC7B, 0x7AFF0E72, 0x31DA576F,
	0x30252CB4, 0x577960AC, 0x166E9E5A, 0xA9374A61,
	0x71369C96, 0x7FF826AE, 0xE8175326, 0xCABBFD33,
	0x0191190E, 0x699D3C3E, 0x36B40B22, 0xB3950513,
	0x9B889BFA, 0xA52A5007, 0xAC290FED, 0x3B4E4A4F,
	0xB753D8D6, 0x3C531F22, 0x582F6427, 0xA9CD93A9,
	0x546E39AE, 0x242FAAD2, 0xD2E0F747, 0x09F6325D,
	0x59D48719, 0xAD7EB66E, 0xD5512878, 0x56DEBF9D,
	0x5107E5A5, 0xF1C00AA4, 0x814CCCA8, 0x600D90F0,
	0x9BE97619, 0x915FA5F2, 0x2B5628DD, 0xA33D5F5A,
	0x595DF7C1, 0x6966215D, 0x50EC8337, 0xF1D21372,
	0x0EE2EEFB, 0xAD9E70B7, 0xAB0D2FE4, 0xCF277B5D,
	0x62585A2C, 0x835A7844, 0x74B1FA6B, 0x49BAFFD5,
	0x2EA9C864, 0x129311A8, 0xBDFA1867, 0x83CA5997,
	0x9D1DB719, 0x84BB79E6, 0x9E3F99F2, 0x313F6101,
	0x1B99245B, 0xD15D8FB2, 0xCEF90F81, 0x2945268D,
	0xDBBCF573, 0xB1021886, 0x9EE7EC1D, 0x1CF824F7,
	0x7EAA2E32, 0x69C0A2B5, 0x7494419C, 0xE253D7D3,
	0x48DA3D12, 0x45B8B571, 0xDB4D147A, 0xD82D8DDE,
	0x265D10A2, 0xB0A6EB9A, 0x7E1C93A6, 0x36FE2F46,
	0xDCAD6B00, 0x05439191, 0xB0CE5484, 0x61D1C309,
	0x8DA62A03, 0x06D0FE2F, 0xBAC6DD3C, 0xCA2006F3,
	0x8321B1AF, 0x0411A6F3, 0xE8918EAC, 0x21A2C152,
	0x91C0D54F, 0x6AAA14FA, 0xDD22A440, 0x88CB2075,
	0x7A4EB813, 0x67AFA071, 0xD8D98C9C, 0x31F10D47,
	0x6FF1A8A8, 0x2FAAF0A1, 0x48A221BB, 0x3BE6948B,
	0xAA79E79B, 0x0EA7278C, 0x7A3857EF, 0x49B7FE55,
	0xD51CB931, 0x041C018D, 0x00B90501, 0x45EA7881,
	0x8FC1DBCF, 0xB80B32A9, 0xABACD2E9, 0x677BDC40,
	0xECACE542, 0x6D6514EB, 0x31C09FF7, 0x5E6C1ABD,
	0x1C391D0F, 0x0E9D77F1, 0x7119392D, 0x6BE9B0BA,
	0x6194FA77, 0x45E62148, 0x42234AF2, 0xC3239D66,
	0x939CBDBC, 0x56200D9C, 0x6B275208, 0x001A61F3,
	0xCCC2A546, 0x4B722BE0, 0xEE25F2B7, 0x6D86CF9E,
	0xAA6BE0CD, 0x4DCDA7B6, 0x78D4AA13, 0x36EA7AD9,
	0x3F29D700, 0xDEEA2D84, 0x6A6AF5BD, 0x18AFB81C,
	0xD8E4E73C, 0x8AA708BA, 0x658B94D9, 0xA676478C,
	0xCFA10C22, 0x25593C74, 0x8D962235, 0x5F980270,
	0x3DF6EBC0, 0x8E7D92FA, 0xC3EE55E1, 0xD5F72447,
	0x02B0FA95, 0x52B0B520, 0x70D2C11F, 0x3A6FDD6C,
	0x193AA698, 0x5496F7D5, 0x4208931B, 0x7A4106EC,
	0x83E86840, 0xF49B6F8C, 0xBA3D9A51, 0x55F54DDD,
	0x2DE51372, 0x9AFB571B, 0x3AB35406, 0xAD64FF1F,
	0xC77764FE, 0x7F864466, 0x416D9CD4, 0xA2489278,
	0xE30B86E4, 0x0B5231B6, 0xBA67AED6, 0xE5AB2467,
	0x60028B90, 0x1D9E20C6, 0x2A7C692A, 0x6B691CDB,
	0x9E51F817, 0x9B763DEC, 0x3D29323F, 0xCFE12B68,
	0x754B459B, 0xA2238047, 0xD9C55514, 0x6BDCFFC1,
	0x693E6340, 0x82383FE7, 0x1916EA5F, 0xEC7BCD59,
	0x72DE165A, 0xE79A1617, 0x8EC86234, 0xA8F0D284,
	0x20C90226, 0x7BF98884, 0x28A58331, 0x3EC3FA6E,
	0x4CE0895B, 0xC353B4D0, 0x33EF064F, 0x21E5E210,
	0xC8BB589D, 0xE85DCAB2, 0xAC65829F, 0xA7BF92D0,
	0x05A6174D, 0x25A50C2E, 0xE5C78777, 0x3D75021F,
	0x4BAA9C98, 0x23BDC884, 0x9653BBD7, 0xBADCE7F5,
	0xC283A484, 0xC040DF2E, 0x9370A841, 0x2F316022,
	0x36EED231, 0xAC2CBC0C, 0x13C0A49B, 0xCDD12997,
	0x07FE91B2, 0xCD7EABCD, 0x2C01271D, 0x18432DF8,
	0x599C6BC7, 0x75E93D5A, 0xB67A6EE2, 0x8E738E16,
	0xFF9073FD, 0xAF77026A, 0xF86EA2FC, 0x91509EA3,
	0x33A78DC6, 0x4F79234A, 0x3A7535BC, 0x3539FCB1,
	0x3103EE52, 0x4F6F1E69, 0x6BB3EBBC, 0x4CB77555,
	0x8DD1E999, 0x2ADE439D, 0x11521FAE, 0xB94D2545,
	0x8DDE9ABD, 0x1909393F, 0xB792A23D, 0x749C455B,
	0xB5B60F2C, 0x380459CE, 0x0DAD5820, 0xB130845B,
	0x291CBD52, 0xDE9A5BB7, 0x51DEF961, 0x515B6408,
	0xCA6E823E, 0x382E6E74, 0xEEBE3D71, 0x4C8F0C6A,
	0xE676DCEA, 0x14E1DC7C, 0x6F7FC634, 0xCF85A943,
	0xD39EA96E, 0x136E7C93, 0x7164B304, 0xF32F1333,
	0x35C34034, 0xDE39D721, 0x91A87439, 0xC410111F,
	0x29F17AAC, 0x1316A6FF, 0x12F194EE, 0x420B9499,
	0xF72DB0DC, 0x690B9F93, 0x17D14BB2, 0x8F931AB8,
	0x217500BC, 0x875413F8, 0x98B2E43D, 0xC51F9571,
	0x54CEBDCA, 0x0719CC79, 0xF3C7080D, 0xE4286771,
	0xA3EAB3CD, 0x4A6B00E0, 0x11CF0759, 0x7E897379,
	0x5B32876C, 0x5E8CD4F6, 0x0CEDFA64, 0x919AC2C7,
	0xB214F3B3, 0x0E89C38C, 0xF0C43A39, 0xEAE10522,
	0x835BCE06, 0x9EEC43C2, 0xEA26A9D6, 0x69531821,
	0x6725B24A, 0xDA81B0E2, 0xD5B4AE33, 0x080F99FB,
	0x15A83DAF, 0x29DFC720, 0x91E1900F, 0x28163D58,
	0x83D107A2, 0x4EAC149A, 0x9F71DA18, 0x61D5C4FA,
	0xE3AB2A5F, 0xC7B0D63F, 0xB3CC752A, 0x61EBCFB6,
	0x26FFB52A, 0xED789E3F, 0xAA3BC958, 0x455A8788,
	0xC9C082A9, 0x0A1BEF0E, 0xC29A5A7E, 0x150D4735,
	0x943809E0, 0x69215510, 0xEF0B0DA9, 0x3B4E9FB3,
	0xD8B5D04C, 0xC7A023A8, 0xB0D50288, 0x64821375,
	0xC260E8CF, 0x8496BD2C, 0xFF4F5435, 0x0FB5560C,
	0x7CD74A52, 0x93589C80, 0x88975C47, 0x83BDA89D,
	0x8BCC4296, 0x01B82C21, 0xFD821DBF, 0x26520B47,
	0x04983E19, 0xD3E1CA27, 0x782C580F, 0x326FF573,
	0xC157BCC7, 0x4F5E6B84, 0x44EBFBFB, 0xDA26D9D8,
	0x6CD9D08E, 0x1719F1D8, 0x715C0487, 0x2C2D3C92,
	0x53FAABA9, 0xBC836146, 0x510C92D6, 0xE089F82A,
	0x4680171F, 0x369F00DE, 0x70EC2331, 0x0E253D55,
	0xDAFB9717, 0xE5DD922D, 0x95915D21, 0xA0202F96,
	0xA161CC47, 0xEACFA6F1, 0xED5E9189, 0xDAB87684,
	0xA4B76D4A, 0xFA704897, 0x631F10BA, 0xD39DA8F9,
	0x5DB4C0E4, 0x16FDE42A, 0x2DFF7580, 0xB56FEC7E,
	0xC3FFB370, 0x8E6F36BC, 0x6097D459, 0x514D5D36,
	0xA5A737E2, 0x3977B9B3, 0xFD31A0CA, 0x903368DB,
	0xE8370D61, 0x98109520, 0xADE23CAC, 0x99F82E04,
	0x41DE7EA3, 0x84A1C295, 0x09191BE0, 0x30930D02,
	0x1C9FA44A, 0xC406B6D7, 0xEEDCA152, 0x6149809C,
	0xB0099EF4, 0xC5F653A5, 0x4C10790D, 0x7303286C,
};
