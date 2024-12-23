#include "world/region.h"
#include "world/world.h"
#include "world/chunk.h"

#include "voxel.h"
#include "log.h"

#include <arpa/inet.h>

#include <nbt/nbt.h>

#include <stdlib.h>
#include <string.h>
#include <zlib.h>

struct region *region_new(struct world *world, int32_t x, int32_t z)
{
	struct region *region = calloc(sizeof(*region), 1);
	if (!region)
	{
		LOG_ERROR("region allocation failed");
		return NULL;
	}
	region->world = world;
	region->x = x;
	region->z = z;
	region->filename = NULL;
	region->file = NULL;
	region->sectors = NULL;
	region->sectors_count = 0;
	if (voxel_asprintf(&region->filename, "saves/world/region/r.%d.%d.mca", region->x / (REGION_WIDTH * CHUNK_WIDTH), region->z / (REGION_WIDTH * CHUNK_WIDTH)) == -1)
	{
		LOG_ERROR("failed to allocate filename");
		free(region);
		return NULL;
	}
	memset(region->chunks, 0, sizeof(region->chunks));
	memset(region->storage_timestamp, 0, sizeof(region->storage_timestamp));
	memset(region->storage_header, 0, sizeof(region->storage_header));
	region_load(region);
	return region;
}

void region_delete(struct region *region)
{
	if (!region)
		return;
	region_save(region);
	if (region->file)
		fclose(region->file);
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
		chunk_delete(region->chunks[i]);
	free(region->sectors);
	free(region->filename);
}

void region_check_clear(struct region *region)
{
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
	{
		if (region->chunks[i])
			return;
	}
	TAILQ_REMOVE(&region->world->regions, region, chain);
	region_delete(region);
}

void region_load(struct region *region)
{
	region->file = fopen(region->filename, "ab+");
	if (!region->file)
	{
		LOG_ERROR("failed to open region file \"%s\"", region->filename);
		abort();
	}
	if (fseek(region->file, 0, SEEK_SET))
		LOG_ERROR("failed to seek 0 on file \"%s\"", region->filename);
	if (fread(region->storage_header, sizeof(region->storage_header), 1, region->file) != 1)
	{
		LOG_WARN("failed to read header of \"%s\", writing empty one", region->filename);
		if (fwrite(region->storage_header, sizeof(region->storage_header), 1, region->file) != 1)
			LOG_ERROR("failed to write empty storage header of \"%s\"", region->filename);
	}
	if (fread(region->storage_timestamp, sizeof(region->storage_timestamp), 1, region->file) != 1)
	{
		LOG_WARN("failed to read timestamp of \"%s\", writing empty one", region->filename);
		if (fwrite(region->storage_timestamp, sizeof(region->storage_timestamp), 1, region->file) != 1)
			LOG_ERROR("failed to write empty storage timestamp of \"%s\"", region->filename);
	}
	if (fseek(region->file, 0, SEEK_END))
		LOG_ERROR("failed to get size of file \"%s\"", region->filename);
	int64_t filesize = ftell(region->file);
	if (filesize == -1)
		LOG_ERROR("failed to ftell \"%s\"", region->filename);
	if (filesize & 0xFFF)
	{
		static const uint8_t zeros[0x1000] = {0};
		if (fwrite(zeros, 1, filesize & 0xFFF, region->file) != 1)
			LOG_ERROR("failed to grow region file");
		filesize += filesize & 0xFFF;
	}
	size_t sectors_count = filesize / REGION_SECTOR_SIZE;
	size_t sectors_size = (sectors_count + 7) / 8;
	size_t old_sectors_size = (region->sectors_count + 7) / 8;
	uint8_t *new_sectors = realloc(region->sectors, sectors_size);
	if (!new_sectors)
	{
		LOG_ERROR("sectors allocation failed");
		abort();
	}
	memset(&new_sectors[old_sectors_size], 0, sectors_size - old_sectors_size);
	region->sectors = new_sectors;
	region->sectors_count = sectors_count;
	region->sectors[0] |= 3;
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
	{
		uint32_t offset_alloc = region->storage_header[i];
		offset_alloc = ntohl(offset_alloc);
		if (!offset_alloc)
			continue;
		uint32_t offset = (offset_alloc >> 8) & 0xFFFFFF;
		uint32_t allocated = offset_alloc & 0xFF;
		if (offset + allocated > region->sectors_count)
		{
			LOG_WARN("invalid (offset + allocated): %u, sectors count: %u", (unsigned)(offset + allocated), (unsigned)region->sectors_count);
			region->storage_header[i] = 0;
			continue;
		}
		for (size_t j = 0; j < allocated; ++j)
		{
			size_t idx = offset + j;
			region->sectors[idx / 8] |= (1 << (idx & 8));
		}
	}
}

void region_save(struct region *region)
{
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
	{
		if (!region->chunks[i])
			continue;
		region_save_chunk(region, region->chunks[i]);
	}
}

static char *zip_memory(const void *data, size_t size, size_t *compressed_size)
{
	return NULL;
}

