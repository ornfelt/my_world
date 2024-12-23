#ifndef WOW_WDT_H
#define WOW_WDT_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

#define WOW_MPHD_FLAG_WMO        0x01
#define WOW_MPHD_FLAG_MCCV       0x02
#define WOW_MPHD_FLAG_BIG_ALPHA  0x04
#define WOW_MPHD_FLAG_M2_SORT    0x08
#define WOW_MPHD_FLAG_MCLV       0x10
#define WOW_MPHD_FLAG_REVERSE    0x20
#define WOW_MPHD_FLAG_0x40       0x40
#define WOW_MPHD_FLAG_HEIGHT_TEX 0x80

struct wow_mphd
{
	struct wow_chunk_header header;
	uint32_t flags;
	uint32_t something;
	uint32_t padding[6];
};

#define WOW_MAIN_FLAG_ADT 0x1

struct wow_main
{
	struct wow_chunk_header header;
	struct
	{
		uint32_t flags;
		uint32_t padding;
	} data[64 * 64];
};

struct wow_wdt_file
{
	struct wow_mver mver;
	struct wow_mphd mphd;
	struct wow_main main;
	struct wow_mwmo mwmo;
	struct wow_modf modf;
};

struct wow_wdt_file *wow_wdt_file_new(struct wow_mpq_file *mpq);
void wow_wdt_file_delete(struct wow_wdt_file *file);

#ifdef __cplusplus
}
#endif

#endif
