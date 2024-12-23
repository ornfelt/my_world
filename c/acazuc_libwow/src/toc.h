#ifndef WOW_TOC_H
#define WOW_TOC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct wow_toc_file
{
	char **files;
	uint32_t files_nb;
	char *title;
	char *notes;
	char *deps;
	char *url;
	uint32_t version;
};

struct wow_toc_file *wow_toc_file_new(const uint8_t *data, uint32_t len);
void wow_toc_file_delete(struct wow_toc_file *file);

#ifdef __cplusplus
}
#endif

#endif