static uint8_t *unzip_memory(const uint8_t *data, size_t size, size_t *uncompressed_size)
{
	size_t used = 0;
	uint8_t *ret = NULL;
	*uncompressed_size = 0;
	z_stream zstream;
	memset(&zstream, 0, sizeof(zstream));
	if (inflateInit(&zstream) != Z_OK)
		return NULL;
	do
	{
		*uncompressed_size += 4096;
		uint8_t *new_ret = realloc(ret, *uncompressed_size);
		if (!new_ret)
		{
			LOG_ERROR("allocation failed");
			inflateEnd(&zstream);
			free(ret);
			return NULL;
		}
		ret = new_ret;
		zstream.avail_in = size - used;
		zstream.next_in = (uint8_t*)data + used;
		zstream.avail_out = 4096;
		zstream.next_out = &ret[*uncompressed_size - 4096];
		int res = inflate(&zstream,  Z_NO_FLUSH);
		*uncompressed_size -= zstream.avail_out;
		used += (size - used) - zstream.avail_in;
		if (res == Z_OK)
			continue;
		if (res == Z_STREAM_END)
			break;
		LOG_ERROR("zlib decompression failed: %d", res);
		inflateEnd(&zstream);
		free(ret);
		return NULL;
	} while (*uncompressed_size < 1024 * 1024);
	inflateEnd(&zstream);
	return ret;
}

void region_save_chunk(struct region *region, struct chunk *chunk)
{
#if 0
	if (!chunk->changed)
		return;
	chunk->changed = true;
	uint32_t sectors_len = 0;
	char *data;
	{
		uint32_t data_len = chunk->nbt.nbt->getHeaderSize() + chunk->nbt.nbt->getDataSize() + 5;
		data = (char*)malloc(data_len);
		if (!data)
		{
			LOG_ERROR("allocation failed");
			abort();
		}
		memset(data, 0, data_len);
		NBTBuffer buffer;
		buffer.data = data;
		buffer.pos = 0;
		buffer.len = data_len;
		chunk->nbt.nbt->writeHeader(&buffer);
		chunk->nbt.nbt->writeData(&buffer);
		size_t compressed_size;
		char *compressed = zip_memory(data, data_len, &compressed_size);
		if (!compressed)
		{
			LOG_ERROR("failed to compress");
			abort();
		}
		sectors_len = ceil((compressed_size + 5) / (float)REGION_SECTOR_SIZE);
		free(data);
		data = (char*)malloc(sectors_len * REGION_SECTOR_SIZE);
		if (!data)
		{
			LOG_ERROR("allocation failed");
			abort();
		}
		int32_t len = compressed_size;
		len = ntohl(len);
		memmove(data, &len, 4);
		int8_t compression = 2;
		memmove(data + 4, &compression, 1);
		memmove(data + 5, compressed, compressed_size);
		memset(data + 5 + compressed_size, 0, sectors_len * REGION_SECTOR_SIZE - compressed_size - 5);
		free(compressed);
	}
	uint32_t header_pos = region_xz_id((chunk->x - region->x) / CHUNK_WIDTH, (chunk->z - region->z) / CHUNK_WIDTH);
	int32_t timestamp = g_voxel->frametime / 1000;
	region->storage_timestamp[header_pos] = ntohl(timestamp);
	uint32_t offset_alloc = region->storage_header[header_pos];
	offset_alloc = ntohl(offset_alloc);
	uint32_t offset = (offset_alloc >> 8) & 0xFFFFFF;
	uint8_t allocated = offset_alloc & 0xFF;
	if (!offset)
		goto alloc;
	if (sectors_len < allocated)
		goto write;
	if (sectors_len < allocated)
	{
		for (size_t i = sectors_len; i < allocated; ++i)
			region->sectors[i] = false;
		goto write;
	}
	for (size_t i = 0; i < allocated; ++i)
		region->sectors[offset + i] = false;
alloc:
	if (sectors_len > region->sectors.size())
		goto new_sector;
	for (size_t i = 0; i < region->sectors.size() - sectors_len; ++i)
	{
		for (uint32_t j = 0; j < sectors_len; ++j)
		{
			if (region->sectors[i + j])
				goto next_test;
		}
		offset = i;
		allocated = sectors_len;
		offset_alloc = ((offset & 0xFFFFFF) << 8) | allocated;
		region->storage_header[header_pos] = ntohl(offset_alloc);
		for (size_t i = 0; i < allocated; ++i)
			region->sectors[offset + i] = true;
		goto write;
next_test:
		continue;
	}
new_sector:
	offset = region->sectors.size();
	allocated = sectors_len;
	offset_alloc = ((offset & 0xFFFFFF) << 8) | allocated;
	region->storage_header[header_pos] = ntohl(offset_alloc);
	region->sectors.resize(region->sectors.size() + allocated, true);
write:
	if (fseek(region->file, offset * REGION_SECTOR_SIZE, SEEK_SET))
		LOG_ERROR("failed to seek region \"%s\" chunk storage", region->filename);
	if (fwrite(data, sectors_len * REGION_SECTOR_SIZE, 1, region->file) != 1)
		LOG_ERROR("failed to write region \"%s\" chunk storage", region->filename);
	if (fseek(region->file, header_pos * 4, SEEK_SET))
		LOG_ERROR("failed to seek region \"%s\" offset", region->filename);
	if (fwrite(&region->storage_header[header_pos], 4, 1, region->file) != 1)
		LOG_ERROR("failed to write region \"%s\" offset", region->filename);
	if (fseek(region->file, 4096 + header_pos * 4, SEEK_SET))
		LOG_ERROR("failed to seek region \"%s\" timestamp", region->filename);
	if (fwrite(&region->storage_timestamp[header_pos], 4, 1, region->file) != 1)
		LOG_ERROR("failed to write region \"%s\" timestamp", region->filename);
	free(data);
#endif
}

