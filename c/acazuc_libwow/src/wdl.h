#ifndef WOW_WDL_H
#define WOW_WDL_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

struct wow_mare
{
	struct wow_chunk_header header;
	int16_t data[17 * 17 + 16 * 16];
};

struct wow_maof
{
	struct wow_chunk_header header;
	uint32_t offsets[64][64];
};

struct wow_wdl_file
{
	struct wow_mver mver;
	struct wow_maof maof;
	struct wow_mare mare[64][64];
};

struct wow_wdl_file *wow_wdl_file_new(struct wow_mpq_file *mpq);
void wow_wdl_file_delete(struct wow_wdl_file *file);

#ifdef __cplusplus
}
#endif

#endif
