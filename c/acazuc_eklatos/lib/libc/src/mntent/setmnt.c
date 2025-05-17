#include <mntent.h>

FILE *
setmnt(const char *file, const char *type)
{
	return fopen(file, type);
}
