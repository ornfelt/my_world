#ifndef WOW_TRS_H
#define WOW_TRS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

struct wow_trs_file_entry
{
	char *name;
	char *hash;
};

struct wow_trs_file_dir
{
	char *name;
	struct wow_trs_file_entry *entries;
	uint32_t entries_nb;
};

struct wow_trs_file
{
	struct wow_trs_file_dir *dirs;
	uint32_t dirs_nb;
};

struct wow_trs_file *wow_trs_file_new(struct wow_mpq_file *mpq);
void wow_trs_file_delete(struct wow_trs_file *file);

#ifdef __cplusplus
}
#endif

#endif
