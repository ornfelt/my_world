#include "zmp.h"
#include "mpq.h"

#include <stdlib.h>

struct wow_zmp_file *
wow_zmp_file_new(struct wow_mpq_file *mpq)
{
	struct wow_zmp_file *file;

	file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	if (wow_mpq_read(mpq, file->data, sizeof(file->data)) != sizeof(file->data))
		goto err;
	return file;

err:
	wow_zmp_file_delete(file);
	return NULL;
}

void
wow_zmp_file_delete(struct wow_zmp_file *file)
{
	if (!file)
		return;
	free(file);
}
