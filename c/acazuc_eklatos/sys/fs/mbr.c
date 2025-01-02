#include <errno.h>
#include <disk.h>
#include <file.h>
#include <std.h>
#include <vfs.h>
#include <uio.h>

#define BLOCK_SIZE 512

struct mbr_part
{
	uint8_t status;
	uint8_t start_h;
	uint8_t start_s;
	uint8_t start_c;
	uint8_t type;
	uint8_t end_h;
	uint8_t end_s;
	uint8_t end_c;
	uint32_t lba;
	uint32_t sectors;
};

struct mbr
{
	uint8_t code[446];
	struct mbr_part partitions[4];
	uint8_t sig[2];
} __attribute__ ((packed));

static int read_mbr(uint8_t *data, struct disk *disk)
{
	struct file *file;
	int ret = file_frombdev(disk->bdev, O_RDONLY, &file);
	if (ret)
		return ret;
	ret = file_open(file, NULL);
	if (ret)
	{
		file_free(file);
		return ret;
	}
	ret = file_readseq(file, data, BLOCK_SIZE, 0);
	file_free(file);
	if (ret < 0)
		return ret;
	if (ret != BLOCK_SIZE)
		return -EIO;
	return 0;
}

int mbr_parse(struct disk *disk)
{
	uint8_t data[512];
	ssize_t ret;

	ret = read_mbr(data, disk);
	if (ret)
		return ret;
	struct mbr *mbr = (struct mbr*)data;
	for (size_t i = 0; i < 4; ++i)
	{
		struct mbr_part part = mbr->partitions[i];
#if 0
		printf("status: %x, type: %x\n", part.status, part.type);
#endif
		switch (part.type)
		{
			case 0x83:
			{
				struct partition *partition;
				ret = partition_new(disk, disk->partitions_nb + 1,
				                    part.lba * BLOCK_SIZE,
				                    part.sectors * BLOCK_SIZE,
				                    &partition);
				if (ret)
				{
					printf("failed to create partition: %d\n", (int)ret);
					break;
				}
				struct partition **partitions = realloc(disk->partitions, sizeof(part) * (disk->partitions_nb + 1), 0);
				if (!partitions)
					return -ENOMEM;
				disk->partitions = partitions;
				partitions[disk->partitions_nb++] = partition;
				break;
			}
			case 0xCD:
			{
				struct partition *partition;
				ret = partition_new(disk, disk->partitions_nb + 1,
				                    0,
				                    (part.lba + part.sectors) * BLOCK_SIZE,
				                    &partition);
				if (ret)
				{
					printf("failed to create partition: %d\n", (int)ret);
					break;
				}
				struct partition **partitions = realloc(disk->partitions, sizeof(part) * (disk->partitions_nb + 1), 0);
				if (!partitions)
					return -ENOMEM;
				disk->partitions = partitions;
				partitions[disk->partitions_nb++] = partition;
				break;
			}
			case 0:
				break;
			default:
				printf("unknown partition type: 0x%x\n", part.type);
				break;
		}
	}
	return 0;
}