void region_tick(struct region *region)
{
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
	{
		if (!region->chunks[i])
			continue;
		chunk_tick(region->chunks[i]);
		region_save_chunk(region, region->chunks[i]);
	}
}

void region_draw_entities(struct region *region)
{
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
	{
		if (!region->chunks[i])
			continue;
		chunk_draw_entities(region->chunks[i]);
	}
}

void region_draw(struct region *region, uint8_t layer)
{
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
	{
		if (!region->chunks[i])
			continue;
		chunk_draw(region->chunks[i], layer);
	}
}

void region_generate_chunk(struct region *region, int32_t x, int32_t z)
{
	struct chunk *chunk = region_get_chunk(region, x, z);
	if (!chunk)
		chunk = region_create_chunk(region, x, z);
	chunk_generate(chunk);
}

static bool read_chunk_storage(struct region *region, uint32_t storage,
                               struct nbt_tag_compound **tagp)
{
	if (fseek(region->file, (storage >> 8) * REGION_SECTOR_SIZE, SEEK_SET))
	{
		LOG_ERROR("failed to seek section");
		return false;
	}
	int32_t clen = 0;
	if (fread(&clen, 4, 1, region->file) != 1)
	{
		LOG_ERROR("failed to read section length");
		return false;
	}
	clen = ntohl(clen);
	if (clen < 0)
	{
		LOG_ERROR("negative section length");
		return false;
	}
	clen--;
	int8_t compression;
	if (fread(&compression, 1, 1, region->file) != 1)
	{
		LOG_ERROR("failed to read section compression");
		return false;
	}
	if (compression != 2)
	{
		LOG_ERROR("section compression not supported (%d) in %s at %u",
		          (int)compression, region->filename,
		          (unsigned)((storage >> 8) * REGION_SECTOR_SIZE));
		return false;
	}
	uint8_t *cdata = (uint8_t*)malloc(clen);
	if (fread(cdata, clen, 1, region->file) != 1)
	{
		LOG_ERROR("invalid region sectors read");
		return false;
	}
	size_t uncompressed_size;
	uint8_t *uncompressed = unzip_memory(cdata, clen, &uncompressed_size);
	free(cdata);
	if (!uncompressed)
	{
		LOG_ERROR("failed to uncompress file");
		return false;
	}
	struct nbt_stream *stream = nbt_stream_memory_new(uncompressed, uncompressed_size);
	if (!stream)
	{
		LOG_ERROR("failed to create NBT stream");
		return false;
	}
	struct nbt_tag *tag = nbt_tag_read(stream);
	free(uncompressed);
	if (!tag)
	{
		LOG_WARN("no NBT found");
		*tagp = NULL;
		return true;
	}
	if (tag->type != NBT_TAG_COMPOUND)
	{
		LOG_WARN("invalid chunk NBT tag: %d", (int)tag->type);
		nbt_tag_delete(tag);
		*tagp = NULL;
		return true;
	}
	*tagp = (struct nbt_tag_compound*)tag;
	return true;
}

struct chunk *region_create_chunk(struct region *region, int32_t x, int32_t z)
{
	struct nbt_tag_compound *tag = NULL;
	uint32_t storage = region->storage_header[region_xz_id(x, z)];
	storage = ntohl(storage);
	if (storage)
	{
		if (!read_chunk_storage(region, storage, &tag))
			abort();
	}
	struct chunk *chunk = chunk_new(region->world, region->x + x * CHUNK_WIDTH, region->z + z * CHUNK_WIDTH, tag);
	region_set_chunk(region, x, z, chunk);
	return chunk;
}

void region_set_chunk(struct region *region, int32_t x, int32_t z, struct chunk *chunk)
{
	uint32_t id = region_xz_id(x, z);
	struct chunk *old_chunk = region->chunks[id];
	region->chunks[id] = chunk;
	if (!chunk && old_chunk)
		region_save_chunk(region, old_chunk);
}

struct chunk *region_get_chunk(struct region *region, int32_t x, int32_t z)
{
	return region->chunks[region_xz_id(x, z)];
}

void region_regenerate_buffers(struct region *region)
{
	for (size_t i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i)
	{
		struct chunk *chunk = region->chunks[i];
		if (!chunk)
			continue;
		chunk_regenerate_buffers(chunk);
	}
}
