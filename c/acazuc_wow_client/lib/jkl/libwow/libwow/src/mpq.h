#ifndef WOW_MPQ_H
#define WOW_MPQ_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WOW_MPQ_CRYPT_OFFSET_HASH_BUCKET   0x000
#define WOW_MPQ_CRYPT_OFFSET_HASH_NAME_A   0x100
#define WOW_MPQ_CRYPT_OFFSET_HASH_NAME_B   0x200
#define WOW_MPQ_CRYPT_OFFSET_HASH_FILE_KEY 0x300
#define WOW_MPQ_CRYPT_OFFSET_DECRYPT_TABLE 0x400

#define WOW_MPQ_KEY_HASH_TABLE  0xC3AF3770 /* hash("(hash table)" , WOW_MPQ_CRYPT_OFFSET_HASH_FILE_KEY) */
#define WOW_MPQ_KEY_BLOCK_TABLE 0xEC83B3A3 /* hash("(block table)", WOW_MPQ_CRYPT_OFFSET_HASH_FILE_KEY) */

#define WOW_MPQ_BLOCK_IMPLODE       0x00000100
#define WOW_MPQ_BLOCK_COMPRESS      0x00000200
#define WOW_MPQ_BLOCK_ENCRYPTED     0x00010000
#define WOW_MPQ_BLOCK_FIX_KEY       0x00020000
#define WOW_MPQ_BLOCK_PATCH_FILE    0x00100000
#define WOW_MPQ_BLOCK_DELETE_MARKER 0x02000000
#define WOW_MPQ_BLOCK_SINGLE_UNIT   0x01000000
#define WOW_MPQ_BLOCK_SECTOR_CRC    0x04000000
#define WOW_MPQ_BLOCK_EXISTS        0x80000000

#define WOW_MPQ_COMPRESSION_NONE   0x00
#define WOW_MPQ_COMPRESSION_ZLIB   0x02
#define WOW_MPQ_COMPRESSION_PKWARE 0x08
#define WOW_MPQ_COMPRESSION_BZIP2  0x10
#define WOW_MPQ_COMPRESSION_SPARSE 0x20
#define WOW_MPQ_COMPRESSION_LZMA   0x12

struct wow_mpq_header
{
	uint32_t id;
	uint32_t header_size;
	uint32_t archive_size;
	uint16_t format_version;
	uint16_t block_size;
	uint32_t hash_table_pos;
	uint32_t block_table_pos;
	uint32_t hash_table_size;
	uint32_t block_table_size;
};

struct wow_mpq_header2
{
	uint64_t hi_block_table_pos;
	uint16_t hash_table_posHi;
	uint16_t block_table_posHi;
};

struct wow_mpq_block
{
	uint32_t offset;
	uint32_t block_size;
	uint32_t file_size;
	uint32_t flags;
};

struct wow_mpq_hash
{
	uint32_t name_a;
	uint32_t name_b;
	uint16_t lc_locale;
	uint16_t platform;
	uint32_t block_index;
};

struct wow_mpq_archive
{
	struct wow_mpq_header2 header2;
	struct wow_mpq_header header;
	struct wow_mpq_block *block_table;
	struct wow_mpq_hash *hash_table;
	char *filename;
};

struct wow_mpq_archive_view
{
	const struct wow_mpq_archive *archive;
	uint8_t *buffer; /* compression buffer */
	FILE *file;
};

struct wow_mpq_compound
{
	struct wow_mpq_archive_view *archives;
	uint32_t archives_nb;
};

struct wow_mpq_file
{
	uint8_t *data;
	uint32_t size;
	uint32_t pos;
};

uint32_t wow_mpq_hash_string(const char *str, uint32_t hash_type);

struct wow_mpq_archive *wow_mpq_archive_new(const char *filename);
void wow_mpq_archive_delete(struct wow_mpq_archive *archive);

struct wow_mpq_compound *wow_mpq_compound_new(void);
void wow_mpq_compound_delete(struct wow_mpq_compound *compound);
bool wow_mpq_compound_add_archive(struct wow_mpq_compound *compound,
                                  const struct wow_mpq_archive *archive);
const struct wow_mpq_block *wow_mpq_get_archive_block(const struct wow_mpq_archive_view *archive,
                                                      const char *filename);
struct wow_mpq_file *wow_mpq_get_archive_file(const struct wow_mpq_archive_view *archive,
                                              const char *filename);
const struct wow_mpq_block *wow_mpq_get_block(const struct wow_mpq_compound *compound,
                                              const char *filename);
struct wow_mpq_file *wow_mpq_get_file(const struct wow_mpq_compound *compound,
                                      const char *filename);

void wow_mpq_file_delete(struct wow_mpq_file *file);
uint32_t wow_mpq_read(struct wow_mpq_file *file, void *data, uint32_t len);
int32_t wow_mpq_seek(struct wow_mpq_file *file, int32_t offset, int32_t whence);

void wow_mpq_normalize_mpq_fn(char *fn, size_t size);
void wow_mpq_normalize_blp_fn(char *fn, size_t size);
void wow_mpq_normalize_m2_fn(char *fn, size_t size);

#ifdef __cplusplus
}
#endif

#endif
