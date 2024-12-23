#ifndef LIBWOW_ZMP_H
#define LIBWOW_ZMP_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

struct wow_zmp_file
{
	uint32_t data[128 * 128];
};

struct wow_zmp_file *wow_zmp_file_new(struct wow_mpq_file *mpq);
void wow_zmp_file_delete(struct wow_zmp_file *file);

#ifdef __cplusplus
}
#endif

#endif
