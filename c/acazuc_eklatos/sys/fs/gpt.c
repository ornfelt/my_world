#include <file.h>
#include <disk.h>
#include <vfs.h>
#include <std.h>

#define GUID_FMT "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "-" \
                 "%02" PRIx8 "%02" PRIx8 "-" \
                 "%02" PRIx8 "%02" PRIx8 "-" \
                 "%02" PRIx8 "%02" PRIx8 "-" \
                 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8
#define GUID_PRINT(v) v[0x3], v[0x2], v[0x1], v[0x0], \
                      v[0x5], v[0x4], v[0x7], v[0x6], \
                      v[0x9], v[0x8], v[0xF], v[0xE], \
                      v[0xD], v[0xC], v[0xB], v[0xA]

#define BLOCK_SIZE 512

struct gpt_header
{
	uint8_t sig[8];
	uint32_t revision;
	uint32_t size;
	uint32_t crc32;
	uint32_t reserved;
	uint64_t current_lba;
	uint64_t backup_lba;
	uint64_t first_lba;
	uint64_t last_lba;
	uint8_t guid[16];
	uint64_t part_lba;
	uint32_t part_count;
	uint32_t part_size;
	uint32_t part_crc32;
};

struct gpt_part
{
	uint8_t type_guid[16];
	uint8_t guid[16];
	uint64_t start_lba;
	uint64_t end_lba;
	uint64_t attributes;
	uint16_t name[];
};

static int create_partition(struct disk *disk, const struct gpt_part *part)
{
	struct partition *partition;
	int ret = partition_new(disk, disk->partitions_nb + 1,
	                        part->start_lba * BLOCK_SIZE,
	                        (part->end_lba - part->start_lba) * BLOCK_SIZE,
	                        &partition);
	if (ret)
	{
		printf("failed to create partition: %d\n", ret);
		return ret;
	}
	struct partition **partitions = realloc(disk->partitions, sizeof(*part) * (disk->partitions_nb + 1), 0);
	if (!partitions)
		return -ENOMEM;
	disk->partitions = partitions;
	partitions[disk->partitions_nb++] = partition;
	return 0;
}

int gpt_parse(struct disk *disk)
{
	const struct gpt_header *header;
	uint8_t header_data[BLOCK_SIZE];
	ssize_t ret;
	struct file *file;

	ret = file_frombdev(disk->bdev, O_RDONLY, &file);
	if (ret)
		return ret;
	ret = file_open(file, NULL);
	if (ret)
	{
		file_free(file);
		return ret;
	}
	ret = file_readseq(file, header_data, BLOCK_SIZE, BLOCK_SIZE);
	if (ret < 0)
	{
		printf("gpt: failed to read header lba\n");
		file_free(file);
		return ret;
	}
	header = (struct gpt_header*)&header_data[0];
	if (memcmp(header->sig, "\x45\x46\x49\x20\x50\x41\x52\x54", 8))
	{
		file_free(file);
		return -ENOENT;
	}
#if 0
	printf("revision : 0x%08" PRIx32 "\n", header->revision);
	printf("size     : 0x%08" PRIx32 "\n", header->size);
	printf("crc32    : 0x%08" PRIx32 "\n", header->crc32);
	printf("current  : 0x%016" PRIx64 "\n", header->current_lba);
	printf("backup   : 0x%016" PRIx64 "\n", header->backup_lba);
	printf("first    : 0x%016" PRIx64 "\n", header->first_lba);
	printf("last     : 0x%016" PRIx64 "\n", header->last_lba);
	printf("guid     : " GUID_FMT "\n", GUID_PRINT(header->guid));
	printf("part lba : 0x%016" PRIx64 "\n", header->part_lba);
	printf("part nb  : 0x%08" PRIx32 "\n", header->part_count);
	printf("part size: 0x%08" PRIx32 "\n", header->part_size);
	printf("part crc : 0x%08" PRIx32 "\n", header->part_crc32);
#endif
	/* XXX assert crc32 */
	/* XXX assert 0x80 part size */
	for (size_t i = 0; i < header->part_count; ++i)
	{
		uint8_t data[BLOCK_SIZE];
		uint64_t lba = header->part_lba + (header->part_size * i) / BLOCK_SIZE;
		ret = file_readseq(file, data, BLOCK_SIZE, lba * BLOCK_SIZE);
		if (ret < 0)
		{
			printf("gpt: failed to read part lba\n");
			file_free(file);
			return ret;
		}
		uint32_t offset = (header->part_size * i) % BLOCK_SIZE;
		const struct gpt_part *part = (struct gpt_part*)&data[offset];
		if (!memcmp(part->type_guid, "\x00\x00\x00\x00\x00\x00\x00\x00"
		                             "\x00\x00\x00\x00\x00\x00\x00\x00",
		                             16))
			continue;
#if 0
		printf("part type guid: " GUID_FMT "\n", GUID_PRINT(part->type_guid));
		printf("part guid     : " GUID_FMT "\n", GUID_PRINT(part->guid));
		printf("part start lba: 0x%016" PRIx64 "\n", part->start_lba);
		printf("part end lba  : 0x%016" PRIx64 "\n", part->end_lba);
		printf("part attrs    : 0x%016" PRIx64 "\n", part->attributes);
		printf("part name     : ");
		for (size_t n = 0; n < header->part_size - 0x38; ++n)
		{
			if (!part->name[n])
				break;
			printf("%c", part->name[n]);
		}
		printf("\n");
#endif
		ret = create_partition(disk, part);
	}
	file_free(file);
	return 0;
}
